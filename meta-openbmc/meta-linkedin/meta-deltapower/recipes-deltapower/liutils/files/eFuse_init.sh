#!/bin/bash
PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

for i in `seq 1 48`;
  do
     eFuse_off.sh $i
  done
