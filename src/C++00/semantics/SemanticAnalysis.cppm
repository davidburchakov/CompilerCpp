//
// Created by David Burchakov on 6/24/26.
//

module;
#include "antlr4-runtime.h"

export module SemanticAnalysis;

/* imports go here */
import SymbolTableModule;
import SemanticVisitor;
import Reports;

export namespace CppZero {
    class SemanticAnalysis {
    public:
        SymbolTable &symbol_table;
        Reports<Report> &reports;

        SemanticAnalysis(SymbolTable &symbol_table, Reports<Report> &reports)
            : symbol_table(symbol_table), reports(reports) { }

        // Pass down your root AST context from your parser entry point
        void analyse(antlr4::tree::ParseTree *tree) {
            /* static analysis based off the symbol table can go here */

            /* runtime analysis requiring AST validation checks */
            SemanticVisitor visitor(symbol_table, reports);
            visitor.visit(tree); // Triggers the traversal pass
        }
    };
}
