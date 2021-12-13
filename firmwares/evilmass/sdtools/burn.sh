#!/bin/bash

source device_sd.txt

if [ -z "$1" ]
  then
    echo "No argument supplied, syntax: ./burn.sh sd_image_name.img"
	exit 1
fi

dd if=$1 of=$sd_dev bs=2048 status=progress
