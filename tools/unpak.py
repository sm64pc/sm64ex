#!/usr/bin/env python3

# requires Pillow and zstandard for Python
# on msys, install Pillow with
#   pacman -S mingw-w64-x86_64-python-pillow
# zstd needs to be compiled, because the one in pip3 fails to do so:
#   pacman -S mingw-w64-x86_64-python-setuptools mingw-w64-x86_64-zstd
#   git clone https://github.com/indygreg/python-zstandard.git --recursive && cd python-zstandard
#   python setup.py build_ext --external clean
# run like this:
#   ./unpak.py pakfile.pak outdir tools/default_crcmap.txt
# any files not found in crcmap will go to the "nonmatching" folder

import os
import sys
import zstd
import struct
from PIL import Image

PAK_MAGIC = b'\x11\xde\x37\x10\x68\x75\xb6\xe8'

if len(sys.argv) < 3:
    print('usage: unpak <input.pak> <outdir> [<crcmap>]')
    sys.exit(1)

pakfname = sys.argv[1]
outpath = sys.argv[2]
mapfname = "crcmap.txt"
if len(sys.argv) > 3:
    mapfname = sys.argv[3]

# load the CRC map
crcmap = dict()
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
            path = os.path.join(outpath, tok[1].strip())
            if crc in crcmap:
                crcmap[crc].append(path)
            else:
                crcmap[crc] = [path]
except OSError as e:
    print('could not open {0}: {1}'.format(mapfname, e))
    sys.exit(2)
except ValueError as e:
    print('invalid integer in {0}: {1}'.format(mapfname, e))
    sys.exit(2)

unmatchdir = os.path.join(outpath, "nonmatching")
if not os.path.exists(unmatchdir):
    os.makedirs(unmatchdir)

# read the PAK
try:
    texlist = []
    with open(pakfname, "rb") as f:
        magic = f.read(len(PAK_MAGIC))
        if magic != PAK_MAGIC:
            print('invalid magic in PAK ' + pakfname)
            sys.exit(3)
        texcount = int.from_bytes(f.read(8), byteorder='little')
        print('reading {0} textures from {1}'.format(texcount, pakfname))
        for i in range(texcount):
            crc = int.from_bytes(f.read(4), byteorder='little')
            size = int.from_bytes(f.read(4), byteorder='little')
            offset = int.from_bytes(f.read(8), byteorder='little')
            width = int.from_bytes(f.read(8), byteorder='little')
            height = int.from_bytes(f.read(8), byteorder='little')
            texlist.append((crc, size, offset, width, height))
        for (crc, size, ofs, w, h) in texlist:
            f.seek(ofs)
            data = f.read(size)
            img = Image.frombytes('RGBA', (w, h), zstd.decompress(data))
            if crc in crcmap:
                for path in crcmap[crc]:
                    [fpath, fname] = os.path.split(path)
                    if not os.path.exists(fpath):
                        os.makedirs(fpath)
                    img.save(path)
            else:
                print('unknown crc: {0:08x}'.format(crc))
                path = os.path.join(unmatchdir, "{0:08x}.png".format(crc))
                img.save(path)
except OSError as e:
    print('could not open {0}: {1}'.format(pakfname, e))
    sys.exit(3)
