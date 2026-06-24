//
// Created by David Burchakov on 6/24/26.
//
module;
#include <any>
#include <sstream>
#include <string>
#include <iostream>
#include "CppBaseVisitor.h"
#include "CppParser.h"

export module AssemblyGenerator;

export namespace CppZero {
    class AssemblyGenerator : public CppBaseVisitor {
    private:
        std::stringstream asmOutput;
        std::any optimizedTreeResults; // Holds references to folded nodes

    public:
        explicit AssemblyGenerator(std::any optimizedData)
            : optimizedTreeResults(std::move(optimizedData)) {}

        std::string generateAssembly(antlr4::tree::ParseTree* tree) {
            asmOutput.clear();

            // Boilerplate assembly generation setup
            asmOutput << ".global main\n";
            asmOutput << "main:\n";
            asmOutput << "    pushq %rbp\n";
            asmOutput << "    movq %rsp, %rbp\n";

            // Traverse the AST to append calculations
            visit(tree);

            asmOutput << "    popq %rbp\n";
            asmOutput << "    ret\n";
            return asmOutput.str();
        }

        // Intercept optimization values or calculate fallback sequence actions
        virtual std::any visitAddition(CppParser::AdditionContext *ctx) override {
            // Check if our optimization engine pre-computed a value for this block node
            if (optimizedTreeResults.type() == typeid(int)) {
                int value = std::any_cast<int>(optimizedTreeResults);
                asmOutput << "    movl $" << value << ", %eax\n";
                return value;
            }

            // Fallback un-optimized code generation if non-constants are added
            visit(ctx->children[0]); // Load left side to %eax
            asmOutput << "    pushq %rax\n";
            visit(ctx->children[2]); // Load right side to %eax
            asmOutput << "    movl %eax, %ebx\n";
            asmOutput << "    popq %rax\n";
            asmOutput << "    addl %ebx, %eax\n";
            return std::any();
        }

        virtual std::any visitIntLiteral(CppParser::IntLiteralContext *ctx) override {
            asmOutput << "    movl $" << ctx->getText() << ", %eax\n";
            return std::stoi(ctx->getText());
        }
    };
}
