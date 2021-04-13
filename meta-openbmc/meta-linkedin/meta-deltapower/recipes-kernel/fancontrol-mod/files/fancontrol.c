/*
 * fancontrol.c - The i2c driver for FAN controller
 *
 * Copyright 2018-present LinkedIn. All Rights Reserved.
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
 */

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <i2c_dev_sysfs.h>

#ifdef DEBUG

#define PP_DEBUG(fmt, ...) do {                   \
  printk(KERN_DEBUG "%s:%d " fmt "\n",            \
         __FUNCTION__, __LINE__, ##__VA_ARGS__);  \
} while (0)

#else /* !DEBUG */

#define PP_DEBUG(fmt, ...)

#endif

static ssize_t fancontrol_fan_show(struct device *dev,
                                    struct device_attribute *attr,
                                    char *buf)
{
  int val;

  val = i2c_dev_read_word(dev, attr);
  if (val < 0) {
    return val;
  }

  return scnprintf(buf, PAGE_SIZE, "%u\n", val);
}

#define FANTRAY_PWM_HELP                        \
  "each value represents 1/32 duty cycle"
#define FANTRAY_LED_CTRL_HELP                   \
  "0x0: Under HW control\n"                     \
  "0x1: Red off, Blue on\n"                     \
  "0x2: Red on, Blue off\n"                     \
  "0x3: Red off, Blue off"
#define FANTRAY_LED_BLINK_HELP                  \
  "0: no blink\n"                               \
  "1: blink"

static const i2c_dev_attr_st fancontrol_attr_table[] = {
  {
    "status_word",
    "bit 2:  Temparature Warning\n"
    "bit 11: Fan warning\n"
    "bit 15: VOUT fault",
    fancontrol_fan_show,
    NULL,
    0x79, 0, 16,
  },
  {
    "fan1_2_status",
    "bit 0: fan1 fail\n"
    "bit 1: fan1 speed warning"
    "bit 8: fan2 fail\n"
    "bit 9: fan2 speed warning",
    fancontrol_fan_show,
    NULL,
    0x81, 0, 16,
  },
  {
    "fan3_4_status",
    "bit 0: fan3 fail\n"
    "bit 1: fan3 speed warning"
    "bit 8: fan4 fail\n"
    "bit 9: fan4 speed warning",
    fancontrol_fan_show,
    NULL,
    0x82, 0, 16,
  },
  {
    "vin",
    "vin\n",
    fancontrol_fan_show,
    NULL,
    0x88, 0, 16,
  },
  {
    "temperature1",
    "temperature1",
    fancontrol_fan_show,
    NULL,
    0x8d, 0, 16,
  },
  {
    "temperature2",
    "temperature2",
    fancontrol_fan_show,
    NULL,
    0x8e, 0, 16,
  },
  {
    "temperature3",
    "temperature3",
    fancontrol_fan_show,
    NULL,
    0x8f, 0, 16,
  },
  {
    "temperature4",
    "temperature4",
    fancontrol_fan_show,
    NULL,
    0xd0, 0, 16,
  },
  {
    "fan1_speed",
    "fan1 speed",
    fancontrol_fan_show,
    NULL,
    0x90, 0, 16,
  },
  {
    "fan2_speed",
    "fan2 speed",
    fancontrol_fan_show,
    NULL,
    0x91, 0, 16,
  },
  {
    "fan3_speed",
    "fan3 speed",
    fancontrol_fan_show,
    NULL,
    0x92, 0, 16,
  },
  {
    "fan4_speed",
    "fan4 speed",
    fancontrol_fan_show,
    NULL,
    0x93, 0, 16,
  },
};

static i2c_dev_data_st fancontrol_data;

/*
 * FANCTRL i2c addresses.
 */
static const unsigned short normal_i2c[] = {
  0x30, I2C_CLIENT_END
};

/* FANCTRL id */
static const struct i2c_device_id fancontrol_id[] = {
  { "fancontrol", 0 },
  { },
};
MODULE_DEVICE_TABLE(i2c, fancontrol_id);

/* Return 0 if detection is successful, -ENODEV otherwise */
static int fancontrol_detect(struct i2c_client *client,
                             struct i2c_board_info *info)
{
  /*
   * We don't currently do any detection of the FANCTRL
   */
  strlcpy(info->type, "fancontrol", I2C_NAME_SIZE);
  return 0;
}

static int fancontrol_probe(struct i2c_client *client,
                            const struct i2c_device_id *id)
{
  int n_attrs = sizeof(fancontrol_attr_table) / sizeof(fancontrol_attr_table[0]);
  return i2c_dev_sysfs_data_init(client, &fancontrol_data,
                                 fancontrol_attr_table, n_attrs);
}

static int fancontrol_remove(struct i2c_client *client)
{
  i2c_dev_sysfs_data_clean(client, &fancontrol_data);
  return 0;
}

static struct i2c_driver fancontrol_driver = {
  .class    = I2C_CLASS_HWMON,
  .driver = {
    .name = "fancontrol",
  },
  .probe    = fancontrol_probe,
  .remove   = fancontrol_remove,
  .id_table = fancontrol_id,

  .detect   = fancontrol_detect,
  .address_list = normal_i2c,
};

static int __init fancontrol_mod_init(void)
{
  return i2c_add_driver(&fancontrol_driver);
}

static void __exit fancontrol_mod_exit(void)
{
  i2c_del_driver(&fancontrol_driver);
}

MODULE_AUTHOR("Ping Mao <pmao@linkedin.com>");
MODULE_DESCRIPTION("FANCTRL Driver");
MODULE_LICENSE("GPL");

module_init(fancontrol_mod_init);
module_exit(fancontrol_mod_exit);
