#!/bin/sh
FILE=/mnt/data/config/configure_host
if [ -f "$FILE" ]; then
    echo "$FILE exist"
    useradd -G root readings
    $FILE
fi