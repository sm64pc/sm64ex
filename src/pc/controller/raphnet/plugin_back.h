#ifndef _plugin_back_h__
#define _plugin_back_h__

/* Those match mupen64plus levels */
#define PB_MSG_ERROR	1
#define PB_MSG_WARNING	2
#define PB_MSG_INFO		3
#define PB_MSG_STATUS	4
#define PB_MSG_VERBOSE	5

typedef void (*pb_debugFunc)(int level, const char *message, ...);

int pb_init(pb_debugFunc debugFn);
int pb_shutdown(void);
int pb_scanControllers(void);
int pb_readController(int Control, unsigned char *Command);
int pb_controllerCommand(int Control, unsigned char *Command);
int pb_romOpen(void);
int pb_romClosed(void);

#endif // _plugin_back_h__

