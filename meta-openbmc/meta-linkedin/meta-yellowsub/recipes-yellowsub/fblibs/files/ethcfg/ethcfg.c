/*
 * eth cfg library
 *
 * Copyright 2019-present LinkedIn. All Rights Reserved.
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


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <stdbool.h>
#include <jansson.h>

#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/route.h>
#include <linux/sockios.h>
#include <linux/if_vlan.h>

#include "ethcfg.h"

#define CMD_STR_SIZE  120
#define DEFAULT_ETH_DEVICE "eth0"

#define    IPMI_IP_SRC_STATIC              1
#define    IPMI_IP_SRC_DHCP                2

/*
 * Ethernet Configuration File Path
 */
#define ETH_CONF    "/mnt/data/eth.cfg"

/*
 * Non-Volatile Ethernet Configuration
 */
static int access_eth_cfg_file()
{
  // check for permission to access the /mnt/data
  if (access("/mnt/data/", F_OK) < 0)
  {
    syslog(LOG_WARNING, "access_eth_cfg_file(): access /mnt/data/ error");
    return -1;
  }

  // check if eth conf file exists. If not, create it
  int fd = open(ETH_CONF, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd < 0)
  {
    if (errno == EEXIST)
    {
    // file already existed
    return 0;
    }

    // unspecified error
    syslog(LOG_WARNING, "access_eth_cfg_file(): create eth conf file error");
    return -1;
  }

  // file has been created, fill it with a stub of json
  const char* stub = "{}";
  if (write(fd, stub, 2) != 2)
  {
    syslog(LOG_WARNING, "access_eth_cfg_file(): create json stub error");
    close(fd);
    return -1;
  }

  close(fd);
  return 0;
}

int eth_conf_save_string(const char *key, const char* value)
{
  if(access_eth_cfg_file() != 0)
  {
    return -1;
  }

  json_t *conf;
  conf = json_load_file(ETH_CONF, 0, NULL);

  if(!conf)
  {
    syslog(LOG_WARNING, "eth_conf_save_string(): read eth config json file error");
    return -1;
  }

  if(json_object_set_new(conf, key, json_string(value)) < 0)
  {
    syslog(LOG_WARNING, "eth_conf_save_string(): json object set error key: %s, value: %s", key, value);
    json_decref(conf);
    return -1;
  }

  int rc = json_dump_file(conf, ETH_CONF, 0);
  json_decref(conf);
  if (rc)
  {
    syslog(LOG_WARNING, "eth_conf_save_string(): save key: %s, value: %s to json error", key, value);
    return -1;
  }
  return 0;
}

int eth_conf_save_int(const char *key, int value)
{
  if(access_eth_cfg_file() != 0)
  {
    return -1;
  }

  json_t *conf;
  conf = json_load_file(ETH_CONF, 0, NULL);

  if(!conf)
  {
    syslog(LOG_WARNING, "eth_conf_save_int(): read eth config json file error");
    return -1;
  }

  if(json_object_set_new(conf, key, json_integer(value)) < 0)
  {
    syslog(LOG_WARNING, "eth_conf_save_int(): json object set error key: %s, value: %d", key, value);
    json_decref(conf);
    return -1;
  }

  int rc = json_dump_file(conf, ETH_CONF, 0);
  json_decref(conf);
  if (rc)
  {
    syslog(LOG_WARNING, "eth_conf_save_int(): save key: %s, value: %d to json error", key, value);
    return -1;
  }
  return 0;
}

int eth_conf_delete_key(const char *key)
{
  if(access_eth_cfg_file() != 0)
  {
    return -1;
  }

  json_t *conf;
  conf = json_load_file(ETH_CONF, 0, NULL);

  if(!conf)
  {
    syslog(LOG_WARNING, "eth_conf_delete_key(): read eth config json file error");
    return -1;
  }

  if(json_object_del(conf, key) < 0)
  {
    syslog(LOG_WARNING, "eth_conf_delete_key(): json object delete error key: %s", key);
    json_decref(conf);
    return -1;
  }

  int rc = json_dump_file(conf, ETH_CONF, 0);
  json_decref(conf);
  if (rc)
  {
    syslog(LOG_WARNING, "eth_conf_delete_key(): save key: %s to json error", key);
    return -1;
  }
  return 0;
}

/*
 * Ethernet Configuration Setters
 */
static int disable_dhcp()
{
  char cmd[CMD_STR_SIZE];
  snprintf(cmd, CMD_STR_SIZE, "kill -9 `cat /var/run/dhclient.eth0.pid`");

  if(system(cmd) == -1)
  {
    syslog(LOG_WARNING, "disable_dhcp(): disable dhcp on eth0 error");
    return -1;
  }

  memset(cmd, 0x00, CMD_STR_SIZE);
  snprintf(cmd, CMD_STR_SIZE, "killall -9 dhclient");
  if(system(cmd) == -1)
  {
    syslog(LOG_WARNING, "disable_dhcp(): disable dhcp error");
    return -1;
  }
  return 0;
}

static int set_dhcp_iface(const char *if_name)
{
  char cmd[CMD_STR_SIZE];
  snprintf(cmd, CMD_STR_SIZE, "dhclient %s", if_name);

  if(system(cmd) == -1)
  {
    syslog(LOG_WARNING, "set_dhcp_iface(): set dhcp on interface error, if_name: %s", if_name);
    return -1;
  }
  return 0;
}

static int set_address(int fd, const char *if_name, const char *address)
{
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  struct sockaddr_in *addr;

  ifr.ifr_addr.sa_family = AF_INET;
  snprintf(ifr.ifr_name, IFNAMSIZ, if_name);
  addr=(struct sockaddr_in *)&ifr.ifr_addr;
  inet_pton(AF_INET, address, &addr->sin_addr);

  if(ioctl(fd, SIOCSIFADDR, &ifr) < 0)
  {
    syslog(LOG_WARNING, "set_address(): set address error, if_name: %s, address: %d", if_name, address);
    return -1;
  }
  return 0;
}

static int set_netmask(int fd, const char *if_name, const char *netmask)
{
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  struct sockaddr_in *addr;

  ifr.ifr_addr.sa_family = AF_INET;
  snprintf(ifr.ifr_name, IFNAMSIZ, if_name);
  addr=(struct sockaddr_in *)&ifr.ifr_addr;
  inet_pton(AF_INET, netmask, &addr->sin_addr);

  if(ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
  {
    syslog(LOG_WARNING, "set_netmask(): set netmask error, if_name: %s, netmask: %s", if_name, netmask);
    return -1;
  }
  return 0;
}

static int create_vlan(int fd, const char* if_name, int vlan_id)
{
  struct vlan_ioctl_args if_request;
  memset(&if_request, 0, sizeof(if_request));

  if_request.u.name_type = VLAN_NAME_TYPE_RAW_PLUS_VID_NO_PAD;
  if_request.cmd = ADD_VLAN_CMD;
  snprintf(if_request.device1, IFNAMSIZ, if_name);
  if_request.u.VID = vlan_id;

  // create vlan interface
  if (ioctl(fd, SIOCSIFVLAN, &if_request) < 0)
  {
    syslog(LOG_WARNING, "create_vlan(): create vlan error, if_name: %s, vlanid: %d", if_name, vlan_id);
    return -1;
  }
  return 0;
}

static int interface_up(int fd, const char* if_name)
{
  struct ifreq ifr;
  memset(&ifr, 0, sizeof ifr);

  snprintf(ifr.ifr_name, IFNAMSIZ, if_name);
  ifr.ifr_flags |= IFF_UP;

  if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
  {
    syslog(LOG_WARNING, "create_vlan(): create vlan error, if_name: %s", if_name);
    return -1;
  }
  return 0;
}

static int set_gateway(int fd, char *if_name, const char *address)
{
  struct rtentry rt;
  memset(&rt, 0, sizeof rt);

  struct sockaddr_in *sockinfo = (struct sockaddr_in *)&rt.rt_gateway;
  sockinfo->sin_family = AF_INET;
  sockinfo->sin_addr.s_addr = inet_addr(address);

  sockinfo = (struct sockaddr_in *)&rt.rt_dst;
  sockinfo->sin_family = AF_INET;
  sockinfo->sin_addr.s_addr = INADDR_ANY;

  sockinfo = (struct sockaddr_in *)&rt.rt_genmask;
  sockinfo->sin_family = AF_INET;
  sockinfo->sin_addr.s_addr = INADDR_ANY;

  rt.rt_flags = RTF_UP | RTF_GATEWAY;
  rt.rt_dev = if_name;

  if(ioctl(fd, SIOCADDRT, &rt) < 0)
  {
    syslog(LOG_WARNING, "set_gateway(): set gateway error, if_name: %s, addr: %s", if_name, address);
    return -1;
  }
  return 0;
}

int init_from_nv(lan_config_t *lan)
{
  // make sure we initialize from flash just once
  if(lan->init_from_nv_done)
  {
    return 0;
  }
  lan->init_from_nv_done = 1;

  // check for permission to access the config file
  if (access(ETH_CONF, R_OK) < 0)
  {
    syslog(LOG_WARNING, "init_from_nv(): access config file error");
    return -1;
  }

  json_t *conf;
  conf = json_load_file(ETH_CONF, 0, NULL);

  if(!conf)
  {
    syslog(LOG_WARNING, "init_from_nv(): eth config json file read error");
    return -1;
  }

  json_t *mode = json_object_get(conf, "mode");
  if(!mode || !json_is_string(mode))
  {
    syslog(LOG_WARNING, "init_from_nv(): mode read error");
    json_decref(conf);
    return -1;
  }

  if(disable_dhcp() < 0)
  {
    json_decref(conf);
    return -1;
  }

  // initially, interface name is hardcoded
  char if_name[IFNAMSIZ];
  snprintf(if_name, IFNAMSIZ, DEFAULT_ETH_DEVICE);
  // update ipmi lan context
  snprintf(lan->if_name, IFNAMSIZ, if_name);

  // create socket
  int fd;
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    syslog(LOG_WARNING, "init_from_nv(): socket oper error");
    json_decref(conf);
    return -1;
  }

  json_t *ip_addr_obj = json_object_get(conf, "ip_addr");
  json_t *gateway_obj = json_object_get(conf, "gateway");
  json_t *netmask_obj = json_object_get(conf, "netmask");
  json_t *vlan_id_obj = json_object_get(conf, "vlanid");

  // if VLAN enabled, create a device
  bool vlan_enabled = (vlan_id_obj != NULL) && json_is_integer(vlan_id_obj);
  if(vlan_enabled)
  {
    int vlan_id = json_integer_value(vlan_id_obj);

    // create vlan device
    if(create_vlan(fd, "eth0", vlan_id) < 0)
    {
      printf("Could not create vlan device: %d\n", vlan_id);
      goto init_end;
    }

    // new device created, update name
    snprintf(if_name, IFNAMSIZ, "%s.%d", DEFAULT_ETH_DEVICE, vlan_id);

    // update ipmi lan context
    snprintf(lan->if_name, IFNAMSIZ, if_name);
  }

  // enable dhcp / static mode for current device
  if(strcmp(json_string_value(mode), "dhclient") == 0)
  {
    if(set_dhcp_iface(if_name) < 0)
    {
      goto init_end;
    }
    // update ipmi lan context
    lan->ip_src = (unsigned char)IPMI_IP_SRC_DHCP;
  }
  else if(strcmp(json_string_value(mode), "static") == 0)
  {
    if((!ip_addr_obj || !json_is_string(ip_addr_obj)) || (!gateway_obj || !json_is_string(gateway_obj)))
    {
      syslog(LOG_WARNING, "init_from_nv(): static mode did not provide ip address and / or gateway error");
      goto init_end;
    }

    const char *ip_addr = json_string_value(ip_addr_obj);
    const char *gateway = json_string_value(gateway_obj);

    if(set_address(fd, if_name, "0.0.0.0") < 0)
    {
      goto init_end;
    }

    // set ip address
    if(set_address(fd, if_name, ip_addr) < 0)
    {
      goto init_end;
    }

    // set netmask if present
    if((netmask_obj != NULL) && json_is_string(netmask_obj))
    {
      if(set_netmask(fd, if_name, json_string_value(netmask_obj)) < 0)
      {
        goto init_end;
      }
    }

    // bring interface up
    if(interface_up(fd, if_name) < 0)
    {
      goto init_end;
    }

    // set default gateway
    if(set_gateway(fd, if_name, gateway) < 0)
    {
      goto init_end;
    }

    // update ipmi lan context
    lan->ip_src = (unsigned char)IPMI_IP_SRC_STATIC;
  }
  else
  {
    syslog(LOG_WARNING, "init_from_nv(): undefined mode error");
    goto init_end;
  }

  json_decref(conf);
  close(fd);
  return 0;

init_end:
  json_decref(conf);
  close(fd);
  return -1;
}
