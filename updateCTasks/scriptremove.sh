#!/bin/bash
CREATE="/home/main/Source/C/env/createProject/Tasks-C-global/create.sh"
grep -v "$1" $CREATE > temp && mv temp $CREATE