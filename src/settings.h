#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <cctype>
#include <cstdio>
#include <fstream>
#include <string>

inline float MOUSE_SENS_X = 2.0f;
inline float MOUSE_SENS_Y = 2.0f;
inline float MOUSE_FIRST_PERSON_SENS_X = 2.0f;
inline float MOUSE_FIRST_PERSON_SENS_Y = 4.0f;
inline float MOUSE_REDUCTION = 0.001f;

inline bool SHOW_FPS = true;
inline bool VSYNC = true;
inline bool show_collision_point = false;
inline bool draw_collision_sphere = false;

inline bool settings_loaded = false;

inline std::string
trim_copy(const std::string &text)
{
        size_t begin = 0;
        size_t end = text.size();

        while (begin < end && std::isspace(static_cast<unsigned char>(text[begin]))) {
                ++begin;
        }

        while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
                --end;
        }

        return text.substr(begin, end - begin);
}

inline bool
parse_bool_value(const std::string &value)
{
        return value == "on" || value == "true" || value == "1" || value == "yes";
}

inline void
apply_setting(const std::string &section, const std::string &key, const std::string &value)
{
        if (section == "mouse") {
                if (key == "sens_x") MOUSE_SENS_X = std::stof(value);
                if (key == "sens_y") MOUSE_SENS_Y = std::stof(value);
                if (key == "first_person_sens_x") MOUSE_FIRST_PERSON_SENS_X = std::stof(value);
                if (key == "first_person_sens_y") MOUSE_FIRST_PERSON_SENS_Y = std::stof(value);
                if (key == "reduction") MOUSE_REDUCTION = std::stof(value);
                return;
        }

        if (section == "options") {
                if (key == "show_fps") SHOW_FPS = parse_bool_value(value);
                if (key == "vsync") VSYNC = parse_bool_value(value);
                return;
        }

        if (section == "dev") {
                if (key == "show_collision_point") show_collision_point = parse_bool_value(value);
                if (key == "draw_collision_sphere") draw_collision_sphere = parse_bool_value(value);
        }
}

inline void
load_settings(const char *path = "settings.hcf")
{
        if (settings_loaded) {
                return;
        }

        settings_loaded = true;

        std::ifstream input(path);
        if (!input.is_open()) {
            std::fprintf(stderr, "Warning: could not open %s, using default settings\n", path);
            return;
        }

        std::string section;
        std::string line;

        while (std::getline(input, line)) {
                std::string trimmed = trim_copy(line);

                if (trimmed.empty() || trimmed[0] == '#') {
                        continue;
                }

                if (trimmed.back() == ':') {
                        section = trim_copy(trimmed.substr(0, trimmed.size() - 1));
                        continue;
                }

                size_t split = trimmed.find_first_of(" \t");
                if (split == std::string::npos) {
                        continue;
                }

                std::string key = trimmed.substr(0, split);
                std::string value = trim_copy(trimmed.substr(split));

                if (value.empty()) {
                        continue;
                }

                apply_setting(section, key, value);
        }
}

#endif // SETTINGS_H_
