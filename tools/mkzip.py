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

if len(sys.argv) < 4:
    print('usage: mkzip <lstfile> <dstpath> <legacy>')
    sys.exit(1)

lst = []
with open(sys.argv[1], 'r') as f:
    for line in f:
        line = line.strip()
        if line == '' or line[0] == '#':
            continue
        tok = line.split()
        lst.append((tok[0], tok[1]))
    if not sys.argv[3] or not sys.argv[3] == "1":
        for (fname, aname) in lst:
            path = os.path.join(sys.argv[2], aname)
            old_md5 = md5(fname)
            if not os.path.exists(path) or os.path.exists(path) and old_md5 != md5(path):
                os.makedirs(os.path.dirname(path), exist_ok=True)
                copyfile(fname, path)
                print("Copying: " + path)
            else:
                print("Skipping: " + path + " - MD5: "+md5(fname))
    else:
        regenZip = False
        zipPath = os.path.join(sys.argv[2],"legacy.zip")
        print("Using Legacy System")
        if os.path.exists(zipPath):
            with zipfile.ZipFile(zipPath, 'w', allowZip64=False) as zipf:
                for(zinfo) in zipf.infolist():
                    with zipf.open(zinfo) as tmp:
                        print("Pre Loaded: "+md5(tmp.read()))
                        if md5(fname) != md5(tmp.read()):
                            print("Found a change on the zip, regenerating")
                            regenZip = True
                            break
        else:
            regenZip = True
        if regenZip:
            with zipfile.ZipFile(zipPath, 'w', allowZip64=False) as zipf:
                for (fname, aname) in lst:
                    zipf.write(fname, arcname=aname)
                    print("Copying: " + aname)