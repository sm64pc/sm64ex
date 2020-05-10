# Text-based savedata

This is a very early WIP implementation of text-based save files. So far, only saving works.

Here's an example of a save file:

`write_text_data` creates a file named `experimental_save_file.sm64s`, which contains the data from `gSaveBuffer` we believe would be necessary to reconstruct a 
save file. While this has not been integrated with the rest of the code yet, please feel free to contribute! 

Just drop `save_file.c` into src/game to try it out.

