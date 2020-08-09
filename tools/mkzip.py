#!/usr/bin/env python3

import sys
import os
import os.path
import zipfile
from shutil import copyfile

if len(sys.argv) < 3:
    print('usage: mkzip <lstfile> <dstpath>')
    sys.exit(1)

lst = []
with open(sys.argv[1], 'r') as f:
    for line in f:
        line = line.strip()
        if line == '' or line[0] == '#':
            continue
        tok = line.split()
        lst.append((tok[0], tok[1]))

    for (fname, aname) in lst:
        path = os.path.join(sys.argv[2], aname)
        if not os.path.exists(path):
            os.makedirs(os.path.dirname(path), exist_ok=True)
            copyfile(fname, path)
            print("Copying: " + path)
        else:
            print("Skipping: " + path)
