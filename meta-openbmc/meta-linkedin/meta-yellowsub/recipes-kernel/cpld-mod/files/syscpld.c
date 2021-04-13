/*
 * syscpld.c - The i2c driver for SYSCPLD
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
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

//#define DEBUG

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

static const i2c_dev_attr_st syscpld_attr_table[] = {
  {
    "cpld_rev",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    1, 0, 6,
  },
  {
    "cpld_released",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    1, 6, 1,
  },
  {
    "cpld_sub_rev",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    2, 0, 8,
  },
  {
    "fan1_speed",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x10, 0, 8,
  },
  {
    "fan2_speed",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x11, 0, 8,
  },
  {
    "fan3_speed",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x12, 0, 8,
  },
  {
    "fan4_speed",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x13, 0, 8,
  },
  {
    "fan1_speed2",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x14, 0, 8,
  },
  {
    "fan2_speed2",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x15, 0, 8,
  },
  {
    "fan3_speed2",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x16, 0, 8,
  },
  {
    "fan4_speed2",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x17, 0, 8,
  },
  {
    "pwr_main_enable",
    "0x0: Power main not enabled\n"
    "0x1: Power main is enabled\n",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x1a, 3, 1,
  },
  {
    "pwr_stby_ok",
    "0x0: Power standby not OK\n"
    "0x1: Power standby is OK\n",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x1a, 2, 1,
  },
  {
    "pwr_1.2v_ok",
    "0x0: 1.2v Power not OK\n"
    "0x1: 1.2vPower is OK\n",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x1a, 1, 1,
  },
  {
    "pwr_5v_stby_ok",
    "0x0: 5V standby Power not OK\n"
    "0x1: 5V standby Power is OK\n",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x1a, 0, 1,
  },
  {
    "fan1_pwm_contrl",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x20, 0, 5,
  },
  {
    "fan2_pwm_contrl",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x21, 0, 5,
  },
  {
    "fan3_pwm_contrl",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x22, 0, 5,
  },
  {
    "fan4_pwm_contrl",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x23, 0, 5,
  },
  {
    "uart_mux",
    "0x0: UART0_CPU0\n"
    "0x1: UART0_CPU1\n"
    "0x2: UART0_CPU2\n"
    "0x3: UART0_CPU3\n",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x25, 0, 2,
  },
  {
    "pwr_cyc_all_n",
    "0: power cycle all power\n"
    "1: normal",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x30, 0, 1,
  },
  {
    "pwr_main_n",
    "0: main power is off\n"
    "1: main power is enabled",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x30, 1, 1,
  },
};

static i2c_dev_data_st syscpld_data;

/*
 * SYSCPLD i2c addresses.
 */
static const unsigned short normal_i2c[] = {
  0x31, I2C_CLIENT_END
};

/* SYSCPLD id */
static const struct i2c_device_id syscpld_id[] = {
  { "syscpld", 0 },
  { },
};
MODULE_DEVICE_TABLE(i2c, syscpld_id);

/* Return 0 if detection is successful, -ENODEV otherwise */
static int syscpld_detect(struct i2c_client *client,
                          struct i2c_board_info *info)
{
  /*
   * We don't currently do any detection of the SYSCPLD
   */
  strlcpy(info->type, "syscpld", I2C_NAME_SIZE);
  return 0;
}

static int syscpld_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
  int n_attrs = sizeof(syscpld_attr_table) / sizeof(syscpld_attr_table[0]);
  return i2c_dev_sysfs_data_init(client, &syscpld_data,
                                 syscpld_attr_table, n_attrs);
}

static int syscpld_remove(struct i2c_client *client)
{
  i2c_dev_sysfs_data_clean(client, &syscpld_data);
  return 0;
}

static struct i2c_driver syscpld_driver = {
  .class    = I2C_CLASS_HWMON,
  .driver = {
    .name = "syscpld",
  },
  .probe    = syscpld_probe,
  .remove   = syscpld_remove,
  .id_table = syscpld_id,
  .detect   = syscpld_detect,
  .address_list = normal_i2c,
};

static int __init syscpld_mod_init(void)
{
  return i2c_add_driver(&syscpld_driver);
}

static void __exit syscpld_mod_exit(void)
{
  i2c_del_driver(&syscpld_driver);
}

MODULE_AUTHOR("Ping Mao <pmao@linkedin.com>");
MODULE_DESCRIPTION("SYSCPLD Driver");
MODULE_LICENSE("GPL");

module_init(syscpld_mod_init);
module_exit(syscpld_mod_exit);
