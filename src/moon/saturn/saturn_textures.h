#ifndef SaturnTextures
#define SaturnTextures

#include <string>
#include <vector>

extern std::string custom_eye_name;
extern std::string custom_emblem_name;
extern std::string custom_stache_name;
extern std::string custom_button_name;
extern std::string custom_sideburn_name;
extern std::string custom_sky_name;

extern std::vector<std::string> eye_array;
extern std::vector<std::string> emblem_array;
extern std::vector<std::string> stache_array;
extern std::vector<std::string> button_array;
extern std::vector<std::string> sideburn_array;

void saturn_eye_swap(void);
void saturn_load_eye_array(void);
void saturn_emblem_swap(void);
void saturn_load_emblem_array(void);
void saturn_stache_swap(void);
void saturn_load_stache_array(void);
void saturn_button_swap(void);
void saturn_load_button_array(void);
void saturn_sideburn_swap(void);
void saturn_load_sideburn_array(void);
void saturn_toggle_m_cap(void);
void saturn_toggle_m_buttons(void);
void saturn_sky_swap(void);
//void saturn_toggle_night_skybox(void);

#endif