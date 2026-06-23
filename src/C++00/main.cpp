// ============================================================
// 1. TRADITIONAL HEADERS & LEGACY INCLUDES
// ============================================================
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDebug>
#include <fstream>
#include <iostream>
#include <stdexcept> // Required for std::runtime_error

// Wipe out Qt's 'emit' keyword macro mapping before loading ANTLR
#undef emit

#include "antlr4-runtime.h"
#include "CppLexer.h"
#include "CppParser.h"
#include "../Qt/mainwindow.h"

// ============================================================
// 2. MODERN C++23 MODULE IMPORTS
// ============================================================
import SymbolTableModule;
import SymbolTableVisitorModule;
import Logger;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Load your input source stream file
    std::ifstream fileStream("/home/incidence/Desktop/CompilerCpp/src/C++00/input.txt");
    if (!fileStream.is_open()) {
        qCritical() << "Semantic Error: Could not locate input.txt in the runtime folder!";
        return 1;
    }

    // Feed the data through the Lexer and generate tokens
    antlr4::ANTLRInputStream input(fileStream);
    CppLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);

    // Instantiate the parser and build the AST
    CppParser parser(&tokens);
    CppParser::TranslationUnitContext* tree = parser.translationUnit();

    // Throw an exception if any syntax errors exist
    if (parser.getNumberOfSyntaxErrors() > 0) {
        throw std::runtime_error("Fatal: Compilation aborted due to " +
                                 std::to_string(parser.getNumberOfSyntaxErrors()) +
                                 " syntax error(s).");
    }

    // This code only runs if the code above has 0 errors
    CppZero::Logger::printPrettyAST(tree, parser.getRuleNames());

    // Instantiate Table and Visitor data containers
    CppZero::SymbolTable symbolTable;
    CppZero::SymbolTableVisitor visitor(symbolTable);

    // Run the traversal pass to capture symbols
    visitor.visit(tree);

    symbolTable.printAll();

    qInfo() << "AST semantic processing complete. Launching interface tree viewer...";

    // Launch GUI View
    MainWindow window;
    window.show();

    return QApplication::exec();
}
