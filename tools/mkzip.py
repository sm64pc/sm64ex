#!/usr/bin/env python3

import sys
import os
import os.path
import zipfile
from shutil import copyfile
import hashlib

def md5(fname):
    hash_md5 = hashlib.md5()
    with open(fname, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()

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
        old_md5 = md5(fname);
        if not os.path.exists(path) or os.path.exists(path) and old_md5 != md5(path):
            os.makedirs(os.path.dirname(path), exist_ok=True)
            copyfile(fname, path)
            print("Copying: " + path)
        else:
            print("Skipping: " + path + " - MD5: "+md5(fname))
