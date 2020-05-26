#!/usr/bin/env python3
import sys
if len(sys.argv) < 2:
    print("usage: zeroterm <string>")
else:
    sys.stdout.buffer.write(bytes(sys.argv[1], 'ascii') + b'\x00')
