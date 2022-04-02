#!/bin/bash
echo "Building EldenKeys ..."
gcc EldenKeys.c -lX11 -lc -lXmu -o EldenKeys
if [ $? == 0 ]; then
echo "Building EldenKeys ... DONE"
else
echo "Building EldenKeys ... FAILED"
fi

