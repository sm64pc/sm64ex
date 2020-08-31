#ifndef CONTROLLER_KEYBOARD_H
#define CONTROLLER_KEYBOARD_H

#include <stdbool.h>
#include "controller_api.h"

# define VK_BASE_KEYBOARD 0x0000

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TEXT_INPUT 256
extern char textInput[];

enum TextInputMode {
    TIM_IP,
    TIM_MULTI_LINE,
    TIM_SINGLE_LINE,
};

bool keyboard_on_key_down(int scancode);
bool keyboard_on_key_up(int scancode);
void keyboard_on_all_keys_up(void);
void keyboard_on_text_input(char* text);
char* keyboard_start_text_input(enum TextInputMode, void (*onEscape)(void), void (*onEnter)(void));
void keyboard_stop_text_input(void);

#ifdef __cplusplus
}
#endif

extern struct ControllerAPI controller_keyboard;

#endif
