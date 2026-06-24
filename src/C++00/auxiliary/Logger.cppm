//
// Created by David Burchakov on 6/23/26.
//
module;

#include <iostream>
#include <string>
#include <vector>
#include "antlr4-runtime.h"

export module Logger;

namespace CppZero::Logger {

    // 3. Mark the printing loop utility function as 'export' so main can see it
    export void printPrettyAST(antlr4::tree::ParseTree *node, const std::vector<std::string> &ruleNames, int depth = 0) {
        if (!node) return;

        // Generate Indentation visual padding
        std::string indent = "";
        for (int i = 0; i < depth; ++i) {
            indent += (i == depth - 1) ? " ├── " : " │   ";
        }

        // Identify and print the Node label
        std::string nodeName;
        if (auto* r = dynamic_cast<antlr4::ParserRuleContext*>(node)) {
            nodeName = ruleNames[r->getRuleIndex()]; // Rule Name (e.g. "variableDeclaration")
        } else {
            nodeName = node->getText(); // Raw Token text (e.g. "x", ";", "const")
        }

        // Clean up empty rule logs
        if (nodeName != "declarationModifiers" || node->children.size() > 0) {
            std::cout << indent << nodeName << "\n";
        }

        // Recurse down into Child Nodes
        for (size_t i = 0; i < node->children.size(); ++i) {
            printPrettyAST(node->children[i], ruleNames, depth + 1);
        }
    }

} // namespace CppZero::Logger
