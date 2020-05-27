#!/usr/bin/env python3

import sys
import os
import glob

if len(sys.argv) < 4:
    print("usage: cleancrcmap <in_map> <out_map> <searchdir>")
    sys.exit(1)

# load and check the old map
searchpath = sys.argv[3]
inmap = list()
with open(sys.argv[1], 'r') as f:
    for line in f:
        line = line.strip()
        if line == '' or line[0] == '#':
            continue
        tok = line.split(',')
        crcstr = tok[0].strip()
        if crcstr.startswith('0x'):
            crc = int(crcstr[2:], 16)
        else:
            crc = int(crcstr)
        tok[1] = tok[1].strip()
        [fname, fext] = os.path.splitext(tok[1])
        [fname, ffmt] = os.path.splitext(fname)
        fname = fname + ffmt[:-1] + '*'
        matches = glob.glob(os.path.join(searchpath, fname))
        if len(matches) == 0:
            print("warning: texture '{0}' does not match anything in '{1}'".format(fname, searchpath))
        else:
            for s in matches:
                tup = (crc, os.path.relpath(s, searchpath))
                if not (tup in inmap):
                    inmap.append(tup)

# save cleaned up version to the new one
with open(sys.argv[2], 'w') as f:
    for (crc, fpath) in inmap:
        f.write("0x{0:08x}, {1}\n".format(crc, fpath))

