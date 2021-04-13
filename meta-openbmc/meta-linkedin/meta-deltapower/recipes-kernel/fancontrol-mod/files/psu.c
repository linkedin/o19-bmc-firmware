/*
 * psu.c - The i2c driver for PSU
 *
 * Copyright 2018-present Linkedin. All Rights Reserved.
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

static ssize_t psu_byte_show(struct device *dev,
                             struct device_attribute *attr,
                             char *buf)
{
  int val;

  val = i2c_dev_read_byte(dev, attr);
  if (val < 0) {
    return val;
  }

  return scnprintf(buf, PAGE_SIZE, "%u\n", val);
}

static ssize_t psu_show(struct device *dev,
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

static const i2c_dev_attr_st psu_attr_table[] = {
  {
    "operation",
    NULL,
    psu_byte_show,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x1, 0, 8,
  },
  {
    "vout_mode",
    NULL,
    psu_byte_show,
    NULL,
    0x20, 0, 8,
  },
  {
    "fan_config_1_2",
    NULL,
    psu_byte_show,
    NULL,
    0x3A, 0, 8,
  },
  {
    "fan_command1",
    NULL,
    psu_byte_show,
    NULL,
    0x3b, 0, 8,
  },
  {
    "fan_command2",
    NULL,
//    I2C_DEV_ATTR_SHOW_DEFAULT,
    psu_byte_show,
    NULL,
    0x3c, 0, 8,
  },
  {
    "vin_uv_fault_limit",
    NULL,
    psu_byte_show,
    NULL,
    0x59, 0, 8,
  },
  {
    "status_byte",
    NULL,
    psu_byte_show,
    NULL,
    0x78, 0, 8,
  },
  {
    "status_word",
    NULL,
    psu_show,
    NULL,
    0x79, 0, 16,
  },
  {
    "status_vout",
    NULL,
    psu_byte_show,
    NULL,
    0x7a, 0, 8,
  },
  {
    "status_iout",
    NULL,
    psu_byte_show,
    NULL,
    0x7b, 0, 8,
  },
  {
    "status_input",
    NULL,
    psu_byte_show,
    NULL,
    0x7c, 0, 8,
  },
  {
    "status_temperature",
    NULL,
    psu_byte_show,
    NULL,
    0x7d, 0, 8,
  },
  {
    "status_fan1_2",
    NULL,
    psu_byte_show,
    NULL,
    0x81, 0, 8,
  },
  {
    "status_fan3_4",
    NULL,
    psu_byte_show,
    NULL,
    0x82, 0, 8,
  },
  {
    "vin",
    NULL,
    psu_show,
    NULL,
    0x88, 0, 16,
  },
  {
    "iin",
    NULL,
    psu_show,
    NULL,
    0x89, 0, 16,
  },
  {
    "vout",
    NULL,
    psu_show,
    NULL,
    0x8B, 0, 16,
  },
  {
    "temperature1",
    NULL,
    psu_show,
    NULL,
    0x8d, 0, 16,
  },
  {
    "temperature2",
    NULL,
    psu_show,
    NULL,
    0x8e, 0, 16,
  },
  {
    "temperature3",
    NULL,
    psu_show,
    NULL,
    0x8f, 0, 16,
  },
  {
    "fan1_speed",
    NULL,
    psu_show,
    NULL,
    0x90, 0, 16,
  },
  {
    "pout",
    NULL,
    psu_show,
    NULL,
    0x96, 0, 16,
  },
  {
    "pin",
    NULL,
    psu_show,
    NULL,
    0x97, 0, 16,
  },
};

static i2c_dev_data_st psu_data;

/*
 * PSU i2c addresses.
 */
static const unsigned short normal_i2c[] = {
  0x40, I2C_CLIENT_END
};

/* PSU id */
static const struct i2c_device_id psu_id[] = {
  { "psu", 0 },
  { },
};
MODULE_DEVICE_TABLE(i2c, psu_id);

/* Return 0 if detection is successful, -ENODEV otherwise */
static int psu_detect(struct i2c_client *client,
                          struct i2c_board_info *info)
{
  /*
   * We don't currently do any detection of the PSU
   */
  strlcpy(info->type, "psu", I2C_NAME_SIZE);
  return 0;
}

static int psu_probe(struct i2c_client *client,
                     const struct i2c_device_id *id)
{
  int n_attrs = sizeof(psu_attr_table) / sizeof(psu_attr_table[0]);
  return i2c_dev_sysfs_data_init(client, &psu_data,
                                 psu_attr_table, n_attrs);
}

static int psu_remove(struct i2c_client *client)
{
  i2c_dev_sysfs_data_clean(client, &psu_data);
  return 0;
}

static struct i2c_driver psu_driver = {
  .class    = I2C_CLASS_HWMON,
  .driver = {
    .name = "psu",
  },
  .probe    = psu_probe,
  .remove   = psu_remove,
  .id_table = psu_id,
  .detect   = psu_detect,
  .address_list = normal_i2c,
};

static int __init psu_mod_init(void)
{
  return i2c_add_driver(&psu_driver);
}

static void __exit psu_mod_exit(void)
{
  i2c_del_driver(&psu_driver);
}

MODULE_AUTHOR("Ping Mao <pmao@fb.com>");
MODULE_DESCRIPTION("PSU Driver");
MODULE_LICENSE("GPL");

module_init(psu_mod_init);
module_exit(psu_mod_exit);
