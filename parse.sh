#!/bin/bash

set -e
set -x

ABL_PATH=/Users/alexander/Downloads/sw/pda/google/pixel6/oriole-sd1a.210817.015.a4/bootloader-oriole-slider-1.0-7683913.img_images/abl

clang -o app_parse_symtab -O2 -fsanitize=address parse_symtab.c
./app_parse_symtab $ABL_PATH
