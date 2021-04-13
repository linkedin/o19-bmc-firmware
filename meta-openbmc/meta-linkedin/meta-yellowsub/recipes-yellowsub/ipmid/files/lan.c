/*
 * lan.c
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

#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <linux/if_link.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <openbmc/ipmi.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include <linux/if_vlan.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netinet/ether.h>
#include <linux/route.h>
#include <fcntl.h>
#include <facebook/ethcfg.h>

#define BUFSIZE 8192
#define CMD_STR_SIZE  120

#define IPV6_LINK_LOCAL_BYTE1 0xFE
#define IPV6_LINK_LOCAL_BYTE2 0x80

#define BYTE_MASK 0xFF
#define BYTE1_OFFSET 8
#define BYTE2_OFFSET 16
#define BYTE3_OFFSET 24

/*
 * IP Address Source
 */
#define    IPMI_IP_SRC_UNSPECIFIED         0
#define    IPMI_IP_SRC_STATIC              1
#define    IPMI_IP_SRC_DHCP                2

static inline void init_sockaddr_in(struct sockaddr_in *sin, const char *addr)
{
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    sin->sin_addr.s_addr = inet_addr(addr);
}

static int setnetmask(int s, struct ifreq *ifr, const char *newmask)
{
    init_sockaddr_in((struct sockaddr_in *) &ifr->ifr_netmask, newmask);
    if(ioctl(s, SIOCSIFNETMASK, ifr) < 0) {
        syslog(LOG_WARNING, "setnetmask(): if_name: %s SIOCSIFNETMASK error", ifr->ifr_name);
        return -1;
    }

    eth_conf_save_string("netmask", newmask);
    return 0;
}

static int setaddr(int s, struct ifreq *ifr, const char *addr)
{
    init_sockaddr_in((struct sockaddr_in *) &ifr->ifr_addr, addr);
    if(ioctl(s, SIOCSIFADDR, ifr) < 0) {
        syslog(LOG_WARNING, "setaddr(): if_name: %s SIOCSIFADDR error", ifr->ifr_name);
        return -1;
    }

    eth_conf_save_string("ip_addr", addr);
    return 0;
}

int set_default_gateway(int sockfd, char *if_dev, const char *addr)
{
    struct rtentry rt;
    struct sockaddr_in *dst, *gw, *mask;

    struct sockaddr_in *sockinfo = (struct sockaddr_in *)&rt.rt_gateway;
    memset(&rt,0,sizeof(struct rtentry));

    dst = (struct sockaddr_in *)(&(rt.rt_dst));
    gw = (struct sockaddr_in *)(&(rt.rt_gateway));
    mask = (struct sockaddr_in *)(&(rt.rt_genmask));

    dst->sin_family = AF_INET;
    gw->sin_family = AF_INET;
    mask->sin_family = AF_INET;

    /*
     * Add new default route
     */
    init_sockaddr_in(sockinfo, addr);

    dst->sin_addr.s_addr = 0;
    mask->sin_addr.s_addr = 0;
    rt.rt_flags = RTF_UP | RTF_GATEWAY;

    ioctl(sockfd,SIOCDELRT,&rt);

    rt.rt_dev = if_dev;
    if(ioctl(sockfd, SIOCADDRT, &rt) < 0 ) {
        syslog(LOG_WARNING, "set_default_gateway: if_name: %s SIOCADDRT error", if_dev);
        return -1;
    }
    eth_conf_save_string("gateway", addr);
    return 0;
}

void plat_lan_init(lan_config_t *lan)
{
  struct ifaddrs *ifaddr, *ifa;
  struct sockaddr_in *addr;
  struct sockaddr_in6 *addr6;
  int family, n;
  unsigned long ip;
  unsigned char *ip6;
  int sd;
  struct ifreq ifr;
  uint8_t eui_64_addr[8] = {0x0};
  struct vlan_ioctl_args if_request;
  FILE* fp = NULL;
  char cmd_dft[CMD_STR_SIZE];
  char line[64];
  bool ipv4_populated = false;
  bool ipv6_populated = false;

  if (init_from_nv(lan) < 0) {
    syslog(LOG_WARNING, "plat_lan_init(): init from non volatile error\n");
  }

  if (getifaddrs(&ifaddr) == -1) {
    return;
  }

  sd = socket(PF_INET, SOCK_DGRAM, 0);

  if (sd  < 0) {
      syslog(LOG_WARNING, "Couldn't open a socket\n");
      goto init_done;
  }

  for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
    if (ifa->ifa_addr == NULL) {
      continue;
    }

    family = ifa->ifa_addr->sa_family;

    if ((family == AF_INET) && !ipv4_populated) {
      addr = (struct sockaddr_in*) ifa->ifa_addr;
      ip = addr->sin_addr.s_addr;

      /*
       * copy the ip address from long to byte array with MSB first
       */
      lan->ip_addr[3] = (ip >> BYTE3_OFFSET) & BYTE_MASK;
      lan->ip_addr[2] = (ip >> BYTE2_OFFSET) & BYTE_MASK;
      lan->ip_addr[1] = (ip >> BYTE1_OFFSET) & BYTE_MASK;
      lan->ip_addr[0] = ip & BYTE_MASK;

      if (!strcmp(ifa->ifa_name, "lo") || !strcmp(ifa->ifa_name, "eth1")) {
          continue;
      }

      if (strcmp(ifa->ifa_name, (char*)lan->if_name)) {
          continue;
      }

      ipv4_populated = true;

      strcpy(ifr.ifr_name, ifa->ifa_name);

      /*
       * Get VLAN Id
       */
      memset(&if_request, 0, sizeof(if_request));

      if_request.cmd = GET_VLAN_VID_CMD;
      strcpy(if_request.device1, ifa->ifa_name);

       if (ioctl(sd, SIOCSIFVLAN, &if_request) != -1) {
           lan->vlanId = if_request.u.VID;
           lan->vlan_enable = 1;
           strcpy((char*)lan->if_name, ifa->ifa_name);
           if (lan->channel == 8)
               strcpy(if_request.device1, ifa->ifa_name);
      }
      else {
          lan->vlan_enable = 0;
      }

      /*
       *  Get the MAC address
       */
      if(ioctl(sd, SIOCGIFHWADDR, &ifr) != -1) {
          uint8_t* mac_addr = (uint8_t*)ifr.ifr_hwaddr.sa_data;
          memcpy((void *)&lan->mac_addr[0], (void *)&mac_addr[0], SIZE_MAC_ADDR);
      }

      /*
       * Get Net mask
       */
      if(ioctl(sd, SIOCGIFNETMASK, &ifr) != -1) {
          uint8_t* net_mask = (uint8_t*)ifr.ifr_netmask.sa_data + 2;
          memcpy((void *)&lan->net_mask[0], (void *)&net_mask[0], SIZE_NET_MASK);
      }

      /*
       * get default getaway
       */
      snprintf(cmd_dft, CMD_STR_SIZE, "route -n | grep %s  | grep 'UG[ \t]' | awk '{print $2}'", lan->if_name);
      fp = popen(cmd_dft, "r");

      if(fgets(line, sizeof(line), fp) != NULL) {
            syslog(LOG_WARNING, "default getaway: %s", line);
            int i = 0;
            char* buff = malloc(10);
            buff = strtok(line,".");
            while (buff != NULL)
            {
                lan->df_gw_ip_addr[i] = (unsigned char)atoi(buff);
                buff = strtok(NULL,".");
                i++;
            }

            char* default_getaway = malloc(INET_ADDRSTRLEN);
            snprintf(default_getaway, INET_ADDRSTRLEN, "%u.%u.%u.%u", (uint8_t)lan->df_gw_ip_addr[0],
                     (uint8_t)lan->df_gw_ip_addr[1], (uint8_t)lan->df_gw_ip_addr[2], (uint8_t)lan->df_gw_ip_addr[3]);

            free(buff);
            pclose(fp);
      }
    } else if ((family == AF_INET6) && !ipv6_populated) {
      addr6 = (struct sockaddr_in6*) ifa->ifa_addr;
      ip6 = addr6->sin6_addr.s6_addr;

      // If the address is Link Local, Ignore it
      if ((ip6[0] == IPV6_LINK_LOCAL_BYTE1) && (ip6[1] == IPV6_LINK_LOCAL_BYTE2)) {
        continue;
      }

      // Get the MAC address
      strcpy(ifr.ifr_name, ifa->ifa_name);
      if(ioctl(sd, SIOCGIFHWADDR, &ifr) != -1) {
        uint8_t* mac_addr = (uint8_t*)ifr.ifr_hwaddr.sa_data;

        /*
         * SLAAC address has lower 8B as follows:
         * 3B == First 24b MAC address
         * 2B == FFFE
         * 3B == Last 24b MAC address
         */
        memcpy((void *)&eui_64_addr[0], (void *)&mac_addr[0], 3);
        eui_64_addr[3] = 0xFF;
        eui_64_addr[4] = 0xFE;
        memcpy((void *)&eui_64_addr[5], (void *)&mac_addr[3], 3);
        eui_64_addr[0] += 2;

        // Check if the address is SLAAC address. If yes, skip it.
        if (!memcmp((void *)&ip6[8], (void *)eui_64_addr, 8)) {
          continue;
        }
      }

      // copy the ip address from array with MSB first
      memcpy(lan->ip6_addr, ip6, SIZE_IP6_ADDR);
      ipv6_populated = true;
    }
  }

  // close socket descriptor
  close(sd);

init_done:
  freeifaddrs(ifaddr);
}

int plat_lan_set(lan_config_t *lan, uint8_t option)
{
   char ip_addr[INET_ADDRSTRLEN];
   char netmask[INET_ADDRSTRLEN];
   char default_getaway[INET_ADDRSTRLEN];
   char cmd[CMD_STR_SIZE];
   int  sockfd = 0;
   int  status = 0;

   struct ifreq ifr;
   struct sockaddr_in sin;
   struct vlan_ioctl_args if_request;

   if (!lan) {
     return -1;
   }

   sockfd = socket(AF_INET, SOCK_DGRAM, 0);
   if(sockfd < 0) {
       syslog(LOG_WARNING, "Cannot open control socket");
       return -1;
   }

   memset(&if_request, 0, sizeof(if_request));
   memset(&ifr, 0, sizeof(ifr));
   memset(&sin, 0, sizeof(sin));

   strncpy(ifr.ifr_name, (char*) lan->if_name, IFNAMSIZ);
   ifr.ifr_name[IFNAMSIZ-1] = 0;

   snprintf(ip_addr, INET_ADDRSTRLEN, "%u.%u.%u.%u", (uint8_t)lan->ip_addr[0], (uint8_t)lan->ip_addr[1],
            (uint8_t)lan->ip_addr[2], (uint8_t)lan->ip_addr[3]);

   snprintf(default_getaway, INET_ADDRSTRLEN, "%u.%u.%u.%u", (uint8_t)lan->df_gw_ip_addr[0],
            (uint8_t)lan->df_gw_ip_addr[1], (uint8_t)lan->df_gw_ip_addr[2], (uint8_t)lan->df_gw_ip_addr[3]);

   snprintf(netmask, INET_ADDRSTRLEN, "%u.%u.%u.%u", (uint8_t)lan->net_mask[0], (uint8_t)lan->net_mask[1],
           (uint8_t)lan->net_mask[2], (uint8_t)lan->net_mask[3]);

   switch (option) {
   case LAN_PARAM_IP_ADDR:
       /*
        * set ip address
        */
       if (setaddr(sockfd, &ifr, (char*)ip_addr) < 0) {
           status = -1;
           break;
        }

       status = set_default_gateway(sockfd, ifr.ifr_name, default_getaway);
       break;
   case LAN_PARAM_NET_MASK:
       /*
        * set netmask
        */
       status = setnetmask(sockfd, &ifr, (char*)netmask);
       break;
   case LAN_PARAM_DF_GW_IP_ADDR:
       status = set_default_gateway(sockfd, ifr.ifr_name, default_getaway);
       break;
   case LAN_PARAM_IP_SRC:
       if ((lan->ip_src & 0xf) != IPMI_IP_SRC_STATIC) {
           snprintf(cmd, CMD_STR_SIZE, "pkill -9 dhclient");
           status = system(cmd);
           if (status < 0) {
              syslog(LOG_WARNING, " dhcp disable error");
              break;
           }
           status = setaddr(sockfd, &ifr, "0.0.0.0");
           if (status < 0)
           {
              break;
           }

           snprintf(cmd, CMD_STR_SIZE, "dhclient %s", lan->if_name);
           status = system(cmd);
           if(status < 0) {
              break;
           }
           eth_conf_save_string("mode", "dhclient");
           eth_conf_delete_key("netmask");
       }
       else {
           /*
            * set ip address and default gateway
            */
           snprintf(cmd, CMD_STR_SIZE, "pkill -9 dhclient");
           status = system(cmd);
           if (status < 0) {
               syslog(LOG_WARNING, " dhcp disable error");
               break;
           }
           status = setaddr(sockfd, &ifr, "0.0.0.0");
           if (status < 0)
           {
              break;
           }

           status = setaddr(sockfd, &ifr, (char*)ip_addr);
           if (status < 0)
               break;

           eth_conf_save_string("ip_addr", (char*)ip_addr);

           status = set_default_gateway(sockfd, ifr.ifr_name, default_getaway);
           if(status <0)
           {
               break;
           }
           eth_conf_save_string("mode", "static");
       }
       break;
   case LAN_PARAM_VLAN_ID:
       /*
        * disable VLAN
        */
       if (!lan->vlan_enable) {
            snprintf(cmd, CMD_STR_SIZE, "ip link delete dev %s", lan->if_name);
            status = system(cmd);
            if (status < 0) {
                syslog(LOG_WARNING, " ip link delete error");
                break;
            }
            snprintf((char*)lan->if_name, IFNAMSIZ, "%s", ETH_INTF_NAME);
            eth_conf_delete_key("vlanid");
       }

       /*
        * Enable VLAN
        */
       if (lan->vlan_enable) {
          snprintf(cmd, CMD_STR_SIZE, "pkill -9 dhclient");
          status = system(cmd);
          if (status < 0) {
              syslog(LOG_WARNING, " dhcp disable error");
              break;
          }
          status = setaddr(sockfd, &ifr, "0.0.0.0");
          if (status < 0)
              break;
           memset(if_request.device1, 0, sizeof(if_request.device1));
           snprintf((char*)lan->if_name, IFNAMSIZ, "%s.%d", ETH_INTF_NAME, lan->vlanId);
           strncpy(if_request.device1, ETH_INTF_NAME, IFNAMSIZ);
           if_request.u.VID = lan->vlanId;
           if_request.cmd = ADD_VLAN_CMD;
           if (ioctl(sockfd, SIOCSIFVLAN, &if_request) < 0) {
               syslog(LOG_WARNING, "VLAN enable error");
               status = -1;
               break;
           }
           eth_conf_save_int("vlanid", lan->vlanId);
       }

       snprintf(cmd, CMD_STR_SIZE, "pkill -9 dhclient");
       status = system(cmd);
       if (status < 0) {
           syslog(LOG_WARNING, " dhcp disable error");
           break;
       }
       if ((lan->ip_src & 0xf) == IPMI_IP_SRC_STATIC) {
          status = setaddr(sockfd, &ifr, "0.0.0.0");
          if (status < 0)
              break;
           snprintf(cmd, CMD_STR_SIZE, "ip link set dev %s up", lan->if_name);
           status = system(cmd);
            if (status < 0) {
                syslog(LOG_WARNING, " ip link set dev error");
                break;
            }

           status = set_default_gateway(sockfd, ifr.ifr_name, default_getaway);
           if (status < 0)
               break;
       eth_conf_save_string("mode", "static");
       }

       snprintf(cmd, CMD_STR_SIZE, "dhclient %s", lan->if_name);
       status = system(cmd);
       if (status < 0) {
            syslog(LOG_WARNING, " ip link delete error");
            break;
        }

       eth_conf_save_string("mode", "dhclient");
       eth_conf_delete_key("netmask");

       close(sockfd);
       return status;
   default:
       syslog(LOG_WARNING, "wrong option");
       close(sockfd);
       return -1;
   }

   close(sockfd);
   return status;
}
