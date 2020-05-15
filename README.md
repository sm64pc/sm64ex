# sm64pc
OpenGL adaptation of [n64decomp/sm64](https://github.com/n64decomp/sm64). 

Feel free to report bugs and contribute, but remember, there must be **no upload of any copyrighted asset**. 
Run `./extract-assets.py --clean && make clean` or `make distclean` to remove ROM-originated content. This port has been made possible mostly thanks to [Emill](https://github.com/Emill) and his [n64-fast32-engine](https://github.com/Emill/n64-fast3d-engine/) renderer.

*Read this in other languages: [Español](README_es_ES.md) [简体中文](README_zh_CN.md).*

## Features

 * Native rendering. You can now play SM64 without the need of an emulator. 
 * Variable aspect ratio and resolution. The game can now correctly render at basically any window size.
 * Native xinput controller support. On Linux, DualShock 4 has been confirmed to work plug-and-play.
 * Analog camera control and mouse look. (Activate with `make BETTERCAMERA=1`.)
 * An option to disable drawing distances. (Activate with `make NODRAWINGDISTANCE=1`.)
 * In-game control binding, currently available on the `testing` branch.
 * Skip introductory Peach & Lakitu cutscenes with the `--skip-intro` CLI option

## Building
For building instructions, please refer to the [wiki](https://github.com/sm64pc/sm64pc/wiki).

**Do NOT attempt to compile Windows executables with `WINDOWS_BUILD=1` under Linux or WSL. It will NOT work. Follow the guide on the wiki.**
