# sm64pc
OpenGL adaptation of [n64decomp/sm64](https://github.com/n64decomp/sm64). 

Feel free to report bugs and contribute, but remember, there must be **no upload of any copyrighted asset**. 
Run `./extract-assets.py --clean && make clean` or `make distclean` to remove ROM-originated content.

## Building

### On Linux

#### 1. Copy baserom(s) for asset extraction

For each version (jp/us/eu) that you want to build an executable for, put an existing ROM at
`./baserom.<version>.z64` for asset extraction.

#### 2. Install build dependencies

The build system has the following package requirements:
  * python3 >= 3.6
  * libsdl2-dev
  * [audiofile](https://audiofile.68k.org/)
  * libglew-dev
  * git


__Debian / Ubuntu - targeting 32 bits__
```
sudo apt install build-essential git python3 libaudiofile-dev libglew-dev:i386 libsdl2-dev:i386
```
__Debian / Ubuntu - targeting 64 bits__
```
sudo apt install build-essential git python3 libaudiofile-dev libglew-dev libsdl2-dev
```

__Arch Linux__
```
sudo pacman -S base-devel python audiofile sdl2 glew
```

#### 3. Build the executable.

Run `make` to build (defaults to `VERSION=us`)

```
make VERSION=jp -j6                 # build (J) version with 6 jobs
make VERSION=us WINDOWS-BUILD=1     # builds a (U) Windows executable 
```

### On Windows

Install WSL with a distro of your choice (Ubuntu 18.04 recommended) following [the official guide](https://docs.microsoft.com/en-us/windows/wsl/install-win10).


Then follow the instructions of the Linux section above. It's also possible to build using [MinGW](http://www.mingw.org/), but it tends to be considerably more difficult.

### Current issues

 * Support for the EU version is still experimental.
 * There seems to be savedata-related problems on some 64-bits builds.
 * Camera controls are also bugged for some.
