#ifndef SaturnTextures
#define SaturnTextures

#include <string>
#include <vector>

extern std::string custom_eye_name;
extern std::string custom_sky_name;

extern std::vector<std::string> eye_array;

void saturn_eye_swap(void);
void saturn_load_eye_array(void);
void saturn_toggle_m_cap(void);
void saturn_toggle_m_buttons(void);
void saturn_sky_swap(void);
//void saturn_toggle_night_skybox(void);

#endif