#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDebug>
#include <fstream>
#include <iostream>
#include <stdexcept> 

#undef emit

#include "antlr4-runtime.h"
#include "CppLexer.h"
#include "CppParser.h"
#include "../Qt/mainwindow.h"

import SymbolTableModule;
import SymbolTableVisitorModule;
import SemanticAnalysis;
import ASTOptimizer;
import AssemblyGenerator;
import Logger;
import Reports;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;

    std::ifstream fileStream("/home/incidence/Desktop/CompilerCpp/src/C++00/input.txt");
    if (!fileStream.is_open()) {
        qCritical() << "Semantic Error: Could not locate input.txt in the runtime folder!";
        window.setErrorLogText("[Fatal Error] Could not locate input.txt file.");
        window.show();
        return app.exec();
    }

    antlr4::ANTLRInputStream input(fileStream);
    CppLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);

    CppParser parser(&tokens);
    CppParser::TranslationUnitContext* tree = parser.translationUnit();

    if (parser.getNumberOfSyntaxErrors() > 0) {
        std::string syntaxErrorLog = "[Syntax Error] Found " + 
                                     std::to_string(parser.getNumberOfSyntaxErrors()) + 
                                     " layout syntax errors. Check input source.";
        window.setErrorLogText(syntaxErrorLog);
        window.show();
        delete tree;
        return app.exec();
    }

    CppZero::Logger::printPrettyAST(tree, parser.getRuleNames());

    CppZero::SymbolTable symbolTable;
    CppZero::Reports<CppZero::Report> symbol_table_reports;
    CppZero::SymbolTableVisitor visitor(symbolTable, symbol_table_reports);

    visitor.visit(tree);
    symbolTable.PrintAll();

    std::string aggregatedLog;

    if (!symbol_table_reports.noErrors()) {
        aggregatedLog += symbol_table_reports.toString() + "\n";
    }

    CppZero::Reports<CppZero::Report> semantic_analysis_reports;
    CppZero::SemanticAnalysis semantic_analysis(symbolTable, semantic_analysis_reports);
    semantic_analysis.analyse(tree);

    if (!semantic_analysis_reports.noErrors()) {
        for (const CppZero::Report &error : semantic_analysis_reports.errors) {
            aggregatedLog += std::string(error.getMessage()) + "\n";
        }
    }

    std::cout << "\nStarting Optimization Pass...\n";
    CppZero::ASTOptimizer optimizer;
    std::any optimizationResult = optimizer.optimize(tree);

    if (optimizationResult.type() == typeid(int)) {
        std::cout << "Successfully folded expression value to: "
                  << std::any_cast<int>(optimizationResult) << "\n";
    }

    std::cout << "\nStarting Code Generation (Translating to x86-64 Assembly)...\n";
    CppZero::AssemblyGenerator generator(optimizationResult);
    std::string finalAssembly = generator.generateAssembly(tree);

    // Populate all interface fields safely
    window.setOptimizedAssemblyText(finalAssembly);
    
    if (!aggregatedLog.empty()) {
        window.setErrorLogText(aggregatedLog);
    }

    // window.show();
    window.showMaximized();
    delete tree;
    return app.exec();
}