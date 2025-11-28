#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>
#include <array>
#include <memory>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

namespace fs = std::filesystem;

// Colors class
class Colors {
public:
    static const std::string RESET;
    static const std::string BOLD;
    static const std::string CYAN;
    static const std::string GREEN;
    static const std::string WHITE;
    static const std::string MAGENTA;
    static const std::string YELLOW;
    static const std::string L_YELLOW;
};

const std::string Colors::RESET = "\033[0m";
const std::string Colors::BOLD = "\033[1m";
const std::string Colors::CYAN = "\033[36m";
const std::string Colors::GREEN = "\033[32m";
const std::string Colors::WHITE = "\033[37m";
const std::string Colors::MAGENTA = "\033[35m";
const std::string Colors::YELLOW = "\033[33m";
const std::string Colors::L_YELLOW = "\033[93m";

// Graphics characters
class Gr {
public:
    static const std::string L_TOP;
    static const std::string R_TOP;
    static const std::string L_BOTTOM;
    static const std::string R_BOTTOM;
    static const std::string H_LINE;
    static const std::string V_LINE;
};

const std::string Gr::L_TOP = "┌";
const std::string Gr::R_TOP = "┐";
const std::string Gr::L_BOTTOM = "└";
const std::string Gr::R_BOTTOM = "┘";
const std::string Gr::H_LINE = "─";
const std::string Gr::V_LINE = "│";

// Constants
const std::string N_LINE = "\n";
const int LINE_WIDTH = 70;
const int BOX_WIDTH = 32;
const int FIRST_HBOX_LINE = 28;
const int FIRST_V_POS = 70;
const int TEXT_POS = FIRST_V_POS + 3;
const int NBRLINES = 8;
const int PROMPTLINE = FIRST_HBOX_LINE + 28;

// Messages
const std::vector<std::string> Msg = {
    "*** PRESS Enter to continue ***",
    "*** Checkupdates ***",
    "*** Install Updates ***",
    "*** Last Installed/Updated Packages ***",
    "*** Remove Cache ***",
    "*** Full System Audit Running [Please Wait...] ***",
    "### Full System Audit Report Available in ~/Audit directory ###"
};

const std::vector<std::string> ErrMsg = {
    "No Error",
    "*** No Updates or Error: ",
    "*** Update process failed with exit code "
};

const std::vector<std::string> MenuOptions = {
    "(A)uditFullSystem",
    "(C)heckUpdates",
    "(D)ir",
    "(F)reemem",
    "(I)nstallHistory",
    "(R)emoveAllCache",
    "(U)pdate",
    "(Q)uit"
};

// SystemInfo class
class SystemInfo {
public:
    static std::string run_cmd(const std::string& cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) {
            return "unknown";
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        // Remove trailing newline
        if (!result.empty() && result[result.length()-1] == '\n') {
            result.erase(result.length()-1);
        }
        return result;
    }

    static std::string get_os() {
        std::ifstream file("/etc/os-release");
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line.find("NAME=") == 0) {
                    std::string name = line.substr(5);
                    // Remove quotes
                    if (name.front() == '"' && name.back() == '"') {
                        name = name.substr(1, name.length() - 2);
                    }
                    return name;
                }
            }
        }
        return "unknown";
    }

    static std::string get_shell() {
        try {
            std::string version = run_cmd("bash --version");
            std::istringstream iss(version);
            std::string line;
            if (std::getline(iss, line)) {
                std::istringstream line_stream(line);
                std::vector<std::string> tokens;
                std::string token;
                while (line_stream >> token) {
                    tokens.push_back(token);
                }
                if (tokens.size() > 3) {
                    std::string version_num = tokens[3];
                    size_t paren_pos = version_num.find('(');
                    if (paren_pos != std::string::npos) {
                        version_num = version_num.substr(0, paren_pos);
                    }
                    return "bash " + version_num;
                }
            }
        } catch (...) {}
        return "unknown";
    }

    static std::string get_de() {
        std::string de = run_cmd("plasmashell --version 2> /dev/null");
        if (!de.empty() && de != "unknown") {
            std::istringstream iss(de);
            std::string version, number;
            if (iss >> version >> number) {
                return "KDE " + version + " " + number;
            }
        }
        return "unknown";
    }

    static std::vector<std::string> get_kde_theme() {
        std::vector<std::string> results(4, "unknown");
        if (system("which kreadconfig5 > /dev/null 2>&1") == 0) {
            results[0] = run_cmd("kreadconfig5 --file kdeglobals --group Icons --key Theme 2>/dev/null || echo unknown");
            results[1] = run_cmd("kreadconfig5 --file kcminputrc --group Mouse --key cursorTheme 2>/dev/null || echo unknown");
            results[2] = run_cmd("kreadconfig5 --file kdeglobals --group General --key ColorScheme 2>/dev/null || echo unknown");
            results[3] = run_cmd("kreadconfig5 --file kdeglobals --group KDE --key widgetStyle 2>/dev/null || echo unknown");
        }
        return results;
    }

    static std::string get_fonts() {
        try {
            std::string font = run_cmd("fc-match -v");
            size_t pos = font.find("fullname:");
            if (pos != std::string::npos) {
                size_t start = font.find("\"", pos);
                size_t end = font.find("\"", start + 1);
                if (start != std::string::npos && end != std::string::npos) {
                    std::string font_name = font.substr(start + 1, end - start - 1);
                    size_t paren_pos = font_name.find('(');
                    if (paren_pos != std::string::npos) {
                        font_name = font_name.substr(0, paren_pos);
                    }
                    // Trim whitespace
                    font_name.erase(0, font_name.find_first_not_of(" \t"));
                    font_name.erase(font_name.find_last_not_of(" \t") + 1);
                    return font_name;
                }
            }
        } catch (...) {}
        return "unknown";
    }

    static std::vector<std::string> get_gpu() {
        std::vector<std::string> gpus(2, "");
        try {
            std::string lspci = run_cmd("lspci");
            std::istringstream iss(lspci);
            std::string line;
            while (std::getline(iss, line)) {
                std::string lower_line = line;
                std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(), ::tolower);
                if (lower_line.find("vga") != std::string::npos || 
                    lower_line.find("3d") != std::string::npos || 
                    lower_line.find("display") != std::string::npos) {
                    size_t pos = line.find(": ");
                    if (pos != std::string::npos) {
                        gpus[0] = line.substr(pos + 2);
                    }
                    break;
                }
            }

            std::string glxinfo = run_cmd("glxinfo");
            std::istringstream glx_stream(glxinfo);
            while (std::getline(glx_stream, line)) {
                if (line.find("OpenGL renderer") != std::string::npos) {
                    size_t pos = line.find(": ");
                    if (pos != std::string::npos) {
                        gpus[1] = line.substr(pos + 2);
                    }
                    break;
                }
            }
        } catch (...) {}
        return gpus;
    }

    static std::vector<std::string> get_mem() {
        try {
            std::string free_h = run_cmd("free -h");
            std::string free_m = run_cmd("free -m");
            
            std::istringstream h_stream(free_h);
            std::string line;
            std::vector<std::string> h_tokens;
            if (std::getline(h_stream, line) && std::getline(h_stream, line)) {
                std::istringstream line_stream(line);
                std::string token;
                while (line_stream >> token) {
                    h_tokens.push_back(token);
                }
            }

            std::istringstream m_stream(free_m);
            std::vector<std::string> m_tokens;
            if (std::getline(m_stream, line) && std::getline(m_stream, line)) {
                std::istringstream line_stream(line);
                std::string token;
                while (line_stream >> token) {
                    m_tokens.push_back(token);
                }
            }

            if (h_tokens.size() > 1 && m_tokens.size() > 5) {
                std::string total = h_tokens[1];
                int used = std::stoi(m_tokens[1]) - std::stoi(m_tokens[3]) - std::stoi(m_tokens[5]);
                return {std::to_string(used) + "M", total};
            }
        } catch (...) {}
        return {"unknown", "unknown"};
    }

    static std::string get_root_disk() {
        try {
            std::string df = run_cmd("df -h /");
            std::istringstream iss(df);
            std::string line;
            std::vector<std::string> tokens;
            if (std::getline(iss, line) && std::getline(iss, line)) {
                std::istringstream line_stream(line);
                std::string token;
                while (line_stream >> token) {
                    tokens.push_back(token);
                }
                if (tokens.size() > 4) {
                    return tokens[2] + "/" + tokens[1] + " (" + tokens[4] + ")";
                }
            }
        } catch (...) {}
        return "unknown";
    }

    static std::string get_packages() {
        try {
            std::string packages = run_cmd("pacman -Qq");
            int count = 0;
            std::istringstream iss(packages);
            std::string line;
            while (std::getline(iss, line)) {
                count++;
            }
            return std::to_string(count) + " (pacman)";
        } catch (...) {
            return "unknown";
        }
    }

    static std::string get_cpu() {
        try {
            std::ifstream file("/proc/cpuinfo");
            if (file.is_open()) {
                std::string line;
                while (std::getline(file, line)) {
                    if (line.find("model name") == 0) {
                        size_t pos = line.find(": ");
                        if (pos != std::string::npos) {
                            return line.substr(pos + 2);
                        }
                    }
                }
            }
        } catch (...) {}
        return "unknown";
    }
};

// UI class
class UI {
public:
    static void fmt_out(const std::string& key, const std::string& val) {
        std::cout << Colors::BOLD << Colors::CYAN << std::left << std::setw(20) << key 
                  << Colors::RESET << Colors::GREEN << ": " << Colors::WHITE << val << std::endl;
    }

    static void sep_line() {
        std::cout << Colors::MAGENTA << std::string(LINE_WIDTH, Gr::H_LINE[0]) << Colors::RESET << std::endl;
    }

    static void draw_box() {
        // Position cursor and draw top line
        std::cout << "\033[" << FIRST_HBOX_LINE << ";" << FIRST_V_POS << "H" 
                  << Colors::BOLD << Colors::GREEN << Gr::L_TOP 
                  << std::string(BOX_WIDTH, Gr::H_LINE[0]) << Gr::R_TOP;
        
        // Draw vertical lines
        for (int i = 0; i < NBRLINES; i++) {
            std::cout << "\033[" << FIRST_HBOX_LINE + 1 + i << ";" << FIRST_V_POS << "H" << Gr::V_LINE;
            std::cout << "\033[" << FIRST_HBOX_LINE + 1 + i << ";" << FIRST_V_POS + BOX_WIDTH + 1 << "H" << Gr::V_LINE;
        }
        
        // Draw bottom line
        std::cout << "\033[" << FIRST_HBOX_LINE + NBRLINES + 1 << ";" << FIRST_V_POS << "H" 
                  << Gr::L_BOTTOM << std::string(BOX_WIDTH, Gr::H_LINE[0]) << Gr::R_BOTTOM;
    }

    static void display_info() {
        int textline = FIRST_HBOX_LINE + 1;
        for (const auto& item : MenuOptions) {
            std::cout << "\033[" << textline << ";" << TEXT_POS << "H" 
                      << Colors::BOLD << Colors::YELLOW << item;
            textline++;
        }
    }

    static void press_enter() {
        std::cout << Colors::BOLD << Colors::GREEN << Msg[0] << std::endl;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }

    static void clear_screen() {
        system("clear");
    }
};

// Actions class
class Actions {
public:
    static void freemem() {
        UI::clear_screen();
        
        // Display initial memory usage
        std::cout << Colors::L_YELLOW << Colors::BOLD;
        system("sudo free -h");
        
        // Clear caches - equivalent to vm.drop_caches=3
        system("sudo sysctl -w vm.drop_caches=3");
        system("sync");
        system("echo 3 | sudo tee /proc/sys/vm/drop_caches");
        
        // Display final memory usage
        std::cout << Colors::RESET << Colors::BOLD << "\r";
        system("sudo free -h");
        
        UI::press_enter();
    }

    static void open_ranger() {
        UI::clear_screen();
        system("ranger");
    }

    static void check_updates() {
        UI::clear_screen();
        std::cout << Colors::BOLD << Colors::YELLOW << Msg[1] << Colors::RESET << std::endl;
        int result = system("checkupdates");
        if (result != 0 && result != 2) {
            std::cout << Colors::BOLD << Colors::MAGENTA << ErrMsg[1] << result << Colors::RESET << std::endl;
        }
        UI::press_enter();
    }

    static void install_updates() {
        UI::clear_screen();
        std::cout << Colors::BOLD << Colors::YELLOW << Msg[2] << Colors::RESET << std::endl;
        int result = system("sudo pacman -Syyu");
        if (result != 0) {
            std::cout << Colors::BOLD << Colors::MAGENTA << ErrMsg[2] << result << Colors::RESET << std::endl;
        }
        UI::press_enter();
    }

    static void list_inst_updates() {
        UI::clear_screen();
        std::cout << Colors::BOLD << Colors::YELLOW << Msg[3] << Colors::RESET << std::endl;
        system("expac --timefmt='%Y-%m-%d %T' '%l\t%n' | sort | tail -n 50");
        UI::press_enter();
    }

    static void remove_all_cache() {
        UI::clear_screen();
        std::cout << Colors::BOLD << Colors::YELLOW << Msg[4] << Colors::RESET << std::endl;
        
        std::string home_dir = getenv("HOME");
        
        // Remove cache
        system(("rm -Rvf " + home_dir + "/.cache/").c_str());
        fs::create_directories(home_dir + "/.cache");
        
        // Remove recently used
        system(("rm -Rvf " + home_dir + "/.local/share/recently-used.xbel*").c_str());
        
        UI::press_enter();
    }

    static void full_sys_audit() {
        UI::clear_screen();
        std::cout << Colors::BOLD << Colors::YELLOW << Msg[5] << Colors::RESET << std::endl;
        
        std::string home_dir = getenv("HOME");
        fs::create_directories(home_dir + "/Audit");
        
        system(("sudo lynis audit system --forensics --pentest --verbose --no-log > " + 
                home_dir + "/Audit/FullSysAudit.txt").c_str());
        
        std::cout << N_LINE << Colors::BOLD << Colors::WHITE << Msg[6] << Colors::RESET << N_LINE << std::endl;
        UI::press_enter();
    }
};

void my_fetch() {
    // Get system information
    UI::sep_line();
    UI::fmt_out("OS", SystemInfo::get_os() + " " + SystemInfo::run_cmd("uname -m"));
    UI::fmt_out("Kernel", SystemInfo::run_cmd("uname -sr"));
    UI::fmt_out("Shell", SystemInfo::get_shell());
    UI::fmt_out("Root Disk", SystemInfo::get_root_disk());
    UI::fmt_out("Packages", SystemInfo::get_packages());
    UI::sep_line();
    
    // Get DE information
    char* session_type = getenv("XDG_SESSION_TYPE");
    std::string session = session_type ? session_type : "unknown";
    UI::fmt_out("DE", SystemInfo::get_de() + " - " + session);
    
    auto kde_themes = SystemInfo::get_kde_theme();
    UI::fmt_out("Qt Style", kde_themes[3]);
    UI::fmt_out("Icons", kde_themes[0]);
    UI::fmt_out("Color Scheme", kde_themes[2]);
    UI::fmt_out("Font", SystemInfo::get_fonts());
    UI::fmt_out("Plasma Cursor", kde_themes[1]);
    UI::sep_line();
    
    // Get hardware information
    UI::fmt_out("CPU", SystemInfo::get_cpu() + " (" + std::to_string(std::thread::hardware_concurrency()) + " cores)");
    
    auto gpus = SystemInfo::get_gpu();
    UI::fmt_out("GPU", gpus[0]);
    UI::fmt_out("Second GPU", gpus[1]);
    
    auto memory = SystemInfo::get_mem();
    UI::fmt_out("Mem.(Used/Total)", memory[0] + "/" + memory[1]);
    UI::sep_line();
}

void selection() {
    std::cout << "\033[" << PROMPTLINE << "H" << Colors::RESET;
    
    char choice;
    std::cin >> choice;
    
    switch(choice) {
        case 'r': case 'R':
            Actions::remove_all_cache();
            break;
        case 'f': case 'F':
            Actions::freemem();
            break;
        case 'd': case 'D':
            Actions::open_ranger();
            break;
        case 'c': case 'C':
            Actions::check_updates();
            break;
        case 'u': case 'U':
            Actions::install_updates();
            break;
        case 'i': case 'I':
            Actions::list_inst_updates();
            break;
        case 'a': case 'A':
            Actions::full_sys_audit();
            break;
        default:
            exit(1);
    }
}

int main() {
    while (true) {
        // Display SystemInfo
        UI::clear_screen();
        my_fetch();

        // Display Install Info and Disk Info
        system("paru -P --stats");
        system("dysk -c label+default --filter \"disk <> HDD\" --sort filesystem -u binary");
        system("dysk -c label+default --filter \"disk <> SSD\" --sort filesystem -u binary");

        // Display Box with Menu Options
        UI::draw_box();
        UI::display_info();

        // Select actions
        selection();
    }
    
    return 0;
}
