module;

// 1. Global Module Fragment: Keep legacy includes isolated up here
#include <string>
#include <unordered_map>
#include <iostream>
#include <array>

// 2. Begin C++23 Module Declaration context
export module SymbolTableModule;

namespace CppZero {

    // Tracks lifetime rules and memory layout scopes
    export enum class StorageClass {
        Auto,
        Static,
        Extern,
        ThreadLocal,
        Mutable
    };

    export struct TypeAliasGroup {
        std::string_view canonicalType;
        std::array<std::string_view, 8> aliases; // Max room for 8 strings per group
        size_t aliasCount;
    };

    export inline constexpr std::array<TypeAliasGroup, 12> primitiveTypes {{
        { "int32_t",   { "int32_t", "int", "long", "DWORD" }, 4 },
        { "int128_t",  { "int128_t", "longlonglonglong", "llll", "OCTOWORD" }, 4 },
        { "int64_t",   { "int64_t", "longlong", "ll", "QWORD" }, 4 },
        { "int16_t",   { "int16_t", "short", "WORD" }, 3 },
        { "int8_t",    { "int8_t", "char", "BYTE" }, 3 },
        { "bool",      { "bool", "boolean", "true", "True", "yes", "false", "False", "no" }, 8 },
        { "void",      { "void" }, 1 },
        { "double",    { "double" }, 1 },
        { "float",     { "float" }, 1 },
        { "char16_t",  { "char16_t" }, 1 },
        { "char32_t",  { "char32_t" }, 1 },
        { "wchar_t",   { "wchar_t" }, 1 }
    }};

    export constexpr std::string_view normalizeType(std::string_view rawType) {
        for (const auto& group : primitiveTypes) {
            for (size_t i = 0; i < group.aliasCount; ++i) {
                if (group.aliases[i] == rawType) {
                    return group.canonicalType; // Match found! Return the standard name
                }
            }
        }

        // No match found anywhere in the compile-time array mapping tiers.
        // We throw an exception directly. (C++20/C++23 fully supports throwing exceptions in constexpr blocks!)
        throw std::runtime_error("Compile-Time Semantic Error: Invalid or unrecognized primitive type identifier '" +
                                 std::string(rawType) + "'");
    }

    // Tracks the structural signature of the data format
    export struct Type {
        std::string baseType;           // e.g., "int", "double", "MyClass"
        int pointerCount = 0;           // 0 = raw, 1 = ptr*, 2 = ptr**
        int arrayDimensions = 0;        // 0 = single element, 1 = arr[], 2 = matrix[][]
        bool isLvalueReference = false; // true = standard reference (&)
        bool isRvalueReference = false; // true = move reference (&&)
        bool isConst = false;           // true = constant modifier
        bool isVolatile = false;        // true = volatile modifier
        StorageClass storage = StorageClass::Auto;
    };

    // Tracks the concrete variable instance identity
    export struct Symbol {
        std::string name;          // Variable name identifier (e.g., "matrix")
        Type type;                 // Composition mapping block
        int declarationLine = 0;   // Line mapping location context for compiler debugging
    };

    // Container matching scope mappings
    export class SymbolTable {
    private:
        std::unordered_map<std::string, Symbol> table;

    public:
        void insert(const std::string& name, const Symbol& symbol) {
            if (table.contains(name)) {
                std::cerr << "Semantic Error: Redefinition of variable '" << name
                          << "' on line " << symbol.declarationLine << "\n";
                return;
            }
            table[name] = symbol;
        }

        bool exists(const std::string& name) const {
            return table.contains(name);
        }

        Symbol get(const std::string& name) const {
            if (table.contains(name)) {
                return table.at(name);
            }
            return Symbol{};
        }

        // 🚀 NEW: Iterates and prints the exact structural layout of every collected variable
        void printAll() const {
            std::cout << "\n========================================";
            std::cout << "\n  SYMBOL TABLE CONTENTS (CppZero)";
            std::cout << "\n========================================\n";

            if (table.empty()) {
                std::cout << " (No variables declared)\n";
                std::cout << "========================================\n";
                return;
            }

            for (const auto& [name, symbol] : table) {
                std::cout << "• Variable: " << name << "\n";
                std::cout << "  - Line:       " << symbol.declarationLine << "\n";

                // Print Modifiers
                std::cout << "  - Modifiers:  ";
                if (symbol.type.isConst) std::cout << "const ";
                if (symbol.type.isVolatile) std::cout << "volatile ";

                if (symbol.type.storage == StorageClass::Static) std::cout << "[static] ";
                if (symbol.type.storage == StorageClass::Extern) std::cout << "[extern] ";
                if (symbol.type.storage == StorageClass::ThreadLocal) std::cout << "[thread_local] ";
                std::cout << "\n";

                // Print Type Shape
                std::cout << "  - Base Type:  " << symbol.type.baseType << "\n";
                std::cout << "  - Pointers:   " << std::string(symbol.type.pointerCount, '*') << " (" << symbol.type.pointerCount << ")\n";
                std::cout << "  - References: ";
                if (symbol.type.isLvalueReference) std::cout << "lvalue (&)";
                else if (symbol.type.isRvalueReference) std::cout << "rvalue (&&)";
                else std::cout << "none";
                std::cout << "\n";

                // Print Array Shape
                std::cout << "  - Dimensions: " << symbol.type.arrayDimensions << " "
                          << std::string(symbol.type.arrayDimensions * 2, ']') // Emulates [][] layouts
                          << "\n";
                std::cout << "----------------------------------------\n";
            }
        }
    };

} // namespace CppZero
