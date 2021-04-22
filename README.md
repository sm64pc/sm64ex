# sm64rt

## UNDER CONSTRUCTION

## NOTE: The main dependency of sm64rt, RT64, is currently private until its first public release. For now, this repository only contains the changes required to make the game compatible with it.

Fork of [sm64pc/sm64ex](https://github.com/sm64pc/sm64ex) that adds support for [RT64](https://github.com/DarioSamo/RT64), a hardware-accelerated real-time raytracer.

## Building
For building instructions, please refer to the [sm64ex wiki](https://github.com/sm64pc/sm64ex/wiki) and follow the process as normal with these additional build flags:

* RENDER_API=RT64 (Required to use RT64 as the renderer)

* EXTERNAL_DATA=1 (Required for associating textures to the renderer's material properties)

* NODRAWINGDISTANCE=1 (Optional, but gives good results and prevents pop-in, which can cause issues with objects that cast shadows or appear in reflections)
