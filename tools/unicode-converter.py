#!/usr/bin/env python3

import sys
import os
import os.path
import glob
import re
from pathlib import Path

exported = {}

def char2unicode(ch):
    if ord(ch) > 128:
        formatted = f"{ord(ch)}".zfill(5)
        if not ch in exported:
            print(f"{ch} - {formatted}")
            exported[ch] = exported
        return "{"+formatted+"}"
    else:
        return f"{ch}"

if len(sys.argv) > 2:
    in_path = sys.argv[1]
    out_path = sys.argv[2]
    lines = open(os.path.abspath(in_path), 'r', encoding='utf8').readlines()
    out = open(os.path.abspath(out_path), 'w')

    for line in lines:
        out.write("".join(map(char2unicode, list(line))))
    out.close()
else:
    print("Use ./tools/unicode-converter.py {in} {out}")
