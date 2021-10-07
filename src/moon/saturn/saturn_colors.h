#ifndef SaturnColors
#define SaturnColors

#include <string>
#include <vector>

namespace MoonInternal {
    extern std::vector<std::string> cc_array;
    void load_cc_directory(void);
    void load_cc_file(std::string cc_path);
    void save_cc_file(std::string name);
}

#endif