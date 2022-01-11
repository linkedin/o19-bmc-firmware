#!/bin/sh
check_eth0_ipv4() {
  for i in {1..20};
  do
    ip=$(ifconfig eth0 |grep "inet addr")
    if [[ -z "$ip" ]]; then
      pkill -9 -f dhclient
      sleep 2
      dhclient -d -pf /var/run/dhclient.eth0.pid eth0 > /dev/null 2>&1 &
      sleep 20
    else
      break;
    fi
  done
}

check_eth0_ipv4 &
