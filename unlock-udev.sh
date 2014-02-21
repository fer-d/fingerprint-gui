#!/bin/bash

if [ "$(id -u)" == "0" ]; then
   echo "This script must be run as a user (not root)" 1>&2
   exit 1
fi

device=`lsusb | grep -i upek`
if [ -z "$device" ]; then
  echo "UPEK FingerPrint sensor are no found!"
  exit 1
fi

echo "Found: $device"
bus=`echo $device | cut -f 2 -d ' '`
number=`echo $device | cut -f 4 -d ' ' | cut -f 1 -d :`

usr=`whoami`
sudo usermod -a -G plugdev $usr

sudo chmod 666 /dev/bus/usb/$bus/$number

exit 0