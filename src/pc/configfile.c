// configfile.c - handles loading and saving the configuration options
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "configfile.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

enum ConfigOptionType {
    CONFIG_TYPE_BOOL,
    CONFIG_TYPE_UINT,
    CONFIG_TYPE_FLOAT,
};

struct ConfigOption {
    const char *name;
    enum ConfigOptionType type;
    union {
        bool *boolValue;
        unsigned int *uintValue;
        float *floatValue;
    };
};

/*
 *Config options and default values
 */
bool configFullscreen            = false;
// Keyboard mappings (scancode values)
unsigned int configKeyA          = 0x26;
unsigned int configKeyB          = 0x33;
unsigned int configKeyStart      = 0x39;
unsigned int configKeyL          = 0x34;
unsigned int configKeyR          = 0x36;
unsigned int configKeyZ          = 0x25;
unsigned int configKeyCUp        = 0x148;
unsigned int configKeyCDown      = 0x150;
unsigned int configKeyCLeft      = 0x14B;
unsigned int configKeyCRight     = 0x14D;
unsigned int configKeyStickUp    = 0x11;
unsigned int configKeyStickDown  = 0x1F;
unsigned int configKeyStickLeft  = 0x1E;
unsigned int configKeyStickRight = 0x20;
// Gamepad mappings (SDL_GameControllerButton values)
unsigned int configJoyA          = 0;
unsigned int configJoyB          = 2;
unsigned int configJoyStart      = 6;
unsigned int configJoyL          = 7;
unsigned int configJoyR          = 10;
unsigned int configJoyZ          = 9;
#ifdef BETTERCAMERA
// Mouse button mappings (0 for none, 1 for left, 2 for middle, 3 for right)
unsigned int configMouseA        = 3;
unsigned int configMouseB        = 1;
unsigned int configMouseL        = 4;
unsigned int configMouseR        = 5;
unsigned int configMouseZ        = 2;
// BetterCamera settings
unsigned int configCameraXSens   = 50;
unsigned int configCameraYSens   = 50;
unsigned int configCameraAggr    = 0;
unsigned int configCameraPan     = 0;
bool         configCameraInvertX = false;
bool         configCameraInvertY = false;
bool         configEnableCamera  = false;
bool         configCameraMouse   = false;
#endif

static const struct ConfigOption options[] = {
    {.name = "fullscreen",           .type = CONFIG_TYPE_BOOL, .boolValue = &configFullscreen},
    {.name = "key_a",                .type = CONFIG_TYPE_UINT, .uintValue = &configKeyA},
    {.name = "key_b",                .type = CONFIG_TYPE_UINT, .uintValue = &configKeyB},
    {.name = "key_start",            .type = CONFIG_TYPE_UINT, .uintValue = &configKeyStart},
    {.name = "key_l",                .type = CONFIG_TYPE_UINT, .uintValue = &configKeyL},
    {.name = "key_r",                .type = CONFIG_TYPE_UINT, .uintValue = &configKeyR},
    {.name = "key_z",                .type = CONFIG_TYPE_UINT, .uintValue = &configKeyZ},
    {.name = "key_cup",              .type = CONFIG_TYPE_UINT, .uintValue = &configKeyCUp},
    {.name = "key_cdown",            .type = CONFIG_TYPE_UINT, .uintValue = &configKeyCDown},
    {.name = "key_cleft",            .type = CONFIG_TYPE_UINT, .uintValue = &configKeyCLeft},
    {.name = "key_cright",           .type = CONFIG_TYPE_UINT, .uintValue = &configKeyCRight},
    {.name = "key_stickup",          .type = CONFIG_TYPE_UINT, .uintValue = &configKeyStickUp},
    {.name = "key_stickdown",        .type = CONFIG_TYPE_UINT, .uintValue = &configKeyStickDown},
    {.name = "key_stickleft",        .type = CONFIG_TYPE_UINT, .uintValue = &configKeyStickLeft},
    {.name = "key_stickright",       .type = CONFIG_TYPE_UINT, .uintValue = &configKeyStickRight},
    {.name = "joy_a",                .type = CONFIG_TYPE_UINT, .uintValue = &configJoyA},
    {.name = "joy_b",                .type = CONFIG_TYPE_UINT, .uintValue = &configJoyB},
    {.name = "joy_start",            .type = CONFIG_TYPE_UINT, .uintValue = &configJoyStart},
    {.name = "joy_l",                .type = CONFIG_TYPE_UINT, .uintValue = &configJoyL},
    {.name = "joy_r",                .type = CONFIG_TYPE_UINT, .uintValue = &configJoyR},
    {.name = "joy_z",                .type = CONFIG_TYPE_UINT, .uintValue = &configJoyZ},
#ifdef BETTERCAMERA
    {.name = "mouse_a",              .type = CONFIG_TYPE_UINT, .uintValue = &configMouseA},
    {.name = "mouse_b",              .type = CONFIG_TYPE_UINT, .uintValue = &configMouseB},
    {.name = "mouse_l",              .type = CONFIG_TYPE_UINT, .uintValue = &configMouseL},
    {.name = "mouse_r",              .type = CONFIG_TYPE_UINT, .uintValue = &configMouseR},
    {.name = "mouse_z",              .type = CONFIG_TYPE_UINT, .uintValue = &configMouseZ},
    {.name = "bettercam_enable",     .type = CONFIG_TYPE_BOOL, .boolValue = &configEnableCamera},
    {.name = "bettercam_mouse_look", .type = CONFIG_TYPE_BOOL, .boolValue = &configCameraMouse},
    {.name = "bettercam_invertx",    .type = CONFIG_TYPE_BOOL, .boolValue = &configCameraInvertX},
    {.name = "bettercam_inverty",    .type = CONFIG_TYPE_BOOL, .boolValue = &configCameraInvertY},
    {.name = "bettercam_xsens",      .type = CONFIG_TYPE_UINT, .uintValue = &configCameraXSens},
    {.name = "bettercam_ysens",      .type = CONFIG_TYPE_UINT, .uintValue = &configCameraYSens},
    {.name = "bettercam_aggression", .type = CONFIG_TYPE_UINT, .uintValue = &configCameraAggr},
    {.name = "bettercam_pan_level",  .type = CONFIG_TYPE_UINT, .uintValue = &configCameraPan},
#endif
};

// Reads an entire line from a file (excluding the newline character) and returns an allocated string
// Returns NULL if no lines could be read from the file
static char *read_file_line(FILE *file) {
    char *buffer;
    size_t bufferSize = 8;
    size_t offset = 0; // offset in buffer to write

    buffer = malloc(bufferSize);
    while (1) {
        // Read a line from the file
        if (fgets(buffer + offset, bufferSize - offset, file) == NULL) {
            free(buffer);
            return NULL; // Nothing could be read.
        }
        offset = strlen(buffer);
        assert(offset > 0);

        // If a newline was found, remove the trailing newline and exit
        if (buffer[offset - 1] == '\n') {
            buffer[offset - 1] = '\0';
            break;
        }

        if (feof(file)) // EOF was reached
            break;

        // If no newline or EOF was reached, then the whole line wasn't read.
        bufferSize *= 2; // Increase buffer size
        buffer = realloc(buffer, bufferSize);
        assert(buffer != NULL);
    }

    return buffer;
}

// Returns the position of the first non-whitespace character
static char *skip_whitespace(char *str) {
    while (isspace(*str))
        str++;
    return str;
}

// NULL-terminates the current whitespace-delimited word, and returns a pointer to the next word
static char *word_split(char *str) {
    // Precondition: str must not point to whitespace
    assert(!isspace(*str));

    // Find either the next whitespace char or end of string
    while (!isspace(*str) && *str != '\0')
        str++;
    if (*str == '\0') // End of string
        return str;

    // Terminate current word
    *(str++) = '\0';

    // Skip whitespace to next word
    return skip_whitespace(str);
}

// Splits a string into words, and stores the words into the 'tokens' array
// 'maxTokens' is the length of the 'tokens' array
// Returns the number of tokens parsed
static unsigned int tokenize_string(char *str, int maxTokens, char **tokens) {
    int count = 0;

    str = skip_whitespace(str);
    while (str[0] != '\0' && count < maxTokens) {
        tokens[count] = str;
        str = word_split(str);
        count++;
    }
    return count;
}

// Loads the config file specified by 'filename'
void configfile_load(const char *filename) {
    FILE *file;
    char *line;

    printf("Loading configuration from '%s'\n", filename);

    file = fopen(filename, "r");
    if (file == NULL) {
        // Create a new config file and save defaults
        printf("Config file '%s' not found. Creating it.\n", filename);
        configfile_save(filename);
        return;
    }

    // Go through each line in the file
    while ((line = read_file_line(file)) != NULL) {
        char *p = line;
        char *tokens[2];
        int numTokens;

        while (isspace(*p))
            p++;
        numTokens = tokenize_string(p, 2, tokens);
        if (numTokens != 0) {
            if (numTokens == 2) {
                const struct ConfigOption *option = NULL;

                for (unsigned int i = 0; i < ARRAY_LEN(options); i++) {
                    if (strcmp(tokens[0], options[i].name) == 0) {
                        option = &options[i];
                        break;
                    }
                }
                if (option == NULL)
                    printf("unknown option '%s'\n", tokens[0]);
                else {
                    switch (option->type) {
                        case CONFIG_TYPE_BOOL:
                            if (strcmp(tokens[1], "true") == 0)
                                *option->boolValue = true;
                            else if (strcmp(tokens[1], "false") == 0)
                                *option->boolValue = false;
                            break;
                        case CONFIG_TYPE_UINT:
                            sscanf(tokens[1], "%u", option->uintValue);
                            break;
                        case CONFIG_TYPE_FLOAT:
                            sscanf(tokens[1], "%f", option->floatValue);
                            break;
                        default:
                            assert(0); // bad type
                    }
                    printf("option: '%s', value: '%s'\n", tokens[0], tokens[1]);
                }
            } else
                puts("error: expected value");
        }
        free(line);
    }

    fclose(file);
}

// Writes the config file to 'filename'
void configfile_save(const char *filename) {
    FILE *file;

    printf("Saving configuration to '%s'\n", filename);

    file = fopen(filename, "w");
    if (file == NULL) {
        // error
        return;
    }

    for (unsigned int i = 0; i < ARRAY_LEN(options); i++) {
        const struct ConfigOption *option = &options[i];

        switch (option->type) {
            case CONFIG_TYPE_BOOL:
                fprintf(file, "%s %s\n", option->name, *option->boolValue ? "true" : "false");
                break;
            case CONFIG_TYPE_UINT:
                fprintf(file, "%s %u\n", option->name, *option->uintValue);
                break;
            case CONFIG_TYPE_FLOAT:
                fprintf(file, "%s %f\n", option->name, *option->floatValue);
                break;
            default:
                assert(0); // unknown type
        }
    }

    fclose(file);
}
