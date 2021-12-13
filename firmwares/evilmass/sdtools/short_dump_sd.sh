#!/bin/bash

source device_sd.txt

dd of="$(date '+short_%Y-%m-%d--%H-%M-%S').img" if=$sd_dev status=progress bs=1M count=10
