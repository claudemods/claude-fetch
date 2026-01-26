#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cmath>
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
#include <regex>

namespace fs = std::filesystem;

// Colors class - Keep cyan theme for labels and text
class Colors {
public:
    static const std::string RESET;
    static const std::string BOLD;
    static const std::string CYAN;
    static const std::string L_CYAN;
    static const std::string RED;  // Only for ASCII art
    static const std::string L_RED; // Only for ASCII art
};

const std::string Colors::RESET = "\033[0m";
const std::string Colors::BOLD = "\033[1m";
const std::string Colors::CYAN = "\033[96m";
const std::string Colors::L_CYAN = "\033[96m";
const std::string Colors::RED = "\033[91m";    // For ASCII art
const std::string Colors::L_RED = "\033[91m";  // For ASCII art

// ASCII art for claudefetch - RED theme only
class AsciiArt {
public:
    static const std::vector<std::string> ARCH;
    static const std::vector<std::string> DEFAULT;
};

const std::vector<std::string> AsciiArt::ARCH = {
    Colors::L_RED + "                    " + Colors::RESET,
    Colors::L_RED + "    cccccccccccccccc" + Colors::RESET,
    Colors::L_RED + "  cc:::::::::::::::c" + Colors::RESET,
    Colors::L_RED + " c:::::::::::::::::c" + Colors::RESET,
    Colors::L_RED + "c:::::::cccccc:::::c" + Colors::RESET,
    Colors::L_RED + "c::::::c     ccccccc" + Colors::RESET,
    Colors::L_RED + "c:::::c             " + Colors::RESET,
    Colors::L_RED + "c:::::c             " + Colors::RESET,
    Colors::L_RED + "c::::::c     ccccccc" + Colors::RESET,
    Colors::L_RED + "c:::::::cccccc:::::c" + Colors::RESET,
    Colors::L_RED + " c:::::::::::::::::c" + Colors::RESET,
    Colors::L_RED + "  cc:::::::::::::::c" + Colors::RESET,
    Colors::L_RED + "    cccccccccccccccc" + Colors::RESET,
    Colors::L_RED + "                    " + Colors::RESET
};

const std::vector<std::string> AsciiArt::DEFAULT = {
    Colors::L_RED + "                    " + Colors::RESET,
    Colors::L_RED + "    cccccccccccccccc" + Colors::RESET,
    Colors::L_RED + "  cc:::::::::::::::c" + Colors::RESET,
    Colors::L_RED + " c:::::::::::::::::c" + Colors::RESET,
    Colors::L_RED + "c:::::::cccccc:::::c" + Colors::RESET,
    Colors::L_RED + "c::::::c     ccccccc" + Colors::RESET,
    Colors::L_RED + "c:::::c             " + Colors::RESET,
    Colors::L_RED + "c:::::c             " + Colors::RESET,
    Colors::L_RED + "c::::::c     ccccccc" + Colors::RESET,
    Colors::L_RED + "c:::::::cccccc:::::c" + Colors::RESET,
    Colors::L_RED + " c:::::::::::::::::c" + Colors::RESET,
    Colors::L_RED + "  cc:::::::::::::::c" + Colors::RESET,
    Colors::L_RED + "    cccccccccccccccc" + Colors::RESET,
    Colors::L_RED + "                    " + Colors::RESET
};

// SystemInfo class - Arch Linux + KDE ONLY, NO DEFAULTS
class SystemInfo {
public:
    static std::string run_cmd(const std::string& cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) {
            return "";
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        if (!result.empty() && result[result.length()-1] == '\n') {
            result.erase(result.length()-1);
        }
        return result;
    }

    static std::string get_os() {
        std::string arch = get_arch();
        if (arch.empty()) return "";

        std::ifstream os_release("/etc/os-release");
        if (!os_release.is_open()) return "";

        std::string line;
        while (std::getline(os_release, line)) {
            if (line.find("PRETTY_NAME=") == 0) {
                std::string name = line.substr(12);
                if (name.front() == '"' && name.back() == '"') {
                    name = name.substr(1, name.length() - 2);
                }
                return name + " " + arch;
            }
        }
        return "";
    }

    static std::string get_arch() {
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            return buffer.machine;
        }
        return "";
    }

    static std::string get_hostname() {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            return std::string(hostname);
        }
        return "";
    }

    static std::string get_host() {
        std::string product_name = run_cmd("cat /sys/class/dmi/id/product_name 2>/dev/null");
        std::string product_version = run_cmd("cat /sys/class/dmi/id/product_version 2>/dev/null");

        std::string result;
        if (!product_name.empty()) {
            result = product_name;
            if (!product_version.empty()) {
                result += " (" + product_version + ")";
            }
        }
        return result;
    }

    static std::string get_kernel() {
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            return std::string(buffer.release);
        }
        return "";
    }

    static std::string get_uptime() {
        std::string uptime = run_cmd("uptime -p 2>/dev/null");
        if (!uptime.empty()) {
            if (uptime.find("up ") == 0) {
                return uptime.substr(3);
            }
            return uptime;
        }
        return "";
    }

    static std::string get_packages() {
        std::string packages = run_cmd("pacman -Qq 2>/dev/null | wc -l");
        if (!packages.empty()) {
            return packages + " (pacman)";
        }
        return "";
    }

    static std::string get_shell() {
        // Method 1: Check SHELL environment variable
        char* shell_env = getenv("SHELL");
        if (shell_env != nullptr) {
            std::string shell_path = shell_env;
            size_t last_slash = shell_path.find_last_of("/");
            if (last_slash != std::string::npos) {
                std::string shell_name = shell_path.substr(last_slash + 1);

                // Get version for specific shells
                if (shell_name == "bash") {
                    std::string version = run_cmd("bash --version 2>/dev/null | head -1");
                    if (!version.empty()) {
                        std::regex version_regex(R"(GNU bash, version ([0-9]+\.[0-9]+\.[0-9]+))");
                        std::smatch match;
                        if (std::regex_search(version, match, version_regex)) {
                            return "bash " + match.str(1);
                        }
                        return "bash";
                    }
                } else if (shell_name == "zsh") {
                    std::string version = run_cmd("zsh --version 2>/dev/null");
                    if (!version.empty()) {
                        std::regex version_regex(R"(zsh ([0-9]+\.[0-9]+(?:\.[0-9]+)?))");
                        std::smatch match;
                        if (std::regex_search(version, match, version_regex)) {
                            return "zsh " + match.str(1);
                        }
                        return "zsh";
                    }
                } else if (shell_name == "fish") {
                    std::string version = run_cmd("fish --version 2>/dev/null");
                    if (!version.empty()) {
                        std::regex version_regex(R"(fish, version ([0-9]+\.[0-9]+\.[0-9]+))");
                        std::smatch match;
                        if (std::regex_search(version, match, version_regex)) {
                            return "fish " + match.str(1);
                        }
                        return "fish";
                    }
                } else if (shell_name == "dash") {
                    return "dash";
                } else if (shell_name == "ksh") {
                    std::string version = run_cmd("ksh --version 2>/dev/null | head -1");
                    if (!version.empty()) {
                        std::regex version_regex(R"(version (.+))");
                        std::smatch match;
                        if (std::regex_search(version, match, version_regex)) {
                            return "ksh " + match.str(1);
                        }
                        return "ksh";
                    }
                } else if (shell_name == "tcsh") {
                    std::string version = run_cmd("tcsh --version 2>/dev/null | head -1");
                    if (!version.empty()) {
                        return "tcsh";
                    }
                } else if (shell_name == "csh") {
                    return "csh";
                } else if (shell_name == "ash") {
                    return "ash";
                } else if (shell_name == "mksh") {
                    std::string version = run_cmd("mksh --version 2>/dev/null | head -1");
                    if (!version.empty()) {
                        return "mksh";
                    }
                }
                return shell_name;
            }
        }

        // Method 2: Check current process name
        std::string proc_name = run_cmd("ps -p $$ -o comm= 2>/dev/null");
        if (!proc_name.empty()) {
            if (proc_name == "bash") {
                std::string version = run_cmd("bash --version 2>/dev/null | head -1");
                if (!version.empty()) {
                    std::regex version_regex(R"(GNU bash, version ([0-9]+\.[0-9]+\.[0-9]+))");
                    std::smatch match;
                    if (std::regex_search(version, match, version_regex)) {
                        return "bash " + match.str(1);
                    }
                }
                return "bash";
            } else if (proc_name == "zsh") {
                std::string version = run_cmd("zsh --version 2>/dev/null");
                if (!version.empty()) {
                    std::regex version_regex(R"(zsh ([0-9]+\.[0-9]+(?:\.[0-9]+)?))");
                    std::smatch match;
                    if (std::regex_search(version, match, version_regex)) {
                        return "zsh " + match.str(1);
                    }
                }
                return "zsh";
            } else if (proc_name == "fish") {
                std::string version = run_cmd("fish --version 2>/dev/null");
                if (!version.empty()) {
                    std::regex version_regex(R"(fish, version ([0-9]+\.[0-9]+\.[0-9]+))");
                    std::smatch match;
                    if (std::regex_search(version, match, version_regex)) {
                        return "fish " + match.str(1);
                    }
                }
                return "fish";
            }
            return proc_name;
        }

        // Method 3: Check parent process
        std::string parent_cmd = run_cmd("ps -p $PPID -o comm= 2>/dev/null");
        if (!parent_cmd.empty()) {
            return parent_cmd;
        }

        return "";
    }

    static std::string get_display() {
        // Get display information using xrandr with verbose output
        std::string xrandr_output = run_cmd("xrandr --verbose 2>/dev/null");
        if (xrandr_output.empty()) {
            return "";
        }

        std::istringstream iss(xrandr_output);
        std::string line;
        std::vector<std::map<std::string, std::string>> displays;
        std::map<std::string, std::string> current_display;
        std::string current_name;
        bool in_edid_section = false;
        std::string edid_hex;

        // Parse xrandr output
        while (std::getline(iss, line)) {
            // Check for connected display
            if (line.find(" connected") != std::string::npos) {
                if (!current_name.empty()) {
                    displays.push_back(current_display);
                }

                current_display.clear();
                in_edid_section = false;
                edid_hex.clear();

                std::istringstream line_stream(line);
                line_stream >> current_name;
                current_display["port"] = current_name;

                // Get connection type
                if (line.find("primary") != std::string::npos) {
                    current_display["type"] = "Primary";
                } else {
                    current_display["type"] = "External";
                }

                // Try to get resolution
                std::regex res_regex(R"((\d+)x(\d+))");
                std::smatch res_match;
                if (std::regex_search(line, res_match, res_regex)) {
                    current_display["resolution"] = res_match[0];
                }

                // Try to get refresh rate from the same line
                std::regex hz_regex(R"((\d+\.?\d*)Hz)");
                std::smatch hz_match;
                if (std::regex_search(line, hz_match, hz_regex)) {
                    current_display["refresh_rate"] = hz_match[1];
                }
            }
            // Look for EDID information
            else if (line.find("EDID:") != std::string::npos) {
                in_edid_section = true;
                edid_hex.clear();
            }
            else if (in_edid_section) {
                // Check if we're still in EDID section
                if (line.find("\t\t") == 0 || line.find("    ") == 0) {
                    // Parse EDID hex data
                    std::istringstream hex_line(line);
                    std::string hex_part;
                    while (hex_line >> hex_part) {
                        if (hex_part.length() == 2) {
                            edid_hex += hex_part;
                        }
                    }
                } else {
                    in_edid_section = false;

                    // Process collected EDID data
                    if (edid_hex.length() >= 256) {
                        // Extract monitor name from EDID (bytes 72-107)
                        std::string monitor_name;
                        for (int i = 144; i < 216 && i + 2 <= edid_hex.length(); i += 2) {
                            std::string hex_byte = edid_hex.substr(i, 2);
                            try {
                                int byte_val = std::stoi(hex_byte, nullptr, 16);
                                if (byte_val >= 32 && byte_val <= 126) {
                                    monitor_name += static_cast<char>(byte_val);
                                } else if (byte_val == 10) {
                                    // Newline character
                                    break;
                                }
                            } catch (...) {}
                        }

                        // Clean up monitor name
                        size_t null_pos = monitor_name.find('\0');
                        if (null_pos != std::string::npos) {
                            monitor_name = monitor_name.substr(0, null_pos);
                        }

                        // Remove trailing spaces and control characters
                        while (!monitor_name.empty() &&
                            (monitor_name.back() == ' ' ||
                            monitor_name.back() == '\n' ||
                            monitor_name.back() == '\r')) {
                            monitor_name.pop_back();
                            }

                            // Filter out non-alphanumeric monitor names (like "default")
                            bool has_letters = false;
                        bool has_numbers = false;
                        for (char c : monitor_name) {
                            if (std::isalpha(c)) has_letters = true;
                            if (std::isdigit(c)) has_numbers = true;
                        }

                        if (!monitor_name.empty() && (has_letters || has_numbers)) {
                            current_display["monitor_name"] = monitor_name;
                        }

                        // Try to get physical size from EDID (bytes 21-22 in cm)
                        try {
                            if (edid_hex.length() >= 44) {
                                std::string width_hex = edid_hex.substr(42, 2);
                                std::string height_hex = edid_hex.substr(44, 2);
                                int width_cm = std::stoi(width_hex, nullptr, 16);
                                int height_cm = std::stoi(height_hex, nullptr, 16);

                                if (width_cm > 0 && height_cm > 0) {
                                    // Convert to inches
                                    double width_in = width_cm / 2.54;
                                    double height_in = height_cm / 2.54;
                                    double diagonal_in = sqrt(width_in * width_in + height_in * height_in);

                                    std::stringstream size_ss;
                                    size_ss << std::fixed << std::setprecision(1) << diagonal_in;
                                    current_display["size"] = size_ss.str();
                                }
                            }
                        } catch (...) {}
                    }
                }
            }
            // Look for specific display properties
            else if (!current_name.empty()) {
                // Get refresh rate if not already found
                if (current_display.find("refresh_rate") == current_display.end()) {
                    std::regex hz_regex(R"((\d+\.?\d*)Hz)");
                    std::smatch hz_match;
                    if (std::regex_search(line, hz_match, hz_regex)) {
                        current_display["refresh_rate"] = hz_match[1];
                    }
                }
            }
        }

        // Add the last display
        if (!current_name.empty()) {
            displays.push_back(current_display);
        }

        // Format the display information
        if (displays.empty()) {
            return "";
        }

        std::stringstream result;
        for (size_t i = 0; i < displays.size(); i++) {
            const auto& display = displays[i];

            if (i > 0) {
                result << ", ";
            }

            // Add monitor name if available (e.g., 26LG3000)
            if (display.find("monitor_name") != display.end()) {
                result << display.at("monitor_name");
            } else if (display.find("port") != display.end()) {
                // Fall back to port name if no monitor name
                result << display.at("port");
            }

            // Add resolution
            if (display.find("resolution") != display.end()) {
                result << " " << display.at("resolution");
            }

            // Add size
            if (display.find("size") != display.end()) {
                result << " in " << display.at("size") << "\"";
            }

            // Add refresh rate
            if (display.find("refresh_rate") != display.end()) {
                result << ", " << display.at("refresh_rate") << " Hz";
            }

            // Add display type
            if (display.find("type") != display.end()) {
                result << " [" << display.at("type") << "]";
            }
        }

        return result.str();
    }

    static std::string get_de() {
        char* kde_session = getenv("KDE_SESSION_VERSION");
        char* xdg_desktop = getenv("XDG_CURRENT_DESKTOP");

        if (kde_session || (xdg_desktop && (std::string(xdg_desktop).find("KDE") != std::string::npos ||
            std::string(xdg_desktop).find("Plasma") != std::string::npos))) {
            std::string plasma_version = run_cmd("plasmashell --version 2>/dev/null | head -1");
        if (!plasma_version.empty()) {
            std::regex version_regex("[0-9]+\\.[0-9]+\\.[0-9]+");
            std::smatch match;
            if (std::regex_search(plasma_version, match, version_regex)) {
                return "KDE Plasma " + match.str();
            }
        }
        return "KDE Plasma";
            }
            return "";
    }

    static std::string get_wm() {
        char* wayland_display = getenv("WAYLAND_DISPLAY");
        if (wayland_display != nullptr) {
            return "KWin (Wayland)";
        }
        return "KWin";
    }

    static std::string get_wm_theme() {
        std::string theme = run_cmd("kreadconfig6 --file kdeglobals --group KDE --key widgetStyle 2>/dev/null");
        if (theme.empty()) {
            theme = run_cmd("kreadconfig5 --file kdeglobals --group KDE --key widgetStyle 2>/dev/null");
        }
        return theme;
    }

    static std::string get_theme() {
        std::string qt_theme = run_cmd("kreadconfig6 --file kdeglobals --group KDE --key widgetStyle 2>/dev/null");
        if (qt_theme.empty()) {
            qt_theme = run_cmd("kreadconfig5 --file kdeglobals --group KDE --key widgetStyle 2>/dev/null");
        }

        std::string color_scheme = run_cmd("kreadconfig6 --file kdeglobals --group General --key ColorScheme 2>/dev/null");
        if (color_scheme.empty()) {
            color_scheme = run_cmd("kreadconfig5 --file kdeglobals --group General --key ColorScheme 2>/dev/null");
        }

        std::string gtk2_theme = run_cmd("grep 'gtk-theme-name' ~/.gtkrc-2.0 2>/dev/null | cut -d'=' -f2 | tr -d '\"'");
        std::string gtk3_theme = run_cmd("grep 'gtk-theme-name' ~/.config/gtk-3.0/settings.ini 2>/dev/null | cut -d'=' -f2 | tr -d '\"'");

        std::string result;
        if (!qt_theme.empty()) {
            result = qt_theme;
            if (!color_scheme.empty()) {
                result += " (" + color_scheme + ")";
            }
            result += " [Qt]";
        }

        if (!gtk2_theme.empty()) {
            if (!result.empty()) result += ", ";
            result += gtk2_theme + " [GTK2]";
        }

        if (!gtk3_theme.empty()) {
            if (!result.empty()) result += ", ";
            result += gtk3_theme + " [GTK3]";
        }

        return result;
    }

    static std::string get_icons() {
        std::string qt_icons = run_cmd("kreadconfig6 --file kdeglobals --group Icons --key Theme 2>/dev/null");
        if (qt_icons.empty()) {
            qt_icons = run_cmd("kreadconfig5 --file kdeglobals --group Icons --key Theme 2>/dev/null");
        }

        std::string gtk2_icons = run_cmd("grep 'gtk-icon-theme-name' ~/.gtkrc-2.0 2>/dev/null | cut -d'=' -f2 | tr -d '\"'");
        std::string gtk3_icons = run_cmd("grep 'gtk-icon-theme-name' ~/.config/gtk-3.0/settings.ini 2>/dev/null | cut -d'=' -f2 | tr -d '\"'");

        std::string result;
        if (!qt_icons.empty()) {
            result = qt_icons + " [Qt]";
        }

        if (!gtk2_icons.empty()) {
            if (!result.empty()) result += ", ";
            result += gtk2_icons + " [GTK2]";
        }

        if (!gtk3_icons.empty()) {
            if (!result.empty()) result += ", ";
            result += gtk3_icons + " [GTK3]";
        }

        return result;
    }

    static std::string get_font() {
        std::string qt_font = run_cmd("kreadconfig6 --file kdeglobals --group General --key font 2>/dev/null");
        if (qt_font.empty()) {
            qt_font = run_cmd("kreadconfig5 --file kdeglobals --group General --key font 2>/dev/null");
        }

        std::string gtk2_font = run_cmd("grep 'gtk-font-name' ~/.gtkrc-2.0 2>/dev/null | cut -d'=' -f2 | tr -d '\"'");
        std::string gtk3_font = run_cmd("grep 'gtk-font-name' ~/.config/gtk-3.0/settings.ini 2>/dev/null | cut -d'=' -f2 | tr -d '\"'");

        std::string result;
        if (!qt_font.empty()) {
            result = qt_font + " [Qt]";
        }

        if (!gtk2_font.empty()) {
            if (!result.empty()) result += ", ";
            result += gtk2_font + " [GTK2]";
        }

        if (!gtk3_font.empty()) {
            if (!result.empty()) result += ", ";
            result += gtk3_font + " [GTK3]";
        }

        return result;
    }

    static std::string get_cursor() {
        std::string cursor_theme = run_cmd("kreadconfig6 --file kcminputrc --group Mouse --key cursorTheme 2>/dev/null");
        if (cursor_theme.empty()) {
            cursor_theme = run_cmd("kreadconfig5 --file kcminputrc --group Mouse --key cursorTheme 2>/dev/null");
        }

        std::string cursor_size = run_cmd("kreadconfig6 --file kcminputrc --group Mouse --key cursorSize 2>/dev/null");
        if (cursor_size.empty()) {
            cursor_size = run_cmd("kreadconfig5 --file kcminputrc --group Mouse --key cursorSize 2>/dev/null");
        }

        if (!cursor_theme.empty()) {
            if (!cursor_size.empty()) {
                return cursor_theme + " (" + cursor_size + "px)";
            }
            return cursor_theme;
        }
        return "";
    }

    static std::string get_terminal() {
        // Check multiple methods for terminal detection

        // Method 1: Check TERM_PROGRAM environment variable (used by many terminals)
        char* term_program = getenv("TERM_PROGRAM");
        if (term_program != nullptr) {
            std::string term = term_program;
            if (term == "Konsole") {
                std::string version = run_cmd("konsole --version 2>/dev/null");
                if (!version.empty()) {
                    std::regex version_regex("[0-9]+\\.[0-9]+\\.[0-9]+");
                    std::smatch match;
                    if (std::regex_search(version, match, version_regex)) {
                        return "Konsole " + match.str();
                    }
                }
                return "Konsole";
            } else if (term == "GNOME Terminal") {
                std::string version = run_cmd("gnome-terminal --version 2>/dev/null");
                if (!version.empty()) {
                    std::regex version_regex("[0-9]+\\.[0-9]+\\.[0-9]+");
                    std::smatch match;
                    if (std::regex_search(version, match, version_regex)) {
                        return "GNOME Terminal " + match.str();
                    }
                }
                return "GNOME Terminal";
            } else if (term == "iTerm.app") {
                return "iTerm2";
            }
        }

        // Method 2: Check TERMINAL_EMULATOR (used by some terminals)
        char* term_emulator = getenv("TERMINAL_EMULATOR");
        if (term_emulator != nullptr) {
            std::string term = term_emulator;
            if (term.find("Konsole") != std::string::npos) {
                return "Konsole";
            } else if (term.find("gnome-terminal") != std::string::npos) {
                return "GNOME Terminal";
            }
        }

        // Method 3: Get parent process name
        std::string parent_cmd = run_cmd("ps -p $PPID -o comm= 2>/dev/null");
        if (!parent_cmd.empty()) {
            if (parent_cmd == "konsole") {
                std::string version = run_cmd("konsole --version 2>/dev/null");
                if (!version.empty()) {
                    std::regex version_regex("[0-9]+\\.[0-9]+\\.[0-9]+");
                    std::smatch match;
                    if (std::regex_search(version, match, version_regex)) {
                        return "Konsole " + match.str();
                    }
                }
                return "Konsole";
            } else if (parent_cmd == "gnome-terminal") {
                std::string version = run_cmd("gnome-terminal --version 2>/dev/null");
                if (!version.empty()) {
                    std::regex version_regex("[0-9]+\\.[0-9]+\\.[0-9]+");
                    std::smatch match;
                    if (std::regex_search(version, match, version_regex)) {
                        return "GNOME Terminal " + match.str();
                    }
                }
                return "GNOME Terminal";
            } else if (parent_cmd == "xfce4-terminal") {
                std::string version = run_cmd("xfce4-terminal --version 2>/dev/null");
                if (!version.empty()) {
                    std::regex version_regex("[0-9]+\\.[0-9]+\\.[0-9]+");
                    std::smatch match;
                    if (std::regex_search(version, match, version_regex)) {
                        return "XFCE Terminal " + match.str();
                    }
                }
                return "XFCE Terminal";
            } else if (parent_cmd == "terminator") {
                std::string version = run_cmd("terminator --version 2>/dev/null | head -1");
                if (!version.empty()) {
                    std::regex version_regex("[0-9]+\\.[0-9]+");
                    std::smatch match;
                    if (std::regex_search(version, match, version_regex)) {
                        return "Terminator " + match.str();
                    }
                }
                return "Terminator";
            } else if (parent_cmd == "tilix") {
                std::string version = run_cmd("tilix --version 2>/dev/null");
                if (!version.empty()) {
                    std::regex version_regex("[0-9]+\\.[0-9]+");
                    std::smatch match;
                    if (std::regex_search(version, match, version_regex)) {
                        return "Tilix " + match.str();
                    }
                }
                return "Tilix";
            } else if (parent_cmd == "alacritty") {
                std::string version = run_cmd("alacritty --version 2>/dev/null");
                if (!version.empty()) {
                    std::regex version_regex("[0-9]+\\.[0-9]+\\.[0-9]+");
                    std::smatch match;
                    if (std::regex_search(version, match, version_regex)) {
                        return "Alacritty " + match.str();
                    }
                }
                return "Alacritty";
            } else if (parent_cmd == "kitty") {
                std::string version = run_cmd("kitty --version 2>/dev/null");
                if (!version.empty()) {
                    std::regex version_regex("[0-9]+\\.[0-9]+\\.[0-9]+");
                    std::smatch match;
                    if (std::regex_search(version, match, version_regex)) {
                        return "Kitty " + match.str();
                    }
                }
                return "Kitty";
            } else if (parent_cmd == "xterm") {
                std::string version = run_cmd("xterm -version 2>/dev/null | head -1");
                if (!version.empty()) {
                    std::regex version_regex("[0-9]+\\.[0-9]+");
                    std::smatch match;
                    if (std::regex_search(version, match, version_regex)) {
                        return "XTerm " + match.str();
                    }
                }
                return "XTerm";
            } else if (parent_cmd == "rxvt") {
                return "Rxvt";
            } else if (parent_cmd == "urxvt") {
                return "URxvt";
            } else if (parent_cmd == "mate-terminal") {
                return "MATE Terminal";
            } else if (parent_cmd == "lxterminal") {
                return "LXTerminal";
            } else if (parent_cmd == "terminology") {
                return "Terminology";
            }
        }

        // Method 4: Check for specific terminal environment variables
        if (getenv("KONSOLE_VERSION") != nullptr) {
            return "Konsole";
        } else if (getenv("GNOME_TERMINAL_SCREEN") != nullptr) {
            return "GNOME Terminal";
        } else if (getenv("XTERM_VERSION") != nullptr) {
            return "XTerm";
        } else if (getenv("ALACRITTY_LOG") != nullptr) {
            return "Alacritty";
        } else if (getenv("KITTY_WINDOW_ID") != nullptr) {
            return "Kitty";
        }

        // Method 5: Check $TERM environment variable (fallback)
        char* term_env = getenv("TERM");
        if (term_env != nullptr) {
            std::string term = term_env;
            if (term == "xterm-kitty") {
                return "Kitty";
            } else if (term == "xterm-termite") {
                return "Termite";
            } else if (term.find("rxvt") != std::string::npos) {
                return "Rxvt";
            }
        }

        // Method 6: Try to detect by checking desktop entry
        std::string desktop_term = run_cmd("xdg-mime query default x-scheme-handler/terminal 2>/dev/null");
        if (!desktop_term.empty()) {
            if (desktop_term.find("konsole") != std::string::npos) return "Konsole";
            if (desktop_term.find("gnome-terminal") != std::string::npos) return "GNOME Terminal";
            if (desktop_term.find("xfce4-terminal") != std::string::npos) return "XFCE Terminal";
            if (desktop_term.find("terminator") != std::string::npos) return "Terminator";
            if (desktop_term.find("tilix") != std::string::npos) return "Tilix";
        }

        // Method 7: Fallback to checking current process
        std::string self_cmd = run_cmd("ps -p $$ -o comm= 2>/dev/null");
        if (!self_cmd.empty()) {
            return self_cmd;
        }

        return "";
    }

    static std::string get_cpu() {
        std::ifstream cpuinfo("/proc/cpuinfo");
        if (!cpuinfo.is_open()) return "";

        std::string line;
        std::string model;
        int cores = 0;
        int threads = 0;
        float base_freq = 0.0;
        float current_freq = 0.0;

        while (std::getline(cpuinfo, line)) {
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
            } else if (line.find("siblings") == 0) {
                size_t pos = line.find(": ");
                if (pos != std::string::npos) {
                    threads = std::stoi(line.substr(pos + 2));
                }
            } else if (line.find("cpu MHz") == 0) {
                size_t pos = line.find(": ");
                if (pos != std::string::npos) {
                    float freq = std::stof(line.substr(pos + 2)) / 1000.0;
                    if (current_freq < freq) {
                        current_freq = freq;
                    }
                }
            }
        }

        // Try to get base frequency from model name
        if (!model.empty()) {
            std::regex freq_regex(R"((@\s*(\d+\.?\d*)\s*GHz))");
            std::smatch match;
            if (std::regex_search(model, match, freq_regex)) {
                try {
                    base_freq = std::stof(match[2]);
                } catch (...) {
                    base_freq = 0.0;
                }
            }
        }

        // Get current frequency from /proc/cpuinfo or sysfs
        if (current_freq == 0.0) {
            std::string freq_file = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq";
            std::ifstream freq_stream(freq_file);
            if (freq_stream.is_open()) {
                std::string freq_str;
                if (std::getline(freq_stream, freq_str)) {
                    try {
                        current_freq = std::stof(freq_str) / 1000000.0;
                    } catch (...) {}
                }
            }
        }

        if (!model.empty()) {
            std::stringstream ss;
            ss << model;

            if (cores > 0 && threads > 0) {
                ss << " (" << cores;
                if (threads > cores) {
                    ss << "C/" << threads << "T";
                } else {
                    ss << ")";
                }
            }

            if (current_freq > 0.0) {
                ss << " @ " << std::fixed << std::setprecision(2) << current_freq << " GHz";
            } else if (base_freq > 0.0) {
                ss << " @ " << std::fixed << std::setprecision(2) << base_freq << " GHz";
            }

            return ss.str();
        }
        return "";
    }

    static std::string get_gpu() {
        std::string lspci_output = run_cmd("lspci 2>/dev/null");
        std::istringstream iss(lspci_output);
        std::string line;
        std::vector<std::string> gpus;

        while (std::getline(iss, line)) {
            if (line.find("VGA") != std::string::npos ||
                line.find("3D") != std::string::npos ||
                line.find("Display") != std::string::npos) {

                size_t colon = line.find(": ");
            if (colon != std::string::npos) {
                std::string gpu_info = line.substr(colon + 2);

                // Clean up the GPU info
                // Remove common redundant prefixes
                std::string clean_gpu = gpu_info;

                // Add classification
                std::string type = "[Integrated]";
                if (gpu_info.find("NVIDIA") != std::string::npos) {
                    type = "[Dedicated]";
                } else if (gpu_info.find("AMD") != std::string::npos) {
                    type = "[Dedicated]";
                } else if (gpu_info.find("Intel") != std::string::npos) {
                    if (gpu_info.find("UHD Graphics") != std::string::npos ||
                        gpu_info.find("HD Graphics") != std::string::npos ||
                        gpu_info.find("Iris") != std::string::npos) {
                        type = "[Integrated]";
                        }
                } else if (gpu_info.find("Radeon") != std::string::npos) {
                    type = "[Dedicated]";
                } else if (gpu_info.find("GeForce") != std::string::npos) {
                    type = "[Dedicated]";
                } else if (gpu_info.find("Quadro") != std::string::npos) {
                    type = "[Dedicated]";
                }

                gpus.push_back(clean_gpu + " " + type);
            }
                }
        }

        if (!gpus.empty()) {
            std::stringstream result;
            for (size_t i = 0; i < gpus.size(); i++) {
                if (i > 0) {
                    result << ", ";
                }
                result << gpus[i];
            }
            return result.str();
        }

        // Fallback to lspci grep
        std::string lspci = run_cmd("lspci | grep -E 'VGA|3D|Display' | head -1");
        if (!lspci.empty()) {
            size_t colon = lspci.find(": ");
            if (colon != std::string::npos) {
                std::string gpu_info = lspci.substr(colon + 2);
                std::string type = "[Integrated]";

                if (gpu_info.find("NVIDIA") != std::string::npos ||
                    gpu_info.find("AMD") != std::string::npos ||
                    gpu_info.find("Radeon") != std::string::npos ||
                    gpu_info.find("GeForce") != std::string::npos) {
                    type = "[Dedicated]";
                    }

                    return gpu_info + " " + type;
            }
        }
        return "";
    }

    static std::string get_memory() {
        std::ifstream meminfo("/proc/meminfo");
        if (!meminfo.is_open()) return "";

        long total_kb = 0, free_kb = 0, buffers_kb = 0, cached_kb = 0;
        std::string line;

        while (std::getline(meminfo, line)) {
            if (line.find("MemTotal:") == 0) {
                total_kb = std::stol(line.substr(9));
            } else if (line.find("MemFree:") == 0) {
                free_kb = std::stol(line.substr(8));
            } else if (line.find("Buffers:") == 0) {
                buffers_kb = std::stol(line.substr(8));
            } else if (line.find("Cached:") == 0) {
                cached_kb = std::stol(line.substr(7));
            }
        }

        if (total_kb > 0) {
            long used_kb = total_kb - free_kb - buffers_kb - cached_kb;
            float total_gb = total_kb / 1048576.0;
            float used_gb = used_kb / 1048576.0;
            int percent = (used_kb * 100) / total_kb;

            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << used_gb << " GiB / " << total_gb << " GiB (" << percent << "%)";
            return ss.str();
        }
        return "";
    }

    static std::string get_swap() {
        std::ifstream meminfo("/proc/meminfo");
        if (!meminfo.is_open()) return "";

        long total_kb = 0, free_kb = 0;
        std::string line;

        while (std::getline(meminfo, line)) {
            if (line.find("SwapTotal:") == 0) {
                total_kb = std::stol(line.substr(10));
            } else if (line.find("SwapFree:") == 0) {
                free_kb = std::stol(line.substr(9));
            }
        }

        if (total_kb > 0) {
            long used_kb = total_kb - free_kb;
            float total_gb = total_kb / 1048576.0;
            float used_gb = used_kb / 1048576.0;
            int percent = (used_kb * 100) / total_kb;

            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << used_gb << " GiB / " << total_gb << " GiB (" << percent << "%)";
            return ss.str();
        }
        return "";
    }

    static std::string get_disk() {
        std::string df = run_cmd("df -h / 2>/dev/null | tail -1");
        if (!df.empty()) {
            std::istringstream iss(df);
            std::vector<std::string> tokens;
            std::string token;

            while (iss >> token) {
                tokens.push_back(token);
            }

            if (tokens.size() >= 6) {
                std::string used = tokens[2];
                std::string total = tokens[1];
                std::string percent = tokens[4];

                std::string filesystem = run_cmd("findmnt -n -o FSTYPE / 2>/dev/null");
                if (filesystem.empty()) {
                    filesystem = run_cmd("lsblk -f / 2>/dev/null | tail -1 | awk '{print $2}'");
                }

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
                    return used + " / " + total + " (" + percent + ") - " + filesystem;
                }
            }
        }
        return "";
    }

    static std::string get_local_ip() {
        struct ifaddrs *ifaddr, *ifa;
        if (getifaddrs(&ifaddr) == -1) return "";

        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr) continue;

            if (ifa->ifa_addr->sa_family == AF_INET) {
                std::string ifname = ifa->ifa_name;
                if (ifname.find("lo") == 0) continue;

                char host[NI_MAXHOST];
                int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                    host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
                if (s == 0) {
                    struct sockaddr_in *netmask_addr = (struct sockaddr_in *)ifa->ifa_netmask;
                    if (netmask_addr != nullptr) {
                        unsigned long netmask_val = ntohl(netmask_addr->sin_addr.s_addr);
                        int prefix_length = 0;
                        while (netmask_val) {
                            prefix_length += netmask_val & 1;
                            netmask_val >>= 1;
                        }

                        freeifaddrs(ifaddr);
                        std::stringstream ss;
                        ss << "(" << ifname << "): " << host << "/" << prefix_length;
                        return ss.str();
                    }
                }
            }
        }

        freeifaddrs(ifaddr);
        return "";
    }

    static std::string get_locale() {
        char* locale = getenv("LANG");
        if (locale != nullptr) {
            return std::string(locale);
        }
        return "";
    }
};

// Get terminal width - Improved version
int get_terminal_width() {
    // First try ioctl
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        return w.ws_col;
    }

    // Then try environment variables
    char* columns = getenv("COLUMNS");
    if (columns) {
        try {
            int cols = std::stoi(columns);
            if (cols > 0) return cols;
        } catch (...) {
            // Ignore conversion errors
        }
    }

    // Try tput command as fallback
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("tput cols 2>/dev/null", "r"), pclose);
    if (pipe) {
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        if (!result.empty()) {
            try {
                int cols = std::stoi(result);
                if (cols > 0) return cols;
            } catch (...) {
                // Ignore conversion errors
            }
        }
    }

    // Default fallback
    return 120;
}

// UI class for claudefetch-style display
class ClaudeFetchUI {
public:
    static void print_label_value(const std::string& label, const std::string& value, int line_num = 0) {
        // Fixed starting position - 3cm from ASCII art (approx 12 character spaces)
        // ASCII art is 20 chars wide + 12 spaces = 32
        int label_start = 32;

        // Calculate max label width
        static const int MAX_LABEL_WIDTH = 12;

        if (line_num < AsciiArt::ARCH.size()) {
            // Print the ASCII art line
            std::cout << AsciiArt::ARCH[line_num];

            // Add spaces for 3cm gap
            std::cout << std::string(12, ' ');

            // Print label and value - Both in cyan, values are BOLD
            std::cout << Colors::BOLD << Colors::L_CYAN
            << std::left << std::setw(MAX_LABEL_WIDTH) << label
            << Colors::RESET << Colors::BOLD << Colors::L_CYAN << value << Colors::RESET;
        } else {
            // For lines beyond ASCII art, just indent
            std::cout << std::string(label_start, ' ')
            << Colors::BOLD << Colors::L_CYAN
            << std::left << std::setw(MAX_LABEL_WIDTH) << label
            << Colors::RESET << Colors::BOLD << Colors::L_CYAN << value << Colors::RESET;
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
        std::cout << "\033[2J\033[1;1H";

        // Get all system info - NO DEFAULTS
        std::string os = SystemInfo::get_os();
        std::string host = SystemInfo::get_host();
        std::string kernel = SystemInfo::get_kernel();
        std::string uptime = SystemInfo::get_uptime();
        std::string packages = SystemInfo::get_packages();
        std::string shell = SystemInfo::get_shell();
        std::string display = SystemInfo::get_display();
        std::string de = SystemInfo::get_de();
        std::string wm = SystemInfo::get_wm();
        std::string wm_theme = SystemInfo::get_wm_theme();
        std::string theme = SystemInfo::get_theme();
        std::string icons = SystemInfo::get_icons();
        std::string font = SystemInfo::get_font();
        std::string cursor = SystemInfo::get_cursor();
        std::string terminal = SystemInfo::get_terminal();
        std::string cpu = SystemInfo::get_cpu();
        std::string gpu = SystemInfo::get_gpu();
        std::string memory = SystemInfo::get_memory();
        std::string swap = SystemInfo::get_swap();
        std::string disk = SystemInfo::get_disk();
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
        print_label_value("WM Theme:", wm_theme, line_num++);

        // Line 11: Theme
        if (theme.length() > 60) {
            theme = theme.substr(0, 57) + "...";
        }
        print_label_value("Theme:", theme, line_num++);

        // Line 12: Icons
        if (icons.length() > 60) {
            icons = icons.substr(0, 57) + "...";
        }
        print_label_value("Icons:", icons, line_num++);

        // Line 13: Font
        if (font.length() > 60) {
            font = font.substr(0, 57) + "...";
        }
        print_label_value("Font:", font, line_num++);

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

        // Line 20: Disk
        print_label_value("Disk (/):", disk, line_num++);

        // Line 21: Local IP
        print_label_value("Local IP:", local_ip, line_num++);

        // Line 22: Locale
        print_label_value("Locale:", locale, line_num++);

        // Line 23: Version - Now with proper label
        std::string version = " [v1.0 26-01-2026]";
        print_label_value("claudefetch Version:", version, line_num++);

        // Add any remaining ASCII art lines
        while (line_num < AsciiArt::ARCH.size()) {
            std::cout << AsciiArt::ARCH[line_num] << std::endl;
            line_num++;
        }

        std::cout << Colors::BOLD << Colors::L_CYAN << std::string(80, '=') << Colors::RESET << std::endl;
    }

    static void display_menu() {
        std::cout << Colors::BOLD << Colors::L_CYAN << "claudefetch System Maintenance Menu:" << Colors::RESET << std::endl;
        std::cout << Colors::BOLD << Colors::L_CYAN << "  (A)udit Full System" << Colors::L_CYAN << "   "
        << Colors::BOLD << Colors::L_CYAN << "(C)heck Updates" << Colors::L_CYAN << "   "
        << Colors::BOLD << Colors::L_CYAN << "(F)ree Memory" << Colors::RESET << std::endl;
        std::cout << Colors::BOLD << Colors::L_CYAN << "  (R)emove Cache" << Colors::L_CYAN << "        "
        << Colors::BOLD << Colors::L_CYAN << "(U)pdate System" << Colors::L_CYAN << "       "
        << Colors::BOLD << Colors::L_CYAN << "(Q)uit to Shell" << Colors::RESET << std::endl;
        std::cout << Colors::BOLD << Colors::L_CYAN << "\nSelect option: " << Colors::RESET;
        std::cout.flush();
    }
};

// Actions class
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
        std::cout << Colors::L_CYAN << "Executing: " << command << Colors::RESET << std::endl;
        system(command.c_str());
        std::cout << Colors::L_CYAN << "\nCommand completed. Press Enter to continue..." << Colors::RESET << std::endl;
        std::string dummy;
        std::getline(std::cin, dummy);
    }
}

void selection() {
    char choice;
    std::cin >> choice;
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
