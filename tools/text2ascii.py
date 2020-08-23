#!/usr/bin/env python3

import sys
import os
import os.path
import glob
import re
from pathlib import Path

path = "/home/alex/Documents/Projects/Render96/Render96ex - FastBuild/charmap.txt"
lines = open(os.path.abspath(path), 'r').readlines()

lineID = {}

for lineIndex, uline in enumerate(lines, 0):
    line = uline.strip()
    if(line.startswith("'")):
        unf = line.split(" = ")
        char = unf[0].replace("'", "")
        value = unf[1] if len(unf[1].split(", ")) < 2 else unf[1].split(", ")[1]
        lineID[lineIndex] = "{"+f"\"{char}\", {value}"+"}, "

print(f"Array size: {len(lineID)}")

lines = 0
counter = 0

for b in lineID: 
    a = lineID[b]
    print(a, end = '' if counter < 8 else '\n')
    counter += 1
    lines += 1
    if counter > 8:
        counter = 0
print('\n')
print(f' LINES {lines}\n')