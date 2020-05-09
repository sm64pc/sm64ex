# L-trigger mapping

Some parts of the code might require the pressing of the L-trigger for testing reasons. 

If you need that, alter `controller_sdl.c`.
In the following line:
```
    if (SDL_GameControllerGetButton(sdl_cntrl, SDL_CONTROLLER_BUTTON_LEFTSHOULDER))    pad->button |= Z_TRIG;
```
Replace `Z_TRIG` with `L_TRIG`, save and rebuild.

On a DS4, this now means that Z-trigger will be mapped to L2 and the L-trigger to L1.
