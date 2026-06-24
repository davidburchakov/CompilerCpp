//
// Created by David Burchakov on 6/24/26.
//
module;
#include <any>
#include <string>
#include "CppBaseVisitor.h"
#include "CppParser.h"

export module ASTOptimizer;

export namespace CppZero {
    class ASTOptimizer : public CppBaseVisitor {
    public:
        ASTOptimizer() = default;

        // Base entry point
        std::any optimize(antlr4::tree::ParseTree* tree) {
            return visit(tree);
        }

        virtual std::any visitIntLiteral(CppParser::IntLiteralContext *ctx) override {
            return std::stoi(ctx->getText());
        }


        virtual std::any visitVarDeclStatement(CppParser::VarDeclStatementContext *ctx) override {
            return visitChildren(ctx);
        }

private:
    // Recursively flattens left-leaning addition trees into a vector of numbers
    void collectConstants(antlr4::tree::ParseTree* node, std::vector<int>& values, bool& isValidSequence) {
        if (!isValidSequence) return;

        // 1. If it's another addition node, keep drilling down left and right children
        if (auto* addCtx = dynamic_cast<CppParser::AdditionContext*>(node)) {
            if (addCtx->children.size() >= 3) {
                collectConstants(addCtx->children[0], values, isValidSequence); // Left sub-tree
                collectConstants(addCtx->children[2], values, isValidSequence); // Right sub-tree
            }
        }
        // 2. If it's a leaf integer node, capture the number
        else if (auto* intCtx = dynamic_cast<CppParser::IntLiteralContext*>(node)) {
            values.push_back(std::stoi(intCtx->getText()));
        }
        // 3. Strip parentheses wrappers if present
        else if (auto* parenCtx = dynamic_cast<CppParser::ParentExpressionContext*>(node)) {
            if (parenCtx->children.size() >= 2) {
                collectConstants(parenCtx->children[1], values, isValidSequence);
            }
        }
        // 4. Any variables or non-integers break our specific formula track
        else {
            isValidSequence = false;
        }
    }

    // Checks if the collected array matches exactly 1, 2, 3, ..., n
    bool verifyContiguousSequence(const std::vector<int>& values) {
        if (values.empty() || values[0] != 1) return false;

        for (size_t i = 1; i < values.size(); ++i) {
            if (values[i] != values[i - 1] + 1) {
                return false; // Broken sequence sequence chain (e.g. 1 + 2 + 4 + 5)
            }
        }
        return true;
    }

public:
    virtual std::any visitAddition(CppParser::AdditionContext *ctx) override {
        std::vector<int> collectedValues;
        bool isValidSequence = true;

        // Attempt to flatten this addition subtree
        collectConstants(ctx, collectedValues, isValidSequence);

        // If the subtree is a pure integer sequence starting at 1 (like 1+2+3+4+5)
        if (isValidSequence && verifyContiguousSequence(collectedValues)) {
            int n = collectedValues.back(); // Get the upper bound limit '5'

            std::cout << "[Optimization] Detected sequential sum up to n=" << n
                      << ". Substituting with algebraic formula! n * (n + 1) / 2 \n";

            // Apply O(1) formula: (n * (n + 1)) / 2
            return (n * (n + 1)) / 2;
        }

        // Fallback to standard constant folding if it's a normal mixed addition (e.g., x + y + 2)
        std::any left = visit(ctx->children[0]);
        std::any right = visit(ctx->children[2]);
        if (left.type() == typeid(int) && right.type() == typeid(int)) {
            return std::any_cast<int>(left) + std::any_cast<int>(right);
        }

        return visitChildren(ctx);
    }

    };
}
