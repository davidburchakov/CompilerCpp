//
// Created by David Burchakov on 6/24/26.
//

module;
/* Traditional headers go here */

export module SemanticAnalysis;

/* imports go here */
import SymbolTableModule;
import SemanticVisitor;

export namespace CppZero {
    class SemanticAnalysis {
    public:
        void analyse(SymbolTable &symbolTable) {
            /* static analysis */
            /* is based off of the information already gathered in the symbol table */


            /* runtime analysis */
            /* additionally requires runtime AST checks */
            CppZero::SemanticVisitor visitor;

        }
    };
};