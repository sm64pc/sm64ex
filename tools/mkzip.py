#!/usr/bin/env python3

import sys
import os
import json
import os.path as path
from shutil import copyfile
import hashlib

def md5(fname):
    md5_hash = hashlib.md5()
    a_file = open(fname, "rb")
    content = a_file.read()
    md5_hash.update(content)
    return md5_hash.hexdigest()

moonFolder = "addons/moon64/"
bitProperties = {
    "bit": {
        "name": "Moon64",
        "authors": [ "Nintendo" ],
        "description": "SM64 Original Textures",
        "version": 1.0,
        "readOnly": True
    }
}

lst = []
baseAddon = path.join(sys.argv[2], moonFolder)

with open(sys.argv[1], 'r') as f:
    for line in f:
        data = line.strip().split()
        out = path.join(baseAddon, "assets", data[1]).replace("gfx", "graphics").replace("texts", "langs")
        os.makedirs(path.dirname(out), exist_ok=True)
        if((path.exists(out) and path.exists(data[0]) and md5(out) != md5(data[0])) or not path.exists(out)):
            copyfile(data[0], out)
            print(f"Copying: {data[0]}")

with open(path.join(baseAddon, "properties.json"), 'w') as fp:
    json.dump(bitProperties, fp, indent=4)