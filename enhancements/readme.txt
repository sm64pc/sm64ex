Puppycam 2.0 release notes:

- Reworked Camera mode system, utilising behaviour flags, allowing for new camera modes to be created easier than ever before. bettercamera.h contains all modes, with their appropriate flags.

- Reworked Collision. Puppycam now utilises CuckyDev's Raycasting system, which offers far more reliable collision checking as well as increased performance. The major change that comes with this however, is that the game must now be compiled with -O2 optimisation. This shouldn't be a problem for most people however, because if you're doing anything remotely creative, you want this enabled anyways.

- Improved Code inside bettercamera.inc.c. A lot of the code inside this file has been cleaned up, which cuts down on bloat, and improves readability, as well as performance. Most of it's stuff you'd generally leave alone anyway, but it's good to not write YandereDev tier code.