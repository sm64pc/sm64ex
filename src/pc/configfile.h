#ifndef CONFIGFILE_H
#define CONFIGFILE_H

extern bool         configFullscreen;
extern unsigned int configKeyA;
extern unsigned int configKeyB;
extern unsigned int configKeyStart;
extern unsigned int configKeyL;
extern unsigned int configKeyR;
extern unsigned int configKeyZ;
extern unsigned int configKeyCUp;
extern unsigned int configKeyCDown;
extern unsigned int configKeyCLeft;
extern unsigned int configKeyCRight;
extern unsigned int configKeyStickUp;
extern unsigned int configKeyStickDown;
extern unsigned int configKeyStickLeft;
extern unsigned int configKeyStickRight;
extern unsigned int configJoyA;
extern unsigned int configJoyB;
extern unsigned int configJoyStart;
extern unsigned int configJoyL;
extern unsigned int configJoyR;
extern unsigned int configJoyZ;
extern unsigned int configMouseA;
extern unsigned int configMouseB;
extern unsigned int configMouseStart;
extern unsigned int configMouseL;
extern unsigned int configMouseR;
extern unsigned int configMouseZ;

void configfile_load(const char *filename);
void configfile_save(const char *filename);

#endif
