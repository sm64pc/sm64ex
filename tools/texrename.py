#!/usr/bin/env python3

import sys
import os
import shutil

if len(sys.argv) < 3:
    print("usage: texrename <in_dir> <out_dir> [<crcmap_file>]")
    sys.exit(1)

inpath = sys.argv[1]
outpath = sys.argv[2]
mapfname = "crcmap.txt"
if len(sys.argv) > 3:
    mapfname = sys.argv[3]

# catalog the original texture pack
texmap = dict()
imgexts = frozenset(['.png', '.bmp', '.jpg', '.tga', '.gif'])
try:
    for root, dirs, files in os.walk(inpath):
        for f in files:
            ffull = os.path.join(root, f)
            [fpath, fname] = os.path.split(f)
            ext = os.path.splitext(fname)[1].lower()
            if fname[0] == '.' or not (ext in imgexts):
                continue
            crc = 0
            try:
                if '#' in fname: # rice pack format: "GAME NAME#hash#whatever"
                    crc = int(fname.split('#')[1], 16)
                else: # just the crc probably
                    crc = int(os.path.splitext(fname)[0], 16)
            except ValueError:
                print('unknown filename format: {0}'.format(ffull))
                continue
            texmap[crc] = ffull
except OSError as e:
    print('error opening {0}: {1}'.format(inpath, e))
    sys.exit(2)

# load the CRC map
crcmap = list()
try:
    with open(mapfname, 'r') as f:
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
            crcmap.append((crc, os.path.join(outpath, 'gfx', tok[1].strip())))
except OSError as e:
    print('could not open {0}: {1}'.format(mapfname, e))
except ValueError as e:
    print('invalid integer in {0}: {1}'.format(mapfname, e))
    sys.exit(3)

# copy the files to the correct locations
for (crc, path) in crcmap:
    if not (crc in texmap):
        print('unmatched CRC: {0} ({1})'.format(crc, path))
    else:
        [fpath, fname] = os.path.split(path)
        if not os.path.exists(fpath):
            os.makedirs(fpath)
        shutil.copy2(texmap[crc], path)
