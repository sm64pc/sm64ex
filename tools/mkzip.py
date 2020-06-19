#!/usr/bin/env python3

import sys
import os
import zipfile

if len(sys.argv) < 3:
    print('usage: mkzip <lstfile> <zipfile>')
    sys.exit(1)

lst = []
with open(sys.argv[1], 'r') as f:
    for line in f:
        line = line.strip()
        if line == '' or line[0] == '#':
            continue
        tok = line.split()
        lst.append((tok[0], tok[1]))

with zipfile.ZipFile(sys.argv[2], 'w', allowZip64=False) as zipf:
    for (fname, aname) in lst:
        zipf.write(fname, arcname=aname)
