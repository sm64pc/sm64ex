# Super Mario 64 Enhancements

This directory contains unofficial patches to the source code that provide various features
and enhancements.

To apply a patch, run `tools/apply_patch.sh [patch]` where `[patch]` is the name of the
.patch file you wish to apply. This will perform all of the patch's changes
to the source code.

Likewise, to undo the changes from a patch you applied, run
`tools/revert_patch.sh` with the name of the .patch file you wish to undo. 

To create your own enhancement patch, switch to the `master` Git
branch, make your changes to the code (but do not commit), then run `tools/create_patch.sh`. Your changes will be stored in the .patch file you specify. 

The following enhancements are included in this directory:

## Debug Box - `debug_box.patch`

This allows you to draw 3D boxes for debugging purposes.

Call the `debug_box` function whenever you want to draw one. `debug_box` by default takes two arguments: a center and bounds vec3f. This will draw a box starting from the point (center - bounds) to (center + bounds).
Use `debug_box_rot` to draw a box rotated in the xz-plane. If you want to draw a box by specifying min and max points, use `debug_box_pos` instead.

## Demo Input Recorder - `record_demo.patch`

This patch allows you to record gameplay demos for the attract screen. It requires the latest nightly versions of Project64, and uses the Project64 JavaScript API to dump the demo input data from RAM and write it to a file.

Place the `enhancements/RecordDemo.js` file in the `/Scripts/` folder in the Project64 directory.

In the Scripts window, double click on "RecordDemo" on the list on the left side.

When this is done, it should turn green which lets you know that it has started.

When your demo has been recorded, it will be dumped to the newly created `/SM64_DEMOS/` folder within the Project64 directory.
