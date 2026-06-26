module;

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "absl/container/flat_hash_map.h"

export module SymbolTableModule;

namespace CppZero {
    export enum class StorageClass {
        kAuto,
        kStatic,
        kExtern,
        kThreadLocal,
        kMutable
    };

    export struct TypeAliasGroup {
        std::string_view canonical_type;         // base type, or, hypernym
        std::array<std::string_view, 8> aliases; // 8 aliases at most declared in lexer rules
        size_t alias_count;
    };

    // these types are described in Lexer rules. Known at compile time
    export inline constexpr std::array<TypeAliasGroup, 12> kPrimitiveTypes{
        {
            {"int32_t", {"int32_t", "int", "long", "DWORD"}, 4},
            {"int128_t", {"int128_t", "longlonglonglong", "llll", "OCTOWORD"}, 4},
            {"int64_t", {"int64_t", "longlong", "ll", "QWORD"}, 4},
            {"int16_t", {"int16_t", "short", "WORD"}, 3},
            {"int8_t", {"int8_t", "char", "BYTE"}, 3},
            {
                "bool",
                {"bool", "boolean", "true", "True", "yes", "false", "False", "no"},
                8
            },
            {"void", {"void"}, 1},
            {"double", {"double"}, 1},
            {"float", {"float"}, 1},
            {"char16_t", {"char16_t"}, 1},
            {"char32_t", {"char32_t"}, 1},
            {"wchar_t", {"wchar_t"}, 1},
        }
    };

    export constexpr std::string_view NormalizeType(const std::string_view raw_type) {
        for (const auto &group: kPrimitiveTypes) {
            for (size_t i = 0; i < group.alias_count; ++i) {
                if (group.aliases[i] == raw_type) {
                    return group.canonical_type;
                }
            }
        }

        // C++23 supports throwing exceptions in compile-time evaluation failures. Should actually never throw
        throw std::runtime_error(
            "Compile-Time Semantic Error: Invalid or unrecognized primitive type identifier '" +
                std::string(raw_type) + "'");
    }

    export struct Type {
        std::string base_type; // e.g., "int", "double", "MyClass"
        int pointer_count = 0;
        int array_dimensions = 0;
        bool is_lvalue_reference = false;
        bool is_rvalue_reference = false;
        bool is_const = false;
        bool is_volatile = false;
        StorageClass storage = StorageClass::kAuto;
    };

    export struct Symbol {
        std::string name;            // Identifier
        std::string tree_node_name;  // Abstract Syntax Tree Node
        Type type;                   // info, e.g., is_const, storage type
        int declaration_line = 0;    // respective line
    };

    export class SymbolTable {
    public:
        bool Insert(absl::string_view name, const Symbol &symbol, bool log = false) {
            if (symbol_table_.contains(name)) {
                if (log) {
                    std::cerr << "Semantic Error (Symbol Table Insert()): Redefinition of "
                            "variable '"
                            << name << "' on line " << symbol.declaration_line << "\n";
                }
                return false;
            }
            // Heterogeneous lookup allows lazy allocation only when inserting a new key
            symbol_table_.emplace(name, symbol);
            return true;
        }

        bool Exists(absl::string_view name) const { return symbol_table_.contains(name); }

        Symbol Get(absl::string_view name) const {
            auto it = symbol_table_.find(name);
            if (it != symbol_table_.end()) {
                return it->second;
            }
            return Symbol{};
        }

        void PrintAll() const {
            std::cout << "\n========================================";
            std::cout << "\n  SYMBOL TABLE CONTENTS (CppZero)";
            std::cout << "\n========================================\n";

            if (symbol_table_.empty()) {
                std::cout << " (No variables declared)\n";
                std::cout << "========================================\n";
                return;
            }

            for (const auto &[name, symbol]: symbol_table_) {
                std::cout << "• Variable: " << name << "\n";
                std::cout << "  - Line:       " << symbol.declaration_line << "\n";

                // Print Modifiers
                std::cout << "  - Modifiers:  ";
                if (symbol.type.is_const) std::cout << "const ";
                if (symbol.type.is_volatile) std::cout << "volatile ";

                if (symbol.type.storage == StorageClass::kStatic) std::cout << "[static] ";
                if (symbol.type.storage == StorageClass::kExtern) std::cout << "[extern] ";
                if (symbol.type.storage == StorageClass::kThreadLocal) {
                    std::cout << "[thread_local] ";
                }
                std::cout << "\n";

                // Print Type Shape
                std::cout << "  - Base Type:  " << symbol.type.base_type << "\n";
                std::cout << "  - Pointers:   " << std::string(symbol.type.pointer_count, '*')
                        << " (" << symbol.type.pointer_count << ")\n";
                std::cout << "  - References: ";
                if (symbol.type.is_lvalue_reference) {
                    std::cout << "lvalue (&)";
                } else if (symbol.type.is_rvalue_reference) {
                    std::cout << "rvalue (&&)";
                } else {
                    std::cout << "none";
                }
                std::cout << "\n";

                // Print Array Shape
                std::cout << "  - Dimensions: " << symbol.type.array_dimensions << " "
                        << std::string(symbol.type.array_dimensions * 2, ']')
                        << "\n";
                std::cout << "----------------------------------------\n";
            }
        }

    private:
        absl::flat_hash_map<std::string, Symbol> symbol_table_;
    };

} // namespace CppZero
