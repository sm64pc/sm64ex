# Super Mario 64 Enhancements

This directory contains unofficial patches to the source code that provide various features
and enhancements.

To apply a patch, run `tools/apply_patch.sh [patch]` where `[patch]` is the name of the
.patch file you wish to apply. This will perform all of the patch's changes
to the source code.

Likewise, to undo the changes from a patch you applied, run
`tools/revert_patch.sh` with the name of the .patch file you wish to undo. 

To create your own enhancement patch, switch to the `nightly` Git
branch, make your changes to the code (but do not commit), then run `tools/create_patch.sh`.
Your changes will be stored in the .patch file you specify.

The following enhancements are included in this directory:

## 60 FPS - `60fps_ex.patch`

This allows the game to be rendered at 60 FPS instead of 30 FPS by interpolation (game logic still runs at 30 FPS).

The Mario head intro is the only exception which is still rendered at 30 FPS.

This is the 60fps patch from [sm64-port](https://github.com/sm64-port/sm64-port/tree/master/enhancements) adapted for sm64ex.
