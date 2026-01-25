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
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <limits>
#include <sys/ioctl.h>

namespace fs = std::filesystem;

// Colors class - All cyan theme
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
    static const std::string BLUE;
    static const std::string RED;
    static const std::string L_CYAN;
    static const std::string L_GREEN;
    static const std::string L_BLUE;
    static const std::string L_WHITE;
    static const std::string D_CYAN;
};

const std::string Colors::RESET = "\033[0m";
const std::string Colors::BOLD = "\033[1m";
const std::string Colors::CYAN = "\033[36m";
const std::string Colors::GREEN = "\033[32m";
const std::string Colors::WHITE = "\033[37m";
const std::string Colors::MAGENTA = "\033[35m";
const std::string Colors::YELLOW = "\033[33m";
const std::string Colors::L_YELLOW = "\033[93m";
const std::string Colors::BLUE = "\033[34m";
const std::string Colors::RED = "\033[31m";
const std::string Colors::L_CYAN = "\033[96m";
const std::string Colors::L_GREEN = "\033[92m";
const std::string Colors::L_BLUE = "\033[94m";
const std::string Colors::L_WHITE = "\033[97m";
const std::string Colors::D_CYAN = "\033[36m";

// ASCII art for claudefetch - All cyan
class AsciiArt {
public:
    static const std::vector<std::string> ARCH;
    static const std::vector<std::string> DEFAULT;
};

const std::vector<std::string> AsciiArt::ARCH = {
    Colors::CYAN + "          .-------------------------." + Colors::RESET,
    Colors::CYAN + "          .+=========================." + Colors::RESET,
    Colors::CYAN + "         :++===++==================-       :++-" + Colors::RESET,
    Colors::CYAN + "        :*++====+++++=============-        .==:" + Colors::RESET,
    Colors::CYAN + "       -*+++=====+***++==========:" + Colors::RESET,
    Colors::CYAN + "      =*++++========------------:" + Colors::RESET,
    Colors::CYAN + "     =*+++++=====-                     ..." + Colors::RESET,
    Colors::CYAN + "   .+*+++++=-===:                    .=+++=:" + Colors::RESET,
    Colors::CYAN + "  :++++=====-==:                     -*****+" + Colors::RESET,
    Colors::CYAN + " :++========-=.                      .=+**+." + Colors::RESET,
    Colors::CYAN + " .+==========-.                          ." + Colors::RESET,
    Colors::CYAN + " :+++++++====-                                .--==-." + Colors::RESET,
    Colors::CYAN + "  :++==========.                             :+++++++:" + Colors::RESET,
    Colors::CYAN + "   .-===========.                            =*****+*+" + Colors::RESET,
    Colors::CYAN + "    .-===========:                           .+*****+:" + Colors::RESET,
    Colors::CYAN + "      -=======++++:::::::::::::::::::::::::-:  .---:" + Colors::RESET,
    Colors::CYAN + "       :======++++====+++******************=." + Colors::RESET,
    Colors::CYAN + "        :=====+++==========++++++++++++++*-" + Colors::RESET,
    Colors::CYAN + "         .====++==============++++++++++*-" + Colors::RESET,
    Colors::CYAN + "          .===+==================+++++++:" + Colors::RESET,
    Colors::CYAN + "           .-=======================+++:" + Colors::RESET,
    Colors::CYAN + "             .........................." + Colors::RESET
};

const std::vector<std::string> AsciiArt::DEFAULT = {
    Colors::CYAN + "                    #####" + Colors::RESET,
    Colors::CYAN + "                   #######" + Colors::RESET,
    Colors::CYAN + "                   ##" + Colors::L_CYAN + "O" + Colors::CYAN + "#" + Colors::L_CYAN + "O" + Colors::CYAN + "##" + Colors::RESET,
    Colors::CYAN + "                   #" + Colors::L_CYAN + "#####" + Colors::CYAN + "#" + Colors::RESET,
    Colors::CYAN + "                 ##" + Colors::L_CYAN + "##" + Colors::CYAN + "###" + Colors::L_CYAN + "##" + Colors::CYAN + "##" + Colors::RESET,
    Colors::CYAN + "                #" + Colors::L_CYAN + "##########" + Colors::CYAN + "##" + Colors::RESET,
    Colors::CYAN + "               #" + Colors::L_CYAN + "#####" + Colors::CYAN + "####" + Colors::L_CYAN + "#####" + Colors::CYAN + "#" + Colors::RESET,
    Colors::CYAN + "               ##" + Colors::L_CYAN + "#####" + Colors::CYAN + "####" + Colors::L_CYAN + "#####" + Colors::CYAN + "##" + Colors::RESET,
    Colors::CYAN + "                ####################" + Colors::RESET,
    Colors::CYAN + "                ##" + Colors::L_CYAN + "############" + Colors::CYAN + "##" + Colors::RESET,
    Colors::CYAN + "                 ##################" + Colors::RESET,
    Colors::CYAN + "                  ################" + Colors::RESET,
    Colors::CYAN + "                   ##############" + Colors::RESET,
    Colors::CYAN + "                    ############" + Colors::RESET,
    Colors::CYAN + "                     ##########" + Colors::RESET,
    Colors::CYAN + "                      ########" + Colors::RESET,
    Colors::CYAN + "                       ######" + Colors::RESET,
    Colors::CYAN + "                        ####" + Colors::RESET,
    Colors::CYAN + "                         ##" + Colors::RESET
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
                if (line.find("PRETTY_NAME=") == 0) {
                    std::string name = line.substr(12);
                    // Remove quotes
                    if (name.front() == '"' && name.back() == '"') {
                        name = name.substr(1, name.length() - 2);
                    }
                    return name + " " + get_arch();
                } else if (line.find("NAME=") == 0) {
                    std::string name = line.substr(5);
                    if (name.front() == '"' && name.back() == '"') {
                        name = name.substr(1, name.length() - 2);
                    }
                    return name + " " + get_arch();
                }
            }
        }
        return "unknown";
    }

    static std::string get_arch() {
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            return buffer.machine;
        }
        return "unknown";
    }

    static std::string get_hostname() {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            return std::string(hostname);
        }
        return "unknown";
    }

    static std::string get_shell() {
        char* shell_env = getenv("SHELL");
        if (shell_env != nullptr) {
            std::string shell_path = shell_env;
            size_t last_slash = shell_path.find_last_of("/");
            if (last_slash != std::string::npos) {
                std::string shell_name = shell_path.substr(last_slash + 1);
                std::string version = run_cmd(shell_name + " --version | head -1");
                return shell_name + " " + version.substr(0, version.find('\n'));
            }
        }
        return "unknown";
    }

    static std::string get_de() {
        char* xdg_desktop = getenv("XDG_CURRENT_DESKTOP");
        if (xdg_desktop != nullptr) {
            return xdg_desktop;
        }
        return "unknown";
    }

    static std::string get_wm() {
        char* xdg_desktop = getenv("XDG_CURRENT_DESKTOP");
        if (xdg_desktop != nullptr) {
            std::string desktop = xdg_desktop;
            if (desktop.find("KDE") != std::string::npos || desktop.find("Plasma") != std::string::npos) {
                return "KWin";
            } else if (desktop.find("GNOME") != std::string::npos) {
                return "Mutter";
            } else if (desktop.find("XFCE") != std::string::npos) {
                return "XFWM4";
            }
        }
        return "unknown";
    }

    static std::string get_theme() {
        std::string theme = run_cmd("kreadconfig5 --file kdeglobals --group KDE --key widgetStyle 2>/dev/null || echo Breeze");
        if (theme == "unknown" || theme.empty()) {
            return "Breeze";
        }
        return theme;
    }

    static std::string get_icons() {
        std::string icons = run_cmd("kreadconfig5 --file kdeglobals --group Icons --key Theme 2>/dev/null || echo breeze-dark");
        return icons;
    }

    static std::string get_font() {
        std::string font = run_cmd("kreadconfig5 --file kdeglobals --group General --key font 2>/dev/null || echo 'Noto Sans 10'");
        return font;
    }

    static std::string get_cursor() {
        std::string cursor = run_cmd("kreadconfig5 --file kcminputrc --group Mouse --key cursorTheme 2>/dev/null || echo breeze_cursors");
        std::string size = run_cmd("kreadconfig5 --file kcminputrc --group Mouse --key cursorSize 2>/dev/null || echo 24");
        return cursor + " (" + size + "px)";
    }

    static std::string get_terminal() {
        char* term = getenv("TERM_PROGRAM");
        if (term != nullptr) {
            return term;
        }
        term = getenv("TERM");
        if (term != nullptr) {
            return term;
        }
        return "unknown";
    }

    static std::string get_cpu() {
        try {
            std::ifstream file("/proc/cpuinfo");
            if (file.is_open()) {
                std::string line;
                std::string model;
                int cores = 0;
                float max_freq = 0.0;

                while (std::getline(file, line)) {
                    if (line.find("model name") == 0) {
                        size_t pos = line.find(": ");
                        if (pos != std::string::npos) {
                            model = line.substr(pos + 2);
                        }
                    } else if (line.find("cpu cores") == 0) {
                        size_t pos = line.find(": ");
                        if (pos != std::string::npos) {
                            cores = std::stoi(line.substr(pos + 2));
                        }
                    } else if (line.find("cpu MHz") == 0) {
                        size_t pos = line.find(": ");
                        if (pos != std::string::npos) {
                            float freq = std::stof(line.substr(pos + 2)) / 1000.0;
                            if (freq > max_freq) max_freq = freq;
                        }
                    }
                }

                if (!model.empty()) {
                    std::stringstream ss;
                    ss << model << " (" << cores << ") @ " << std::fixed << std::setprecision(2) << max_freq << " GHz";
                    return ss.str();
                }
            }
        } catch (...) {}
        return "unknown";
    }

    static std::string get_gpu() {
        try {
            std::string lspci = run_cmd("lspci | grep -E 'VGA|3D|Display' | head -1");
            if (!lspci.empty() && lspci != "unknown") {
                size_t colon = lspci.find(": ");
                if (colon != std::string::npos) {
                    std::string gpu_info = lspci.substr(colon + 2);
                    // Trim and clean the GPU string
                    if (gpu_info.length() > 60) {
                        gpu_info = gpu_info.substr(0, 57) + "...";
                    }
                    return gpu_info;
                }
            }
        } catch (...) {}
        return "unknown";
    }

    static std::string get_memory() {
        try {
            std::string meminfo = run_cmd("free -m");
            std::istringstream iss(meminfo);
            std::string line;
            std::getline(iss, line); // Skip header

            if (std::getline(iss, line)) {
                std::istringstream line_stream(line);
                std::vector<std::string> tokens;
                std::string token;

                while (line_stream >> token) {
                    tokens.push_back(token);
                }

                if (tokens.size() >= 6) {
                    float total = std::stof(tokens[1]) / 1024.0;
                    float used = (std::stof(tokens[1]) - std::stof(tokens[3]) - std::stof(tokens[5])) / 1024.0;
                    int percent = static_cast<int>((used / total) * 100);

                    std::stringstream ss;
                    ss << std::fixed << std::setprecision(2) << used << " GiB / " << total << " GiB (" << percent << "%)";
                    return ss.str();
                }
            }
        } catch (...) {}
        return "unknown";
    }

    static std::string get_swap() {
        try {
            std::string meminfo = run_cmd("free -h");
            std::istringstream iss(meminfo);
            std::string line;
            std::getline(iss, line); // Skip first header
            std::getline(iss, line); // Skip memory line
            std::getline(iss, line); // This should be swap line

            std::istringstream line_stream(line);
            std::vector<std::string> tokens;
            std::string token;

            while (line_stream >> token) {
                tokens.push_back(token);
            }

            if (tokens.size() >= 6) {
                return tokens[2] + " / " + tokens[1] + " (" + tokens[4] + ")";
            }
        } catch (...) {}
        return "unknown";
    }

    static std::string get_disk_usage() {
        try {
            // Get detailed disk information including filesystem type
            std::string df = run_cmd("df -h / | tail -1");
            std::istringstream iss(df);
            std::vector<std::string> tokens;
            std::string token;

            while (iss >> token) {
                tokens.push_back(token);
            }

            if (tokens.size() >= 6) {
                // Get filesystem type
                std::string filesystem = run_cmd("findmnt -n -o FSTYPE /");
                if (filesystem.empty() || filesystem == "unknown") {
                    filesystem = run_cmd("lsblk -f / | tail -1 | awk '{print $2}'");
                }

                // Parse sizes for better formatting
                std::string used = tokens[2];
                std::string total = tokens[1];
                std::string percent = tokens[4];

                // Remove 'G' from sizes for GiB formatting
                if (used.find('G') != std::string::npos) {
                    used = used.substr(0, used.find('G'));
                }
                if (total.find('G') != std::string::npos) {
                    total = total.substr(0, total.find('G'));
                }

                // Calculate in GiB with decimals
                try {
                    float used_gb = std::stof(used);
                    float total_gb = std::stof(total);
                    int percent_int = std::stoi(percent.substr(0, percent.find('%')));

                    std::stringstream ss;
                    ss << std::fixed << std::setprecision(2)
                    << used_gb << " GiB / " << total_gb << " GiB (" << percent_int << "%) - "
                    << filesystem;
                    return ss.str();
                } catch (...) {
                    // Fallback if conversion fails
                    return tokens[2] + " / " + tokens[1] + " (" + tokens[4] + ") - " + filesystem;
                }
            }
        } catch (...) {}
        return "unknown";
    }

    static std::string get_uptime() {
        try {
            std::string uptime = run_cmd("uptime -p");
            if (!uptime.empty() && uptime != "unknown") {
                // Remove "up " prefix
                if (uptime.find("up ") == 0) {
                    uptime = uptime.substr(3);
                }
                return uptime;
            }
        } catch (...) {}
        return "unknown";
    }

    static std::string get_packages() {
        try {
            std::string packages = run_cmd("pacman -Qq | wc -l");
            if (!packages.empty() && packages != "unknown") {
                return packages + " (pacman)";
            }
        } catch (...) {}
        return "unknown";
    }

    static std::string get_kernel() {
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            return std::string(buffer.release);
        }
        return "unknown";
    }

    static std::string get_display_info() {
        try {
            std::string xrandr = run_cmd("xrandr --current 2>/dev/null | grep ' connected' | head -1");
            if (!xrandr.empty() && xrandr != "unknown") {
                // Extract resolution
                size_t res_start = xrandr.find(" ");
                size_t res_end = xrandr.find("+");
                if (res_start != std::string::npos && res_end != std::string::npos) {
                    std::string resolution = xrandr.substr(res_start + 1, res_end - res_start - 1);
                    return resolution + " @ 60 Hz";
                }
            }
        } catch (...) {}
        return "unknown";
    }

    static std::string get_local_ip() {
        try {
            struct ifaddrs *ifaddr, *ifa;
            char host[NI_MAXHOST];
            char netmask[NI_MAXHOST];

            if (getifaddrs(&ifaddr) == -1) {
                return "unknown";
            }

            // First try to get IPv4 address on active interfaces
            for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
                if (ifa->ifa_addr == nullptr) continue;

                // Check for IPv4 address on non-loopback interfaces
                if (ifa->ifa_addr->sa_family == AF_INET) {
                    std::string ifname = ifa->ifa_name;
                    // Skip loopback and virtual interfaces
                    if (ifname.find("lo") == 0 ||
                        ifname.find("docker") != std::string::npos ||
                        ifname.find("virbr") != std::string::npos ||
                        ifname.find("veth") != std::string::npos) {
                        continue;
                        }

                        int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                            host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);

                        // Get netmask
                        if (ifa->ifa_netmask != nullptr) {
                            getnameinfo(ifa->ifa_netmask, sizeof(struct sockaddr_in),
                                        netmask, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);

                            // Calculate prefix length
                            struct sockaddr_in *netmask_addr = (struct sockaddr_in *)ifa->ifa_netmask;
                            unsigned long netmask_val = ntohl(netmask_addr->sin_addr.s_addr);
                            int prefix_length = 0;
                            while (netmask_val) {
                                prefix_length += netmask_val & 1;
                                netmask_val >>= 1;
                            }

                            if (s == 0) {
                                freeifaddrs(ifaddr);
                                std::stringstream ss;
                                ss << host << "/" << prefix_length << " (" << ifname << ")";
                                return ss.str();
                            }
                        }
                }
            }

            freeifaddrs(ifaddr);

            // Fallback: Try ip command for interface name
            std::string ip_cmd = run_cmd("ip -4 addr show | grep -v 127.0.0.1 | grep inet | head -1");
            if (!ip_cmd.empty() && ip_cmd != "unknown") {
                // Parse output like: inet 192.168.1.10/24 brd 192.168.1.255 scope global wlan0
                std::istringstream iss(ip_cmd);
                std::string token;
                std::vector<std::string> tokens;
                while (iss >> token) {
                    tokens.push_back(token);
                }

                if (tokens.size() >= 7) {
                    std::string ip_with_prefix = tokens[1]; // e.g., 192.168.1.10/24
                    std::string interface = tokens[6]; // interface name

                    // Extract just the prefix length
                    size_t slash_pos = ip_with_prefix.find('/');
                    if (slash_pos != std::string::npos) {
                        std::string ip = ip_with_prefix.substr(0, slash_pos);
                        std::string prefix = ip_with_prefix.substr(slash_pos + 1);
                        return ip + "/" + prefix + " (" + interface + ")";
                    }
                }
            }

        } catch (...) {}
        return "unknown";
    }

    static std::string get_locale() {
        char* locale = getenv("LANG");
        if (locale != nullptr) {
            return locale;
        }
        return "unknown";
    }
};

// Get terminal width
int get_terminal_width() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
    // Fallback
    char* columns = getenv("COLUMNS");
    if (columns) {
        return atoi(columns);
    }
    return 120; // Default to 120 columns for better display
}

// UI class for claudefetch-style display - FIXED VERSION
class ClaudeFetchUI {
public:
    static void print_label_value(const std::string& label, const std::string& value, int line_num = 0) {
        // Determine if we should print ASCII art on this line
        if (line_num < AsciiArt::ARCH.size()) {
            // Print ASCII art line
            std::cout << AsciiArt::ARCH[line_num];

            // Calculate position for system info (ASCII art is 50 chars wide)
            int info_start = 55; // ASCII art width (50) + 5 spaces

            // Move cursor to info position on same line
            std::cout << "\033[" << (line_num + 1) << ";" << info_start << "H";

            // Print label and value
            std::cout << Colors::BOLD << Colors::L_CYAN << std::left << std::setw(12) << label
            << Colors::RESET << Colors::L_CYAN << value << Colors::RESET;
        } else {
            // Lines after ASCII art - just print info aligned
            int info_start = 55;
            std::cout << std::string(info_start, ' ')
            << Colors::BOLD << Colors::L_CYAN << std::left << std::setw(12) << label
            << Colors::RESET << Colors::L_CYAN << value << Colors::RESET;
        }

        std::cout << std::endl;
    }

    static void display_header() {
        std::string username = getenv("USER") ? getenv("USER") : "user";
        std::string hostname = SystemInfo::get_hostname();

        std::cout << Colors::BOLD << Colors::L_CYAN << username
                  << Colors::L_CYAN << "@"
                  << Colors::BOLD << Colors::L_CYAN << hostname
                  << Colors::RESET << std::endl;
        std::cout << Colors::BOLD << Colors::L_CYAN << std::string(40, '-') << Colors::RESET << std::endl;
    }

    static void display_all_info() {
        // Clear screen first
        std::cout << "\033[2J\033[1;1H"; // Clear screen and move to top-left

        // Get all system info
        std::string os = SystemInfo::get_os();
        std::string host = SystemInfo::run_cmd("cat /sys/class/dmi/id/product_name 2>/dev/null || echo 'unknown'");
        std::string kernel = SystemInfo::get_kernel();
        std::string uptime = SystemInfo::get_uptime();
        std::string packages = SystemInfo::get_packages();
        std::string shell = SystemInfo::get_shell();
        std::string display = SystemInfo::get_display_info();
        std::string de = SystemInfo::get_de();
        std::string wm = SystemInfo::get_wm();
        std::string theme = SystemInfo::get_theme();
        std::string icons = SystemInfo::get_icons();
        std::string font = SystemInfo::get_font();
        std::string cursor = SystemInfo::get_cursor();
        std::string terminal = SystemInfo::get_terminal();
        std::string cpu = SystemInfo::get_cpu();
        std::string gpu = SystemInfo::get_gpu();
        std::string memory = SystemInfo::get_memory();
        std::string swap = SystemInfo::get_swap();
        std::string disk = SystemInfo::get_disk_usage();
        std::string local_ip = SystemInfo::get_local_ip();
        std::string locale = SystemInfo::get_locale();

        int line_num = 0;

        // Line 1: OS
        print_label_value("OS:", os, line_num++);

        // Line 2: Host
        print_label_value("Host:", host, line_num++);

        // Line 3: Kernel
        print_label_value("Kernel:", kernel, line_num++);

        // Line 4: Uptime
        print_label_value("Uptime:", uptime, line_num++);

        // Line 5: Packages
        print_label_value("Packages:", packages, line_num++);

        // Line 6: Shell
        print_label_value("Shell:", shell, line_num++);

        // Line 7: Display
        print_label_value("Display:", display, line_num++);

        // Line 8: DE
        print_label_value("DE:", de, line_num++);

        // Line 9: WM
        print_label_value("WM:", wm, line_num++);

        // Line 10: WM Theme
        print_label_value("WM Theme:", theme, line_num++);

        // Line 11: Theme
        std::string theme_full = theme + " [Qt], Breeze-Dark [GTK]";
        if (theme_full.length() > 60) {
            theme_full = theme_full.substr(0, 57) + "...";
        }
        print_label_value("Theme:", theme_full, line_num++);

        // Line 12: Icons
        std::string icons_full = icons + " [Qt], " + icons + " [GTK]";
        if (icons_full.length() > 60) {
            icons_full = icons_full.substr(0, 57) + "...";
        }
        print_label_value("Icons:", icons_full, line_num++);

        // Line 13: Font
        std::string font_full = font + " [Qt], " + font + " [GTK]";
        if (font_full.length() > 60) {
            font_full = font_full.substr(0, 57) + "...";
        }
        print_label_value("Font:", font_full, line_num++);

        // Line 14: Cursor
        print_label_value("Cursor:", cursor, line_num++);

        // Line 15: Terminal
        print_label_value("Terminal:", terminal, line_num++);

        // Line 16: CPU
        print_label_value("CPU:", cpu, line_num++);

        // Line 17: GPU
        print_label_value("GPU:", gpu, line_num++);

        // Line 18: Memory
        print_label_value("Memory:", memory, line_num++);

        // Line 19: Swap
        print_label_value("Swap:", swap, line_num++);

        // Line 20: Disk (/)
        print_label_value("Disk (/):", disk, line_num++);

        // Line 21: Local IP
        print_label_value("Local IP:", local_ip, line_num++);

        // Line 22: Locale
        print_label_value("Locale:", locale, line_num++);

        // If we have more lines than ASCII art, add blank lines to match
        while (line_num < AsciiArt::ARCH.size()) {
            std::cout << AsciiArt::ARCH[line_num] << std::endl;
            line_num++;
        }

        // Menu separator
        std::cout << Colors::BOLD << Colors::L_CYAN << std::string(80, '=') << Colors::RESET << std::endl;
    }

    static void display_menu() {
        std::cout << Colors::BOLD << Colors::L_CYAN << "claudefetch System Maintenance Menu:" << Colors::RESET << std::endl;
        std::cout << Colors::L_CYAN << "  (A)udit Full System" << Colors::WHITE << "   "
        << Colors::L_CYAN << "(C)heck Updates" << Colors::WHITE << "   "
        << Colors::L_CYAN << "(F)ree Memory" << std::endl;
        std::cout << Colors::L_CYAN << "  (R)emove Cache" << Colors::WHITE << "        "
        << Colors::L_CYAN << "(U)pdate System" << Colors::WHITE << "       "
        << Colors::L_CYAN << "(Q)uit to Shell" << std::endl;
        std::cout << Colors::BOLD << Colors::L_CYAN << "\nSelect option: " << Colors::RESET;
        std::cout.flush();
    }
};

// Actions class - FIXED SINGLE ENTER
class Actions {
public:
    static void freemem() {
        system("clear");
        std::cout << Colors::BOLD << Colors::L_CYAN << "*** Freeing Memory ***" << Colors::RESET << std::endl;
        system("sudo free -h");
        system("sudo sysctl -w vm.drop_caches=3");
        system("sync");
        system("echo 3 | sudo tee /proc/sys/vm/drop_caches");
        std::cout << Colors::RESET << Colors::BOLD << "\r";
        system("sudo free -h");
        press_enter();
    }

    static void check_updates() {
        system("clear");
        std::cout << Colors::BOLD << Colors::L_CYAN << "*** Checking for Updates ***" << Colors::RESET << std::endl;
        int result = system("checkupdates");
        if (result != 0 && result != 2) {
            std::cout << Colors::BOLD << Colors::L_CYAN << "*** No Updates or Error: " << result << Colors::RESET << std::endl;
        }
        press_enter();
    }

    static void install_updates() {
        system("clear");
        std::cout << Colors::BOLD << Colors::L_CYAN << "*** Installing Updates ***" << Colors::RESET << std::endl;
        int result = system("sudo pacman -Syyu");
        if (result != 0) {
            std::cout << Colors::BOLD << Colors::L_CYAN << "*** Update process failed with exit code " << result << Colors::RESET << std::endl;
        }
        press_enter();
    }

    static void remove_all_cache() {
        system("clear");
        std::cout << Colors::BOLD << Colors::L_CYAN << "*** Removing Cache ***" << Colors::RESET << std::endl;

        std::string home_dir = getenv("HOME");
        system(("rm -Rvf " + home_dir + "/.cache/").c_str());
        fs::create_directories(home_dir + "/.cache");
        system(("rm -Rvf " + home_dir + "/.local/share/recently-used.xbel*").c_str());

        press_enter();
    }

    static void full_sys_audit() {
        system("clear");
        std::cout << Colors::BOLD << Colors::L_CYAN << "*** Full System Audit Running [Please Wait...] ***" << Colors::RESET << std::endl;

        std::string home_dir = getenv("HOME");
        fs::create_directories(home_dir + "/Audit");

        system(("sudo lynis audit system --forensics --pentest --verbose --no-log > " +
        home_dir + "/Audit/FullSysAudit.txt 2>&1").c_str());

        std::cout << Colors::BOLD << Colors::L_CYAN << "\n### Full System Audit Report Available in ~/Audit directory ###" << Colors::RESET << std::endl;
        press_enter();
    }

    static void press_enter() {
        std::cout << Colors::BOLD << Colors::L_CYAN << "\n*** PRESS Enter to continue ***" << Colors::RESET << std::endl;
        // SINGLE Enter press - no double input
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
};

// Simple command execution function
void execute_shell_command() {
    std::cout << Colors::BOLD << Colors::L_CYAN << "\nEnter shell command (or 'exit' to return): " << Colors::RESET;
    std::cout.flush();

    std::string command;
    std::getline(std::cin, command);

    if (command == "exit" || command == "q" || command == "quit") {
        std::cout << Colors::L_CYAN << "Returning to claudefetch..." << Colors::RESET << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return;
    }

    if (!command.empty()) {
        std::cout << Colors::CYAN << "Executing: " << command << Colors::RESET << std::endl;
        system(command.c_str());
        std::cout << Colors::L_CYAN << "\nCommand completed. Press Enter to continue..." << Colors::RESET << std::endl;
        // SINGLE Enter press
        std::string dummy;
        std::getline(std::cin, dummy);
    }
}

void selection() {
    char choice;
    std::cin >> choice;
    // SINGLE clear - not double
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch(tolower(choice)) {
        case 'r':
            Actions::remove_all_cache();
            break;
        case 'f':
            Actions::freemem();
            break;
        case 'c':
            Actions::check_updates();
            break;
        case 'u':
            Actions::install_updates();
            break;
        case 'a':
            Actions::full_sys_audit();
            break;
        case 'q':
            std::cout << Colors::BOLD << Colors::L_CYAN << "\n\nShell command mode. Type 'exit' to return." << Colors::RESET << std::endl;
            std::cout << Colors::L_CYAN << "System info remains visible above.\n" << Colors::RESET << std::endl;

            execute_shell_command();
            break;
        default:
            std::cout << Colors::L_CYAN << "Invalid option!" << Colors::RESET << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    while (true) {
        ClaudeFetchUI::display_all_info();
        ClaudeFetchUI::display_menu();
        selection();
    }

    return 0;
}
