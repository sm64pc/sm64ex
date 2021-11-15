#ifndef SaturnColors
#define SaturnColors

#include <string>
#include <vector>

namespace MoonInternal {
    extern std::vector<std::string> cc_array;
    void load_cc_directory(void);
    void load_cc_file(std::string cc_path);
    void save_cc_file(std::string name);
    void delete_cc_file(std::string name);
    std::string global_color_to_cc();
    void load_cc_data(std::string content);
}

#endif