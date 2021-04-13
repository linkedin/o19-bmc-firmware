/*
 * soccpld.c - The i2c driver for SYSCPLD
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

static const i2c_dev_attr_st soccpld_attr_table[] = {
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
    "p2v5_vpp_enable",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x10, 3, 1,
  },
  {
    "p1v05_combined_ok",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x10, 2, 1,
  },
  {
    "apwr_ok",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x10, 1, 1,
  },
  {
    "pvccscfusesus",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x10, 0, 1,
  },
  {
    "reset",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x11, 0, 1,
  },
  {
    "pwrgd_apwrok",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x20, 1, 1,
  },
  {
    "sys_pwr_ok",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x20, 0, 1,
  },
  {
    "pwr_btn",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x30, 0, 1,
  },
  {
    "reset",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x31, 0, 1,
  },
};

static i2c_dev_data_st soccpld_data;

/*
 * SOCCPLD i2c addresses.
 */
static const unsigned short normal_i2c[] = {
  0x31, I2C_CLIENT_END
};

/* SOCCPLD id */
static const struct i2c_device_id soccpld_id[] = {
  { "soccpld", 0 },
  { },
};
MODULE_DEVICE_TABLE(i2c, soccpld_id);

/* Return 0 if detection is successful, -ENODEV otherwise */
static int soccpld_detect(struct i2c_client *client,
                          struct i2c_board_info *info)
{
  /*
   * We don't currently do any detection of the SOCCPLD
   */
  strlcpy(info->type, "soccpld", I2C_NAME_SIZE);
  return 0;
}

static int soccpld_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
  int n_attrs = sizeof(soccpld_attr_table) / sizeof(soccpld_attr_table[0]);
  return i2c_dev_sysfs_data_init(client, &soccpld_data,
                                 soccpld_attr_table, n_attrs);
}

static int soccpld_remove(struct i2c_client *client)
{
  i2c_dev_sysfs_data_clean(client, &soccpld_data);
  return 0;
}

static struct i2c_driver soccpld_driver = {
  .class    = I2C_CLASS_HWMON,
  .driver = {
    .name = "soccpld",
  },
  .probe    = soccpld_probe,
  .remove   = soccpld_remove,
  .id_table = soccpld_id,
  .detect   = soccpld_detect,
  .address_list = normal_i2c,
};

static int __init soccpld_mod_init(void)
{
  return i2c_add_driver(&soccpld_driver);
}

static void __exit soccpld_mod_exit(void)
{
  i2c_del_driver(&soccpld_driver);
}

MODULE_AUTHOR("Ping Mao <pmao@linkedin.com>");
MODULE_DESCRIPTION("SOCCPLD Driver");
MODULE_LICENSE("GPL");

module_init(soccpld_mod_init);
module_exit(soccpld_mod_exit);
