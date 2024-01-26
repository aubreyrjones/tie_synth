#!/usr/bin/env python3

import re

with open('gui_gen.icc', 'r', encoding='utf8') as source:
    lines = source.readlines()

first_line = -1
last_line = -1

for i, line in enumerate(lines):
    if line.startswith("// GUItool: begin"):
        first_line = i
    if line.startswith("// GUItool: end"):
        last_line = i    

target_lines = lines[(first_line + 1) : last_line]


print("""#pragma once
#include <Arduino.h>
#include "../ext/Audio/Audio.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
      
""")

name_regex = re.compile(r"^\w+")

for l in target_lines:
    tokens = l.split()
    name = name_regex.search(tokens[1]).group()
    print(f"extern {tokens[0]} {name};")
