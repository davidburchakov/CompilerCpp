//
// Created by David Burchakov on 6/24/26.
//
module;

#include <any>
#include <string>
#include "CppBaseVisitor.h"
#include "CppParser.h"

export module SemanticVisitor;

import Reports;
import SymbolTableModule;

export namespace CppZero {
    class SemanticVisitor : public CppBaseVisitor {
    private:
        SymbolTable &symbolTable;
        Reports<Report> &reports;

        // Internal helper to cleanly unpack std::any results from child visitors
        std::string extractType(const std::any &result) {
            if (result.has_value() && result.type() == typeid(std::string)) {
                return std::any_cast<std::string>(result);
            }
            return "unknown";
        }

    public:
        explicit SemanticVisitor(SymbolTable &symbolTable, Reports<Report> &reports)
            : symbolTable(symbolTable), reports(reports) {
        }

        // ============================================================
        // 1. LITERAL LEAF NODES (Propagate types upward)
        // ============================================================

        virtual std::any visitIntLiteral(CppParser::IntLiteralContext *ctx) override {
            return std::string("int");
        }

        virtual std::any visitDoubleLiteral(CppParser::DoubleLiteralContext *ctx) override {
            return std::string("double");
        }

        virtual std::any visitCharLiteral(CppParser::CharLiteralContext *ctx) override {
            return std::string("char");
        }

        virtual std::any visitStringLiteral(CppParser::StringLiteralContext *ctx) override {
            return std::string("const char*");
        }

        virtual std::any visitLiteralTrue(CppParser::LiteralTrueContext *ctx) override {
            return std::string("bool");
        }

        // ============================================================
        // 2. IDENTIFIER LOOKUPS
        // ============================================================

        virtual std::any visitVariableIdentifier(CppParser::VariableIdentifierContext *ctx) override {
            std::string varName = ctx->getText();

            // Look up the symbol within the symbol table context
            if (symbolTable.Exists(varName)) {
                // Returns the underlying base type string stored during your first pass
                return symbolTable.Get(varName).type.base_type;
            }

            // Log an error if a variable is used without a prior definition context
            int line = ctx->getStart()->getLine();
            reports.errors.emplace_back(
                "Use of undeclared identifier '" + varName + "' at line " + std::to_string(line),
                ErrorCodeEnum::FAILURE
            );
            return std::string("undeclared " + varName);
        }

        // ============================================================
        // 3. OPERATOR EXPRESSIONS VALIDATION PASS
        // ============================================================

        virtual std::any visitAddition(CppParser::AdditionContext *ctx) override {
            // ANTLR expression sub-trees can be fetched via children elements array
            if (ctx->children.size() < 3) return std::string("unknown");

            // Evaluate types of left-hand side and right-hand side subexpressions
            std::string leftType = extractType(visit(ctx->children[0]));
            std::string rightType = extractType(visit(ctx->children[2]));

            // Semantic Check Rule: Warn on direct 'double' and 'int' mix operations
            if (leftType != "unknown" && rightType != "unknown") {
                if ((leftType == "double" && rightType == "int") ||
                    (leftType == "int" && rightType == "double")) {
                    int line = ctx->getStart()->getLine();
                    reports.warnings.emplace_back(
                        "addition expression mismatch: adding " + leftType + " type with " + rightType +
                        " at line " + std::to_string(line),
                        ErrorCodeEnum::WARNING
                    );
                }
            }

            // Type Promotion Rule: If either side is double, the expression evaluates to double
            if (leftType == "double" || rightType == "double") {
                return std::string("double");
            }
            return leftType;
        }

        // Pass-through wrapper so parenthesized blocks don't drop type values
        virtual std::any visitParentExpression(CppParser::ParentExpressionContext *ctx) override {
            if (ctx->children.size() >= 2) {
                return visit(ctx->children[1]); // Visit inside the parentheses
            }
            return visitChildren(ctx);
        }
    };
}
