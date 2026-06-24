# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Beacon_SuperCap)

target_sources(app PRIVATE src/main.c)
target_include_directories(app PRIVATE inc)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/NfcTag.c)
target_sources(app PRIVATE src/GDEW0154M10.c)
target_sources(app PRIVATE src/epaper.c)
target_sources(app PRIVATE src/qrcode.c)