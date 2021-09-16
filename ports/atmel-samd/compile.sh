#/bin/bash

make V=1 BOARD=metro_m0_express CC=/opt/gcc-arm-none-eabi-9/bin/arm-none-eabi-gcc "$@"
