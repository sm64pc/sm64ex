#ifndef VIBRATION_MODULE
#define VIBRATION_MODULE

#ifndef AVOID_UTYPES
typedef float  f32;
typedef double f64;
typedef char bool;
#endif

enum ControllerType {
    PRO_CONTROLLER,
    DUAL_JOYCON,
    HANDHELD,
};

struct NXController {
    char* name;
    char* icon;
    enum ControllerType type;
};

// Controller
void controller_nx_init();
void get_controller_nx(struct NXController* controller);

// Rumble
void controller_nx_rumble_play(f32 strength, f32 length);
void controller_nx_rumble_stop(void);
void controller_nx_rumble_loop(void);


#endif