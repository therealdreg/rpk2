#!/bin/bash

source device_sd.txt


dd if=/dev/zero of=$sd_dev status=progress bs=2048
