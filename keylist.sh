#!/bin/bash
dumpkeys | grep '^keycode' | sed 's/=//' | sed 's/  / /' | sed 's/  / /' | sed 's/keycode //' | awk -F " " '{print $1,$2}'
