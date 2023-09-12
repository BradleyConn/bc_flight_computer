#!/usr/bin/bash

#/usr/bin/date >> /tmp/udev.log
#!/bin/bash

#if [ -f /tmp/lock.txt ]; then
#   echo "Lock file already exists, exiting." >> /tmp/udev.log
#   exit 1
#lse
#   echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" >> /tmp/udev.log
#   echo "Sleep 10" >> /tmp/udev.log
#   sleep 10;
#   touch /tmp/lock.txt
#   echo "Lock file created." >> /tmp/udev.log
#   echo "Sleep 10" >> /tmp/udev.log
#   sleep 10;
#
#   #cp "u2f.txt" "/media/usb"


# This is what works. Uncomment this if it's desired.
#   echo "Sleep 3"
#   sleep 2;
#   echo "Copying..."
#   cp /home/bradley/git_projects/bc_flight_computer/Software/examples/pico-examples/build/blink/blink.uf2 /media/bradley/RPI-RP2/
#   echo "Copied!"
##   echo "new uf2 copied to /media/bradley/RPI-RP2" >> /tmp/udev.log
##   echo "Sleep 10" >> /tmp/udev.log
##   sleep 10;
##
##   rm /tmp/lock.txt
##   echo "Lock file deleted." >> /tmp/udev.log
##i
