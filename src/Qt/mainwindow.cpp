#include "mainwindow.h"
#include "ui_mainwindow.h"

// 1. Undefine 'emit' so it doesn't break ANTLR's Lexer::emit method
#ifdef emit
#undef emit
#endif

// 2. Include your ANTLR headers safely
#include "antlr4-runtime.h"
#include "CppLexer.h"
#include "CppParser.h"

// 3. Redefine 'emit' back to nothing so Qt's emit keyword works in the rest of this file
#ifndef emit
#define emit
#endif

#include <QTreeWidgetItem>
#include <sstream>

using namespace antlr4;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->codeEditor,
            &QPlainTextEdit::textChanged,
            this,
            &MainWindow::onTextChanged);

    buildAST(ui->codeEditor->toPlainText().toStdString());
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onTextChanged()
{
    buildAST(ui->codeEditor->toPlainText().toStdString());
}


void MainWindow::buildAST(const std::string &code)
{
    ui->astTree->clear();

    antlr4::ANTLRInputStream input(code);
    CppLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    CppParser parser(&tokens);

    tree::ParseTree *tree = parser.translationUnit();

    // Lambda helper to recursively build the tree visually matching ANTLR's hierarchy
    std::function<QTreeWidgetItem*(tree::ParseTree*, QTreeWidgetItem*)> visit;
    visit = [&](tree::ParseTree *node, QTreeWidgetItem *parent) -> QTreeWidgetItem*
    {
        QTreeWidgetItem *item = parent ? new QTreeWidgetItem(parent)
                                       : new QTreeWidgetItem(ui->astTree);

        // Check if the node is a rule context (like translationUnit, expr, etc.)
        if (auto *ruleContext = dynamic_cast<antlr4::ParserRuleContext*>(node)) {
            // Get the actual rule name from the parser
            std::string ruleName = parser.getRuleNames()[ruleContext->getRuleIndex()];
            item->setText(0, QString::fromStdString(ruleName));
            item->setForeground(0, QBrush(Qt::darkBlue)); // Distinct styling for rules
        }
        // Or if it's a leaf node (a terminal token like INT, '+', etc.)
        else if (auto *terminalNode = dynamic_cast<tree::TerminalNode*>(node)) {
            std::string tokenText = terminalNode->getText();

            // Skip showing EOF explicitly unless you want it
            if (tokenText == "<EOF>") {
                delete item;
                return nullptr;
            }

            item->setText(0, QString::fromStdString('"' + tokenText + '"'));
            item->setForeground(0, QBrush(Qt::darkGreen)); // Distinct styling for tokens
        }

        // Traverse child nodes
        for (size_t i = 0; i < node->children.size(); i++) {
            visit(node->children[i], item);
        }

        return item;
    };

    visit(tree, nullptr);
    ui->astTree->expandAll();
}