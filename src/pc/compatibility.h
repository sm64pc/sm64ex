#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

// compatiblity header if behavior between Operat

#ifdef _WIN32
#define _mkdir(path, mode) mkdir(path)
#else
#define _mkdir(path, mode) mkdir(path, mode)
#endif

#endif // COMPATIBILITY