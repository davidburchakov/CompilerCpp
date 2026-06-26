//
// Created by incidence on 6/23/26.
//

module;

// Global Module Fragment: Include non-module ANTLR runtime headers
#include "antlr4-runtime.h"
#include "CppBaseVisitor.h"
#include "CppParser.h"
#include <string>
#include <any>

export module SymbolTableVisitorModule;

import SymbolTableModule;
import Reports;

namespace CppZero {

    export class SymbolTableVisitor : public CppBaseVisitor {
    private:
        SymbolTable &symbolTable;
        Reports<Report> &reports;
        // Recursive helper to unwrap the nested declarator rule variants (*, &, &&, [])
        void unwindDeclarator(CppParser::DeclaratorContext *ctx, Symbol &outSymbol) {
            if (!ctx) return;

            // 1. Base Case: Reached the core variable name
            if (auto baseCtx = dynamic_cast<CppParser::BaseIdentifierContext*>(ctx)) {
                outSymbol.name = baseCtx->IDENTIFIER()->getText();
                outSymbol.declaration_line = baseCtx->IDENTIFIER()->getSymbol()->getLine();
                return;
            }

            // 2. Recursive Case: Pointer (*)
            if (auto ptrCtx = dynamic_cast<CppParser::PointerModifierContext*>(ctx)) {
                outSymbol.type.pointer_count++;
                unwindDeclarator(ptrCtx->declarator(), outSymbol);
                return;
            }

            // 3. Recursive Case: Lvalue Reference (&)
            if (auto lrefCtx = dynamic_cast<CppParser::LvalueRefModifierContext*>(ctx)) {
                outSymbol.type.is_lvalue_reference = true;
                unwindDeclarator(lrefCtx->declarator(), outSymbol);
                return;
            }

            // 4. Recursive Case: Rvalue Reference (&&)
            if (auto rrefCtx = dynamic_cast<CppParser::RvalueRefModifierContext*>(ctx)) {
                outSymbol.type.is_rvalue_reference = true;
                unwindDeclarator(rrefCtx->declarator(), outSymbol);
                return;
            }

            // 5. Recursive Case: Array Bracket Layout ([])
            if (auto arrCtx = dynamic_cast<CppParser::ArrayModifierContext*>(ctx)) {
                outSymbol.type.array_dimensions++;
                unwindDeclarator(arrCtx->declarator(), outSymbol);
                return;
            }
        }

        // Helper to loop through the parsed prefix modifier sequence
        void extractModifiers(CppParser::DeclarationModifiersContext* ctx, Type& outType) {
            // 1. Check if the context block itself is null or has no elements using .empty()
            if (!ctx || ctx->typeModifier().empty()) {
                // Do nothing, but we still check storage specifiers below even if typeModifiers is empty
            } else {
                for (auto* modifier : ctx->typeModifier()) {
                    std::string text = modifier->getText();
                    if (text == "const")    outType.is_const = true;
                    if (text == "volatile") outType.is_const = true;
                }
            }

            // 2. Use .empty() to see if any storage modifiers (static, extern...) were parsed
            if (ctx && !ctx->storageSpecifier().empty()) {
                for (auto* storage : ctx->storageSpecifier()) {
                    std::string text = storage->getText();
                    if (text == "static")       outType.storage = StorageClass::kStatic;
                    if (text == "extern")       outType.storage = StorageClass::kExtern;
                    if (text == "thread_local") outType.storage = StorageClass::kThreadLocal;
                    if (text == "mutable")      outType.storage = StorageClass::kMutable;
                }
            }
        }


    public:
        explicit SymbolTableVisitor(SymbolTable& table, Reports<Report> &reports)
            : symbolTable(table), reports(reports) {}

        // Handle raw declarations: "const int *x;"
        std::any visitDeclaration(CppParser::DeclarationContext *ctx) override {

            Symbol symbol;
            symbol.type.base_type = CppZero::NormalizeType(ctx->primitiveType()->getText());

            extractModifiers(ctx->declarationModifiers(), symbol.type);
            unwindDeclarator(ctx->declarator(), symbol);
            symbol.tree_node_name = "variableDeclaration";

            if(!symbolTable.Insert(symbol.name, symbol)) {
                reports.errors.emplace_back("Duplicate declaration of variable " + symbol.name +
                    " at line: " + std::to_string(symbol.declaration_line) + ", during " + symbol.tree_node_name, ErrorCodeEnum::FAILURE);
            }

            return visitChildren(ctx);
        }

        // Handle initialized declarations: "static double arr[][] = expression;"
        std::any visitInitialization(CppParser::InitializationContext *ctx) override {
            Symbol symbol;
            symbol.type.base_type = ctx->primitiveType()->getText();

            extractModifiers(ctx->declarationModifiers(), symbol.type);
            unwindDeclarator(ctx->declarator(), symbol);
            symbol.tree_node_name = "variableInitialization";

            if(!symbolTable.Insert(symbol.name, symbol)) {
                reports.errors.emplace_back("Duplicate declaration of variable " + symbol.name +
                    " at line: " + std::to_string(symbol.declaration_line) + ", during " + symbol.tree_node_name, ErrorCodeEnum::FAILURE);
            }
            return visitChildren(ctx);
        }
    };

} // namespace CppZero
