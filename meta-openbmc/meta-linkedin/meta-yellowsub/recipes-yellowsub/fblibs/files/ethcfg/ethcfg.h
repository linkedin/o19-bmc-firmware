/*
 *
 * Copyright 2019-present LinkedIn. All Rights Reserved.
 *
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

#ifndef __ETHCFG_H__
#define __ETHCFG_H__

#include <openbmc/ipmi.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Non-Volatile Ethernet Configuration API
 */
int init_from_nv(lan_config_t *lan);
int eth_conf_save_int(const char *key, int value);
int eth_conf_save_string(const char *key, const char* value);
int eth_conf_delete_key(const char *key);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __ETHCFG_H__ */
