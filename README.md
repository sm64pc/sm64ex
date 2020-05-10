# sm64pc-testing
OpenGL adaptation of [n64decomp/sm64](https://github.com/n64decomp/sm64). 

Feel free to report bugs and contribute, but remember, there must be **no upload of any copyrighted asset**. 
Run `./extract-assets.py --clean && make clean` or `make distclean` to remove ROM-originated content.

## The testing branch

This branch is mostly about different control systems, at the moment. A new camera system, `better_camera.c`, made by n64decomp dev Fazana, allows for full xy camera movement, both on C-buttons as well as on the right analog stick. PC controller support has been implemented by [vrmiguel](https://github.com/vrmiguel/sm64-analog-camera), and a deadzone fix has been made by [IvanDSM](https://github.com/IvanDSM/).

[IvanDSM](https://github.com/IvanDSM/) has also implemented a new mouse look functionality, currently in experimental phase.

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
  * libglgw3-dev
  * git


__Debian / Ubuntu - targeting 32 bits__
```
sudo apt install build-essential git python3 libaudiofile-dev libglew-dev:i386 libglfw3-dev:i386 libusb-1.0.0:i386 libsdl2-dev:i386
```
__Debian / Ubuntu - targeting 64 bits__
```
sudo apt install build-essential git python3 libaudiofile-dev libglew-dev libglfw3-dev libusb-1.0.0  libsdl2-dev
```

__Arch Linux__
```
sudo pacman -S base-devel python audiofile sdl2 glew 
```
(Note: also install glfw3 and libusb for testing)
__Void Linux - targeting 64 bits__
```
sudo xbps-install -S base-devel python3 audiofile-devel SDL2-devel glew-devel
```
(Note: also install glfw3 and libusb for testing)
__Void Linux - targeting 32 bits__
```
sudo xbps-install -S base-devel python3 audiofile-devel-32bit SDL2-devel-32bit glew-devel-32bit
```
(Note: also install glfw3 and libusb for testing)
#### 3. Build the executable.

Run `make` to build (defaults to `VERSION=us`)

```
make VERSION=jp -j6                 # build (J) version with 6 jobs
make VERSION=us WINDOWS_BUILD=1     # builds a (U) Windows executable
make BETTERCAMERA=0                 # builds an executable with the original SM64 camera (no analog/mouse support)
make TARGET_RPI=1                   # targets an executable for a Raspberry Pi
```

### On Windows

#### 1. Set up MSYS2, following [this  guide](https://github.com/orlp/dev-on-windows/wiki/Installing-GCC--&-MSYS2).

#### 2. Install dependencies
```
pacman -S mingw-w64-i686-glew mingw-w64-x86_64-glew mingw-w64-i686-SDL2 mingw-w64-x86_64-SDL2 python3
```
#### 3. Copy baserom(s) for asset extraction

For each version (jp/us/eu) that you want to build an executable for, put an existing ROM at
`./baserom.<version>.z64` for asset extraction.

#### 4. On MSYS2, navigate to the sm64pc folder and then enter `./tools/audiofile-0.3.6/`. Inside this directory, run
```
autoreconf -i
```

Only leave this directory on step 9.

#### 5. Run the `configure` script
```
PATH=/mingw64/bin:/mingw32/bin:$PATH LIBS=-lstdc++ ./configure --disable-docs
```
#### 6. Run the `make` script
```
PATH=/mingw64/bin:/mingw32/bin:$PATH make
```
#### 7. Create a lib directory in `tools/`
```
mkdir ../lib
```

#### 8. Copy the compiled libaudiofile to `tools/lib/`
```
cp libaudiofile/.libs/libaudiofile.a ../lib/
cp libaudiofile/.libs/libaudiofile.la ../lib/
```

#### 9. Navigate back to `tools/`, then alter the `Makefile` and add `-lstdc++` on the following line
```
tabledesign_CFLAGS := -Wno-uninitialized -laudiofile -lstdc++
```

#### 10. Run `make`
```
PATH=/mingw64/bin:/mingw32/bin:$PATH make
```

#### 11. Navigate back to the sm64pc root directory.

#### 12.  Finally, run `make` once more. 

(Note that mingw32 and mingw64 have been swapped. This is so you can build the 32bit application successfully.)

```
PATH=/mingw32/bin:/mingw64/bin:$PATH make
```

### For the web

The game can be compiled for web browsers that support WebGL using [Emscripten](https://github.com/emscripten-core). To do so, install [emsdk](https://github.com/emscripten-core/emsdk) and run `make TARGET_WEB=1`.

## Optional enhancements

On the `./enhancements` folder, you'll find several .patch files, which can be applied in the following manner:

```
 git apply fps.patch --ignore-whitespace --reject
```
If any rejections occur, you can search for them with `find | grep .rej`.
Try to solve rejections through [wiggle](https://github.com/neilbrown/wiggle).
```
wiggle rejection.rej --replace
```

### Current issues

 * Some Windows builds have been crashing when booting up from an existing save. Linux builds have not yet shown this issue.
 * The Invert Axis option on the in-game menu is broken.
