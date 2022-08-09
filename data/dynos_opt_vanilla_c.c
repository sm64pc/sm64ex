#include "dynos.c.h"

// Not my problem
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsizeof-pointer-div"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma GCC diagnostic ignored "-Wpointer-sign"
#pragma GCC diagnostic ignored "-Wsign-compare"
#define optmenu_toggle optmenu_toggle_unused
#define optmenu_draw optmenu_draw_unused
#define optmenu_draw_prompt optmenu_draw_prompt_unused
#define optmenu_check_buttons optmenu_check_buttons_unused
#define optmenu_open optmenu_open_unused
#define DYNOS_INL
#include "game/options_menu.c"
#undef DYNOS_INL
#undef optmenu_toggle
#undef optmenu_draw
#undef optmenu_draw_prompt
#undef optmenu_check_buttons
#undef optmenu_open
#pragma GCC diagnostic pop
// Now, that's my problem

extern void dynos_opt_end_submenu();
extern void dynos_opt_convert_submenu(const u8 *label, const u8 *title);
extern void dynos_opt_convert_toggle(const u8 *label, bool *bval);
extern void dynos_opt_convert_scroll(const u8 *label, s32 min, s32 max, s32 step, u32 *uval);
extern void dynos_opt_convert_choice(const u8 *label, const u8 **choices, s32 numChoices, u32 *uval);
extern void dynos_opt_convert_button(const u8 *label, void *action);
extern void dynos_opt_convert_bind(const u8 *label, u32 *uval);

static void dynos_opt_convert_menu(struct SubMenu *submenu) {
    for (s32 i = 0; i != submenu->numOpts; ++i) {
        struct Option *opt = &submenu->opts[i];
        switch (opt->type) {
            case OPT_TOGGLE:
                dynos_opt_convert_toggle(opt->label, opt->bval);
                break;

            case OPT_CHOICE:
                dynos_opt_convert_choice(opt->label, opt->choices, opt->numChoices, opt->uval);
                break;

            case OPT_SCROLL:
                dynos_opt_convert_scroll(opt->label, opt->scrMin, opt->scrMax, opt->scrStep, opt->uval);
                break;

            case OPT_SUBMENU:
                dynos_opt_convert_submenu(opt->label, opt->nextMenu->label);
                dynos_opt_convert_menu(opt->nextMenu);
                dynos_opt_end_submenu();
                break;

            case OPT_BIND:
                dynos_opt_convert_bind(opt->label, opt->uval);
                break;

            case OPT_BUTTON:
                dynos_opt_convert_button(opt->label, opt->actionFn);
                break;

            default:
                break;
        }
    }
}

void dynos_opt_convert_vanilla_main_menu() {
    dynos_opt_convert_menu(&menuMain);
}
