#!/bin/bash
CREATE="/home/main/Source/C/env/createProject/Tasks-C-global/create.sh"
grep -v "$1" $CREATE > /tmp/create_tmp && cat /tmp/create_tmp > $CREATE
rm /tmp/create_tmp