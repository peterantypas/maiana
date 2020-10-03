#!/bin/bash

BIN=ais_transponder.bin

# Write the metadata first, before any reset


if [ $? -eq 0 ] then
    st-flash write $BIN 0x08004000    
else
    echo "Flashing failed"
fi
