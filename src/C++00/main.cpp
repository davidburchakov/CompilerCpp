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
import SemanticAnalysis;
import ASTOptimizer;
import AssemblyGenerator;
import Logger;
import Reports;

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
    CppZero::Reports<CppZero::Report> symbol_table_reports;
    CppZero::SymbolTableVisitor visitor(symbolTable, symbol_table_reports);

    // Run the traversal pass to capture symbols
    visitor.visit(tree);
    symbolTable.PrintAll();

    if (!symbol_table_reports.noErrors()) {
        for (const CppZero::Report &error: symbol_table_reports.errors) {
            std::cout << error.reportMsg << '\n';
        }
        // throw std::runtime_error("Semantic errors during symbol table build-up");
    }

    CppZero::Reports<CppZero::Report> semantic_analysis_reports;
    CppZero::SemanticAnalysis semantic_analysis(symbolTable, semantic_analysis_reports);
    semantic_analysis.analyse(tree);

    if (!semantic_analysis_reports.noErrors()) {
        for (const CppZero::Report &error: semantic_analysis_reports.errors) {
            std::cout << error.reportMsg << '\n';
        }
        // throw std::runtime_error("Semantic errors during semantic analysis");
    }


    // ============================================================
    // 3. OPTIMIZATION PHASE (Constant Folding Calculation)
    // ============================================================
    std::cout << "\nStarting Optimization Pass...\n";
    CppZero::ASTOptimizer optimizer;
    std::any optimizationResult = optimizer.optimize(tree);

    if (optimizationResult.type() == typeid(int)) {
        std::cout << "Successfully folded expression value to: "
                  << std::any_cast<int>(optimizationResult) << "\n";
    }

    // ============================================================
    // 4. TRANSLATION TO ASSEMBLY PHASE
    // ============================================================
    std::cout << "\nStarting Code Generation (Translating to x86-64 Assembly)...\n";
    CppZero::AssemblyGenerator generator(optimizationResult);
    std::string finalAssembly = generator.generateAssembly(tree);

    std::cout << "--- Generated Assembly Output ---\n" << finalAssembly << "---------------------------------\n";


    qInfo() << "AST semantic processing complete. Launching interface tree viewer...";

    // Launch GUI View and pass the finalAssembly variable block inside
    MainWindow window;
    window.setOptimizedAssemblyText(finalAssembly);
    window.show();

    delete tree;
    return QApplication::exec();
}
