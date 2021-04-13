/*
 * fanmon
 *
 * Copyright 2017-present Flex. All Rights Reserved.
 * Copyright 2014-present Facebook. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Daemon to manage the fan speed to ensure that we stay within a reasonable
 * temperature range.  We're using a simplistic algorithm to get started:
 *
 * If the fan is already on high, we'll move it to medium if we fall below
 * a top temperature.  If we're on medium, we'll move it to high
 * if the temperature goes over the top value, and to low if the
 * temperature falls to a bottom level.  If the fan is on low,
 * we'll increase the speed if the temperature rises to the top level.
 *
 * To ensure that we're not just turning the fans up, then back down again,
 * we'll require an extra few degrees of temperature drop before we lower
 * the fan speed.
 *
 * We check the RPM of the fans against the requested RPMs to determine
 * whether the fans are failing, in which case we'll turn up all of
 * the other fans and report the problem..
 *
 */

/* Yeah, the file ends in .cpp, but it's a C program.  Deal. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>
#include <facebook/flex_eeprom.h>
#include "watchdog.h"

#define INTERNAL_TEMPS(x) ((x) * 1000) // stored as C * 1000
#define EXTERNAL_TEMPS(x) ((x) / 1000)
#define WEDGE100_COME_DIMM 100000 // 100C

/*Fan Speed Percentage*/
#define PERCENT(n)   (n)

#define MAX_FAN_FAIL 2

/*
 * The sensor for the uServer CPU is not on the CPU itself, so it reads
 * a little low.  We are special casing this, but we should obviously
 * be thinking about a way to generalize these tweaks, and perhaps
 * the entire configuration.  JSON file?
 */

#define USERVER_TEMP_FUDGE INTERNAL_TEMPS(10)

#define BAD_TEMP INTERNAL_TEMPS(-60)

#define BAD_READ_THRESHOLD 2    /* How many times can reads fail */
#define FAN_FAILURE_THRESHOLD 4 /* How many times can a fan fail */
#define FAN_SHUTDOWN_THRESHOLD 4 /* How long fans can be failed before */
                                 /* we just shut down the whole thing. */

#define PWM_DIR "/sys/bus/i2c/drivers/fancpld/8-0033/"

#define PWM_UNIT_MAX 31

#define LM75_DIR "/sys/bus/i2c/drivers/lm75/"
#define COM_E_DIR "/sys/bus/i2c/drivers/lm75/" /*XXX*/

#define INTAKE_TEMP_DEVICE LM75_DIR "3-004c/hwmon/hwmon4"
#define CHIP_TEMP_DEVICE LM75_DIR "3-004a/hwmon/hwmon2"
#define EXHAUST_TEMP_DEVICE LM75_DIR "3-0048/hwmon/hwmon0"
#define USERVER_TEMP_DEVICE LM75_DIR "3-004b/hwmon/hwmon3"
#define QSFP_TEMP_DEVICE LM75_DIR "3-0049/hwmon/hwmon1"
#define LEFT_INLET_TEMP LM75_DIR "8-0048/hwmon/hwmon8"
#define RIGHT_INLET_TEMP LM75_DIR "8-0049/hwmon/hwmon9"

#define INTERNAL_VOLTS(x) ((x) * 1000) // stored as V * 1000
#define EXTERNAL_VOLTS(x) ((x) / 1000)


#define VOLT_DIR "/sys/bus/i2c/drivers/com_e_driver/4-0033/"

#define VOLT_1V8_DEVICE VOLT_DIR "in0_input"
#define VOLT_1V8_NOMINAL 1800

#define VOLT_3V3_DEVICE VOLT_DIR "in1_input"
#define VOLT_3V3_NOMINAL 3300

#define VOLT_5V0_DEVICE VOLT_DIR "in2_input"
#define VOLT_5V0_NOMINAL 5000

#define VOLT_12V_DEVICE VOLT_DIR "in3_input"
#define VOLT_12V_NOMINAL 12000

#define VOLT_1V2_DEVICE VOLT_DIR "in4_input"
#define VOLT_1V2_NOMINAL 1200

#define VOLT_LOWER(v)   ((95 * (v)) / 100)
#define VOLT_UPPER(v)   ((105 * (v)) / 100)

static const struct rails {
  const char *device;
  const uint nominal;
} rails[] = {
  { VOLT_1V8_DEVICE, VOLT_1V8_NOMINAL },
  { VOLT_3V3_DEVICE, VOLT_3V3_NOMINAL },
  { VOLT_5V0_DEVICE, VOLT_5V0_NOMINAL },
  { VOLT_12V_DEVICE, VOLT_12V_NOMINAL },
  { VOLT_1V2_DEVICE, VOLT_1V2_NOMINAL },
};

#define MEMA_TEMP_DEVICE VOLT_DIR "temp1_input"
#define MEMB_TEMP_DEVICE VOLT_DIR "temp2_input"


#define FAN_READ_RPM_FORMAT "fan%d_input"

#define FAN0_LED PWM_DIR "fantray1_led_ctrl"
#define FAN1_LED PWM_DIR "fantray2_led_ctrl"
#define FAN2_LED PWM_DIR "fantray3_led_ctrl"
#define FAN3_LED PWM_DIR "fantray4_led_ctrl"
#define FAN4_LED PWM_DIR "fantray5_led_ctrl"

#define FAN_LED_BLUE "0x1"
#define FAN_LED_RED "0x2"

#define MAIN_POWER "/sys/bus/i2c/drivers/syscpld/12-0031/pwr_main_n"
#define USERVER_POWER "/sys/bus/i2c/drivers/syscpld/12-0031/pwr_usrv_en"

#define LARGEST_DEVICE_NAME 120

const char *fan_led[] = {FAN0_LED, FAN1_LED, FAN2_LED, FAN3_LED, FAN4_LED,};


#define REPORT_TEMP 720  /* Report temp every so many cycles */

/* Sensor limits and tuning parameters */
#define INTAKE_LIMIT INTERNAL_TEMPS(70)
#define SWITCH_LIMIT INTERNAL_TEMPS(70)

#define TEMP_TOP INTERNAL_TEMPS(70)
#define TEMP_BOTTOM INTERNAL_TEMPS(40)

/*
 * Toggling the fan constantly will wear it out (and annoy anyone who
 * can hear it), so we'll only turn down the fan after the temperature
 * has dipped a bit below the point at which we'd otherwise switch
 * things up.
 */

#define COOLDOWN_SLOP INTERNAL_TEMPS(5)

#define WEDGE_FAN_LOW 35
#define WEDGE_FAN_MEDIUM 50
#define WEDGE_FAN_HIGH 70
#define WEDGE_FAN_MAX 100

/*
 * Mapping physical to hardware addresses for fans;  it's different for
 * RPM measuring and PWM setting, naturally.  Doh.
 */
int fan_to_rpm_map[] = {1, 3, 5, 7, 9};
int fan_to_pwm_map[] = {1, 2, 3, 4, 5};
#define FANS 5
// Tacho offset between front and rear fans:
#define REAR_FAN_OFFSET 1

/*
 * The measured RPM of the fans doesn't match linearly to the requested
 * rate.  In addition, there are coaxially mounted fans, so the rear fans
 * feed into the front fans.  The rear fans will run slower since they're
 * grabbing still air, and the front fants are getting an extra boost.
 *
 * We'd like to measure the fan RPM and compare it to the expected RPM
 * so that we can detect failed fans, so we have a table (derived from
 * hardware testing):
 */

struct rpm_to_pct_map {
  uint pct;
  uint rpm;
};

/* XXX need front map for hawk and bolt */
struct rpm_to_pct_map rpm_front_map[] = {
                                        {50, 10200},
                                        {60, 12150},
                                        {70, 14250},
                                        {80, 16350},
                                        {90, 18300},
                                        {100, 20100},
                                       };
#define FRONT_MAP_SIZE (sizeof(rpm_front_map) / sizeof(struct rpm_to_pct_map))

/* XXX need rear map for hawk and bolt */
struct rpm_to_pct_map rpm_rear_map[] = {
                                        {50, 8400},
                                        {60, 9900},
                                        {70, 11700},
                                        {80, 13350},
                                        {90, 15000},
                                        {100, 17550},
                                       };
#define REAR_MAP_SIZE (sizeof(rpm_rear_map) / sizeof(struct rpm_to_pct_map))

#define FAN_FAILURE_OFFSET 30

int fan_low = WEDGE_FAN_LOW;
int fan_medium = WEDGE_FAN_MEDIUM;
int fan_high = WEDGE_FAN_HIGH;
int fan_max = WEDGE_FAN_MAX;
int total_fans = FANS;
int fan_offset = 0;

int temp_bottom = TEMP_BOTTOM;
int temp_top = TEMP_TOP;

int report_temp = REPORT_TEMP;
bool verbose = false;

void usage() {
  fprintf(stderr,
          "fanmon [-v] [-l <low-pct>] [-m <medium-pct>] "
          "[-h <high-pct>]\n"
          "\t[-b <temp-bottom>] [-t <temp-top>] [-r <report-temp>]\n\n"
          "\tlow-pct defaults to %d%% fan\n"
          "\tmedium-pct defaults to %d%% fan\n"
          "\thigh-pct defaults to %d%% fan\n"
          "\ttemp-bottom defaults to %dC\n"
          "\ttemp-top defaults to %dC\n"
          "\treport-temp defaults to every %d measurements\n\n"
          "fanmon compensates for uServer temperature reading %d degrees low\n"
          "kill with SIGUSR1 to stop watchdog\n",
          fan_low,
          fan_medium,
          fan_high,
          EXTERNAL_TEMPS(temp_bottom),
          EXTERNAL_TEMPS(temp_top),
          report_temp,
          EXTERNAL_TEMPS(USERVER_TEMP_FUDGE));
  exit(1);
}

/* We need to open the device each time to read a value */
int read_device(const char *device, int *value) {
  FILE *fp;
  int rc;

  fp = fopen(device, "r");
  if (!fp) {
    int err = errno;
    syslog(LOG_INFO, "failed to open device %s", device);
    return err;
  }

  rc = fscanf(fp, "%d", value);
  fclose(fp);

  if (rc != 1) {
    syslog(LOG_INFO, "failed to read device %s", device);
    return ENOENT;
  } else {
    return 0;
  }
}

/* We need to open the device again each time to write a value */

int write_device(const char *device, const char *value) {
  FILE *fp;
  int rc;

  fp = fopen(device, "w");
  if (!fp) {
    int err = errno;

    syslog(LOG_INFO, "failed to open device for write %s", device);
    return err;
  }

  rc = fputs(value, fp);
  fclose(fp);

  if (rc < 0) {
    syslog(LOG_INFO, "failed to write device %s", device);
    return ENOENT;
  } else {
    return 0;
  }
}

int read_temp(const char *device, int *value) {
  char full_name[LARGEST_DEVICE_NAME + 1];

  /* We set an impossible value to check for errors */
  *value = BAD_TEMP;
  snprintf(
      full_name, LARGEST_DEVICE_NAME, "%s/temp1_input", device);

  int rc = read_device(full_name, value);
  // XXX COM_E_DIR redefined as LM75, so anything > 100C
  if ((rc || *value > WEDGE100_COME_DIMM) && (strstr(device, COM_E_DIR))) {
    *value = BAD_TEMP;
  }
  return rc;
}

int read_fan_value(const int fan, const char *device, int *value) {
  char device_name[LARGEST_DEVICE_NAME];
  char full_name[LARGEST_DEVICE_NAME];

  snprintf(device_name, LARGEST_DEVICE_NAME, device, fan);
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", PWM_DIR,device_name);
  return read_device(full_name, value);
}

int write_fan_value(const int fan, const char *device, const int value) {
  char full_name[LARGEST_DEVICE_NAME];
  char device_name[LARGEST_DEVICE_NAME];
  char output_value[LARGEST_DEVICE_NAME];

  snprintf(device_name, LARGEST_DEVICE_NAME, device, fan);
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", PWM_DIR, device_name);
  snprintf(output_value, LARGEST_DEVICE_NAME, "%d", value);
  return write_device(full_name, output_value);
}

/* Return fan speed as a percentage of maximum -- not necessarily linear. */

int fan_rpm_to_pct(const struct rpm_to_pct_map *table,
                   const int table_len,
                   int rpm) {
  int i;

  for (i = 0; i < table_len; i++) {
    if (table[i].rpm > rpm) {
      break;
    }
  }

  /*
   * If the fan RPM is lower than the lowest value in the table,
   * we may have a problem -- fans can only go so slow, and it might
   * have stopped.  In this case, we'll return an interpolated
   * percentage, as just returning zero is even more problematic.
   */

  if (i == 0) {
    return (rpm * table[i].pct) / table[i].rpm;
  } else if (i == table_len) { // Fell off the top?
    return table[i - 1].pct;
  }

  // Interpolate the right percentage value:

  int percent_diff = table[i].pct - table[i - 1].pct;
  int rpm_diff = table[i].rpm - table[i - 1].rpm;
  int fan_diff = table[i].rpm - rpm;

  return table[i].pct - (fan_diff * percent_diff / rpm_diff);
}

int fan_speed_okay(const int fan, const int speed, const int slop) {
  int front_fan, rear_fan;
  int front_pct, rear_pct;
  int real_fan;
  int okay;

  /*
   * The hardware fan numbers are different from the physical order
   * in the box, so we have to map them:
   */

  real_fan = fan_to_rpm_map[fan];

  front_fan = 0;
  read_fan_value(real_fan, FAN_READ_RPM_FORMAT, &front_fan);
  front_pct = fan_rpm_to_pct(rpm_front_map, FRONT_MAP_SIZE, front_fan);
  rear_fan = 0;
  read_fan_value(real_fan + REAR_FAN_OFFSET, FAN_READ_RPM_FORMAT, &rear_fan);
  rear_pct = fan_rpm_to_pct(rpm_rear_map, REAR_MAP_SIZE, rear_fan);

  /*
   * If the fans are broken, the measured rate will be rather
   * different from the requested rate, and we can turn up the
   * rest of the fans to compensate.  The slop is the percentage
   * of error that we'll tolerate.
   *
   * XXX:  I suppose that we should only measure negative values;
   * running too fast isn't really a problem.
   */

  okay = (abs(front_pct - speed) * 100 / speed < slop &&
          abs(rear_pct - speed) * 100 / speed < slop);

  if (!okay || verbose) {
    syslog(!okay ? LOG_WARNING : LOG_INFO,
           "fan %d rear %d (%d%%), front %d (%d%%), expected %d",
           fan,
           rear_fan,
           rear_pct,
           front_fan,
           front_pct,
           speed);
  }

  return okay;
}

/* Set fan speed as a percentage */

int write_fan_speed(const int fan, const int value) {
  /*
   * The hardware fan numbers for pwm control are different from
   * both the physical order in the box, and the mapping for reading
   * the RPMs per fan, above.
   */

  int real_fan = fan_to_pwm_map[fan];

  if (value == 0) {
    return write_fan_value(real_fan, "fantray%d_pwm", 0);
  } else {
    int unit = value * PWM_UNIT_MAX / 100;

    // Note that PWM for Wedge100 is in 32nds of a cycle
    return write_fan_value(real_fan, "fantray%d_pwm", unit);
  }
}

/* Set up fan LEDs */

int write_fan_led(const int fan, const char *color) {
  return write_device(fan_led[fan], color);
}

int server_shutdown(const char *why) {
  int fan;
  for (fan = 0; fan < total_fans; fan++) {
    write_fan_speed(fan + fan_offset, fan_max);
  }

  syslog(LOG_EMERG, "Shutting down:  %s", why);
  write_device(USERVER_POWER, "0");
  sleep(5);
  write_device(MAIN_POWER, "0");
  // TODO(7088822):  try throttling, then shutting down server.
  syslog(LOG_EMERG, "Need to implement actual shutdown!");

  /*
   * We have to stop the watchdog, or the system will be automatically
   * rebooted some seconds after fanmon exits (and stops kicking the
   * watchdog).
   */

  stop_watchdog();

  sleep(2);
  exit(2);
}

/* Gracefully shut down on receipt of a signal */

void fand_interrupt(int sig)
{
  int fan;
  for (fan = 0; fan < total_fans; fan++) {
    write_fan_speed(fan + fan_offset, fan_max);
  }

  syslog(LOG_WARNING, "Shutting down fanmon on signal %s", strsignal(sig));
  if (sig == SIGUSR1) {
    stop_watchdog();
  }
  exit(3);
}

bool getPlatformName(char *name)
{
  struct wedge_eeprom_st eeprom;

  /* Retrieve the board type from EEPROM */
  if (wedge_eeprom_parse(NULL, &eeprom) == 0) {
    int src_pos = 0, target_pos = 0;

    while (eeprom.fbw_product_name[src_pos] != '\0') {
      if (eeprom.fbw_product_name[src_pos] != ' ') {
        name[target_pos++] = eeprom.fbw_product_name[src_pos];
      }
      src_pos++;
    }
    name[target_pos] = '\0';
    syslog(LOG_NOTICE, "board type is %s", eeprom.fbw_product_name);
    return true;
  } else
    syslog(LOG_ERR, "wedge_eeprom_parse failed");
  strcpy(name, "");
  return true;
}

int main(int argc, char **argv) {
  /* Sensor values */

  int intake_temp;
  int exhaust_temp;
  int switch_temp;
  int userver_temp;
  int qsfp_temp;
  int avg_temp = 0;
  int left_inlet_temp;
  int right_inlet_temp;
  char platformName[32]="";

  int fan_speed = fan_max;
  int bad_reads = 0;
  int bad_read_intervals = 0;
  int fan_failure = 0;
  int fan_speed_changes = 0;
  int old_speed;

  int fan_bad[FANS];
  int fan;

  unsigned log_count = 0; // How many times have we logged our temps?
  int opt;
  int prev_fans_bad = 0;

  struct sigaction sa;

  sa.sa_handler = fand_interrupt;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);

  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGUSR1, &sa, NULL);

  // Start writing to syslog as early as possible for diag purposes.

  openlog("fanmon", LOG_CONS, LOG_DAEMON);

  while ((opt = getopt(argc, argv, "l:m:h:b:t:r:v")) != -1) {
    switch (opt) {
    case 'l':
      fan_low = atoi(optarg);
      break;
    case 'm':
      fan_medium = atoi(optarg);
      break;
    case 'h':
      fan_high = atoi(optarg);
      break;
    case 'b':
      temp_bottom = INTERNAL_TEMPS(atoi(optarg));
      break;
    case 't':
      temp_top = INTERNAL_TEMPS(atoi(optarg));
      break;
    case 'r':
      report_temp = atoi(optarg);
      break;
    case 'v':
      verbose = true;
      break;
    default:
      usage();
      break;
    }
  }

  if (optind > argc) {
    usage();
  }

  if (temp_bottom > temp_top) {
    fprintf(stderr,
            "Should temp-bottom (%d) be higher than "
            "temp-top (%d)?  Starting anyway.\n",
            EXTERNAL_TEMPS(temp_bottom),
            EXTERNAL_TEMPS(temp_top));
  }

  if (fan_low > fan_medium || fan_low > fan_high || fan_medium > fan_high) {
    fprintf(stderr,
            "fan RPMs not strictly increasing "
            "-- %d, %d, %d, starting anyway\n",
            fan_low,
            fan_medium,
            fan_high);
  }

  daemon(1, 0);

  if (verbose) {
    syslog(LOG_DEBUG, "Starting up;  system should have %d fans.",
           total_fans);
  }

  for (fan = 0; fan < total_fans; fan++) {
    fan_bad[fan] = 0;
    write_fan_speed(fan + fan_offset, fan_speed);
    write_fan_led(fan + fan_offset, FAN_LED_BLUE);
  }

  /* Start watchdog in manual mode */
  start_watchdog(0);

  /* Set watchdog to persistent mode so timer expiry will happen independent
   * of this process's liveliness. */
  set_persistent_watchdog(WATCHDOG_SET_PERSISTENT);

  do {
    sleep(5);  /* Give the fans time to come up to speed */
    kick_watchdog();
    /* can't do anything with temperatures until platform name known? */
  } while (platformName[0] == '\0' && !getPlatformName(platformName));

  while (1) {
    old_speed = fan_speed;

    /* Read sensors */

    read_temp(INTAKE_TEMP_DEVICE, &intake_temp);
    read_temp(EXHAUST_TEMP_DEVICE, &exhaust_temp);
    read_temp(CHIP_TEMP_DEVICE, &switch_temp);
    read_temp(USERVER_TEMP_DEVICE, &userver_temp);
    read_temp(QSFP_TEMP_DEVICE, &qsfp_temp);
    read_temp(LEFT_INLET_TEMP, &left_inlet_temp);
    read_temp(RIGHT_INLET_TEMP, &right_inlet_temp);

    bad_reads = 0; /* XXX reset every interval */
    if (strcasecmp(platformName, "hawk") == 0) {
      if (exhaust_temp != BAD_TEMP) {
        if (qsfp_temp != BAD_TEMP) {
          avg_temp = (exhaust_temp + qsfp_temp) / 2;
        } else {
          avg_temp = exhaust_temp;
          bad_reads++;
        }
      } else {
        if (qsfp_temp != BAD_TEMP) {
          avg_temp = qsfp_temp;
        } else {
          fan_failure++; /* XXX fan max */
          bad_reads++;
        }
        bad_reads++;
      }
    } else if (strcasecmp(platformName, "bolt") == 0) {
      if (left_inlet_temp != BAD_TEMP) {
        if (right_inlet_temp != BAD_TEMP) {
          avg_temp = (left_inlet_temp + right_inlet_temp) / 2;
        } else {
          avg_temp = left_inlet_temp;
          bad_reads++;
        }
      } else {
        if (right_inlet_temp != BAD_TEMP) {
          avg_temp = right_inlet_temp;
        } else {
          fan_failure++; /* XXX fan max */
          bad_reads++;
        }
        bad_reads++;
      }
    }

    /*
     * uServer can be powered down, but all of the rest of the sensors
     * should be readable at any time.
     */

    /* TODO(vineelak) : Add userver_temp too , in case we fail to read temp */
    if ((intake_temp == BAD_TEMP || exhaust_temp == BAD_TEMP ||
         switch_temp == BAD_TEMP)) {
      bad_reads++;
    }

    if (bad_reads > BAD_READ_THRESHOLD) {
      if (++bad_read_intervals >= 2) {
	server_shutdown("Some sensors couldn't be read");
      }
    } else {
      /* good interval; reset interval counter */
      bad_read_intervals = 0;
    }

    if (log_count++ % report_temp == 0) {
      syslog(LOG_DEBUG,
          "Temp intake %d, switch %d, "
          "left inlet %d, right inlet %d,"
          " userver %d, exhaust %d, "
          "qsfp %d average %d, "
          "fan speed %d, speed changes %d",
          intake_temp,
          switch_temp,
          left_inlet_temp, right_inlet_temp,
          userver_temp,
          exhaust_temp,
          qsfp_temp, avg_temp,
          fan_speed,
          fan_speed_changes);
    }

    /* Protection heuristics */

    if (switch_temp > SWITCH_LIMIT) {
      server_shutdown("T2 temp limit reached");
    }

    if (exhaust_temp > INTAKE_LIMIT || userver_temp > SWITCH_LIMIT || qsfp_temp > SWITCH_LIMIT) {
      server_shutdown("Exhaust, userver, or qsfp temp limit reached");
    }

    /*
     * If recovering from a fan problem, spin down fans gradually in case
     * temperatures are still high. Gradual spin down also reduces wear on
     * the fans.
     */
    if (fan_speed >= PERCENT(90)) {
      if (fan_failure == 0) {
        fan_speed = PERCENT(80);			// lower to 80% if no failures
      }
    } else if (fan_speed >= PERCENT(80)) {
      if (avg_temp < INTERNAL_TEMPS(45)) {
        fan_speed = PERCENT(70);			// lower to 70% if < 45C
      }
    } else if (fan_speed >= PERCENT(70)) {
      if (avg_temp > INTERNAL_TEMPS(50)) {
        fan_speed = PERCENT(80);			// raise to 80% if > 50C
      } else if (avg_temp < INTERNAL_TEMPS(40)) {
        fan_speed = PERCENT(60);			// lower to 60% if < 40C
      }
    } else if (fan_speed >= PERCENT(60)) {
      if (avg_temp > INTERNAL_TEMPS(45)) {
        fan_speed = PERCENT(70);			// raise to 70% if > 45C
      } else if (avg_temp < INTERNAL_TEMPS(35)) {
        fan_speed = PERCENT(50);			// lower to 50% if < 35C
      }
    } else {/* low */
      if (avg_temp > INTERNAL_TEMPS(40)) {
        fan_speed = PERCENT(60);			// raise to 60% if > 40C
      }
    }

    /*
     * Update fans only if there are no failed ones. If any fans failed
     * earlier, all remaining fans should continue to run at max speed.
     */

    if (fan_failure == 0 && fan_speed != old_speed) {
      syslog(LOG_NOTICE,
          "Fan speed changing from %d to %d",
          old_speed,
          fan_speed);
      fan_speed_changes++;
      for (fan = 0; fan < total_fans; fan++) {
        write_fan_speed(fan + fan_offset, fan_speed);
      }
    }

    for (unsigned rn = 0; rn < sizeof rails / sizeof rails[0]; rn++) {
      int mv;

      /* open / read failures already logged -- just do range-check */
      if (read_device(rails[rn].device, &mv) == 0) {
        if (mv < VOLT_LOWER(rails[rn].nominal) || mv > VOLT_UPPER(rails[rn].nominal)) {
          syslog(LOG_NOTICE, "Rail expected %u mV, got %u mV", rails[rn].nominal, mv);
        }
      }
    }

    {
      /* read memory temperatures */
      int mema, memb;
      read_device(MEMA_TEMP_DEVICE, &mema);
      read_device(MEMB_TEMP_DEVICE, &memb);
    }

    /*
     * Wait for some change.  Typical I2C temperature sensors
     * only provide a new value every second and a half, so
     * checking again more quickly than that is a waste.
     *
     * We also have to wait for the fan changes to take effect
     * before measuring them.
     */

    sleep(5);

    /* Check fan RPMs */

    for (fan = 0; fan < total_fans; fan++) {
      /*
       * Make sure that we're within some percentage
       * of the requested speed.
       */
      if (fan_speed_okay(fan + fan_offset, fan_speed, FAN_FAILURE_OFFSET)) {
        if (fan_bad[fan] > FAN_FAILURE_THRESHOLD) {
          write_fan_led(fan + fan_offset, FAN_LED_BLUE);
          syslog(LOG_CRIT,
                 "Fan %d has recovered",
                 fan);
        }
        fan_bad[fan] = 0;
      } else {
        fan_bad[fan]++;
        syslog(LOG_DEBUG, "Fan %d has failed for %d consecutive testing cycles.", fan, fan_bad[fan]);
	// rewrite commanded-speed and hope it's okay next time
	// (might just be a user running set_fan_speed script)
        write_fan_speed(fan + fan_offset, fan_speed);
      }
    }

    fan_failure = 0;
    for (fan = 0; fan < total_fans; fan++) {
      if (fan_bad[fan] > FAN_FAILURE_THRESHOLD) {
        fan_failure++;
        write_fan_led(fan + fan_offset, FAN_LED_RED);
      }
    }

    if (fan_failure > 0) {
      if (prev_fans_bad != fan_failure) {
        syslog(LOG_CRIT, "%d fans failed", fan_failure);
      }

      /*
       * If fans are bad, we need to blast all of the
       * fans at 100%;  we don't bother to turn off
       * the bad fans, in case they are all that is left.
       *
       * Note that we have a temporary bug with setting fans to
       * 100% so we only do fan_max = 99%.
       */

      fan_speed = fan_max;
      syslog(LOG_CRIT, "Blasting all fans to %d%%", fan_speed);
      for (fan = 0; fan < total_fans; fan++) {
        write_fan_speed(fan + fan_offset, fan_speed);
      }

      /*
       * On Wedge, we want to shut down everything if none of the fans
       * are visible, since there isn't automatic protection to shut
       * off the server or switch chip.  On other platforms, the CPUs
       * generating the heat will automatically turn off, so this is
       * unnecessary.
       */

      if (fan_failure > MAX_FAN_FAIL) {
        int count = 0;
        for (fan = 0; fan < total_fans; fan++) {
          if (fan_bad[fan] > FAN_SHUTDOWN_THRESHOLD)
            count++;
        }
        if (count > MAX_FAN_FAIL) {
#define TOSTR2(a) #a
#define TOSTR(a) TOSTR2(a)
          server_shutdown(TOSTR(MAX_FAN_FAIL)
            " fans are bad for more than "
            TOSTR(FAN_SHUTDOWN_THRESHOLD)
            " cycles");
        }
      }

      /*
       * Fans can be hot swapped and replaced; in which case the fan daemon
       * will automatically detect the new fan and (assuming the new fan isn't
       * itself faulty), automatically readjust the speeds for all fans down
       * to a more suitable rpm. The fan daemon does not need to be restarted.
       */
    }

    /* Suppress multiple warnings for similar number of fan failures. */
    prev_fans_bad = fan_failure;

    /* if everything is fine, restart the watchdog countdown. If this process
     * is terminated, the persistent watchdog setting will cause the system
     * to reboot after the watchdog timeout. */
    kick_watchdog();
  }
}
