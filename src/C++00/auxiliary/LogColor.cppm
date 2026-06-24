//
// Created by David Burchakov on 6/24/26.
//
module;

#include <string>
#include <string_view>
export module LogColor;


export namespace CppZero {
    class LogColor {
    public:
        // Core Control
        static constexpr std::string_view RESET          = "\033[0m";
        static constexpr std::string_view BOLD           = "\033[1m";
        static constexpr std::string_view DIM            = "\033[2m";
        static constexpr std::string_view ITALIC         = "\033[3m";
        static constexpr std::string_view UNDERLINE      = "\033[4m";

        // Standard Colors
        static constexpr std::string_view RED            = "\033[31m";
        static constexpr std::string_view GREEN          = "\033[32m";
        static constexpr std::string_view YELLOW         = "\033[33m";
        static constexpr std::string_view BLUE           = "\033[34m";
        static constexpr std::string_view MAGENTA        = "\033[35m";
        static constexpr std::string_view CYAN           = "\033[36m";
        static constexpr std::string_view WHITE          = "\033[37m";

        // Expanded Secondary Colors
        static constexpr std::string_view ORANGE         = "\033[38;5;208m"; // 256-color Mode Index 208
        static constexpr std::string_view VIOLET         = "\033[38;5;135m"; // 256-color Mode Index 135
        static constexpr std::string_view PINK           = "\033[38;5;211m"; // 256-color Mode Index 211
        static constexpr std::string_view TEAL           = "\033[38;5;37m";  // 256-color Mode Index 37
        static constexpr std::string_view GOLD           = "\033[38;5;220m"; // 256-color Mode Index 220
    };

    // Helper operator to allow seamless: "text" + LogColor::RED string operations
    inline std::string operator+(std::string_view lhs, std::string_view rhs) {
        std::string result;
        result.reserve(lhs.size() + rhs.size());
        result.append(lhs);
        result.append(rhs);
        return result;
    }

    inline std::string operator+(std::string&& lhs, std::string_view rhs) {
        lhs.append(rhs);
        return std::move(lhs);
    }
}

