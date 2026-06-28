#include "mainwindow.h"
#include "ui_mainwindow.h"

#ifdef emit
#undef emit
#endif

#include "antlr4-runtime.h"
#include "CppLexer.h"
#include "CppParser.h"

#ifndef emit
#define emit
#endif

#include <QFile>
#include <QTextStream>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QSplitter>  // Required for splitter handling
#include <QPen>
#include <QBrush>
#include <QFont>
#include <cmath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>

import ASTOptimizer;
import AssemblyGenerator;

using namespace antlr4;

// (ASTNodeItem implementation remains exactly the same as your previous codebase)
class ASTNodeItem : public QGraphicsEllipseItem {
public:
    QGraphicsLineItem* parentLine = nullptr;
    std::vector<QGraphicsLineItem*> childLines;

    ASTNodeItem(double x, double y, double r, QGraphicsItem* parent = nullptr)
        : QGraphicsEllipseItem(x - r, y - r, r * 2, r * 2, parent)
    {
        setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
    }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
        if (change == ItemPositionChange && scene()) {
            QPointF newPos = value.toPointF();
            QPointF offset = newPos - pos();

            if (parentLine) {
                QLineF line = parentLine->line();
                line.setP2(line.p2() + offset);
                parentLine->setLine(line);
            }

            for (QGraphicsLineItem* childLine : childLines) {
                QLineF line = childLine->line();
                line.setP1(line.p1() + offset);
                childLine->setLine(line);
            }
        }
        return QGraphicsEllipseItem::itemChange(change, value);
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Swap out the mock tree view inside the horizontal splitter wrapper
    QWidget *oldTree = ui->astTree;
    QSplitter *horizontalSplitter = ui->centralwidget->findChild<QSplitter*>("columnsHorizontalSplitter");

    QGraphicsView *graphicsView = new QGraphicsView(this);
    if (horizontalSplitter) {
        // Find where the old tree widget was positioned inside the splitter array
        int index = horizontalSplitter->indexOf(oldTree);
        oldTree->deleteLater();
        // Insert our clean graphics view at the exact same lane location
        horizontalSplitter->insertWidget(index, graphicsView);
    } else {
        oldTree->deleteLater();
    }

    ui->astTree = reinterpret_cast<QTreeWidget*>(graphicsView);

    // Configure proportional startup layout distributions
    QSplitter *verticalSplitter = ui->centralwidget->findChild<QSplitter*>("mainVerticalSplitter");
    if (verticalSplitter) {
        // Allots 80% room height to workspace columns, 20% to bottom console
        verticalSplitter->setSizes(QList<int>({600, 150}));
    }
    if (horizontalSplitter) {
        // Uniform initial column widths across all 5 modules
        horizontalSplitter->setSizes(QList<int>({240, 240, 240, 240, 240, 240}));
    }

    astScene = new QGraphicsScene(this);
    graphicsView->setScene(astScene);
    graphicsView->setRenderHint(QPainter::Antialiasing);

    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    graphicsView->viewport()->installEventFilter(this);

    QFile file("/home/incidence/Desktop/CompilerCpp/src/C++00/input.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        ui->codeEditor->setPlainText(in.readAll());
        file.close();
    }

    connect(ui->codeEditor, &QPlainTextEdit::textChanged, this, &MainWindow::onTextChanged);
    onTextChanged();
}

MainWindow::~MainWindow() {
    delete ui;
}

// (The remaining sections: onTextChanged, buildAST, and eventFilter stay completely identical to your layout)
void MainWindow::onTextChanged()
{
    std::string code = ui->codeEditor->toPlainText().toStdString();

    if (code.empty()) {
        astScene->clear();
        ui->optimizedAssemblyViewer->clear();
        ui->plainAssemblyViewer->clear();
        ui->errorConsole->clear();
        return;
    }

    antlr4::ANTLRInputStream input(code);
    CppLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    CppParser parser(&tokens);

    tree::ParseTree *tree = parser.translationUnit();

    if (parser.getNumberOfSyntaxErrors() > 0) {
        ui->optimizedAssemblyViewer->setPlainText("# Syntax Error: Fix input code to generate assembly...");
        ui->plainAssemblyViewer->setPlainText("# Syntax Error");

        std::string errorMsg = "[Syntax Error] Parse aborted. Found " +
                               std::to_string(parser.getNumberOfSyntaxErrors()) +
                               " structural syntax anomaly tokens.";
        ui->errorConsole->setPlainText(QString::fromStdString(errorMsg));

        buildAST(code);
        return;
    }

    ui->errorConsole->clear();

    CppZero::AssemblyGenerator rawGenerator;
    std::string unoptimizedAssembly = rawGenerator.generateAssembly(tree);

    CppZero::ASTOptimizer optimizer;
    std::any optimizationResult = optimizer.optimize(tree);

    CppZero::AssemblyGenerator optimizedGenerator(optimizationResult);
    std::string optimizedAssembly = optimizedGenerator.generateAssembly(tree);

    ui->plainAssemblyViewer->setPlainText(QString::fromStdString(unoptimizedAssembly));
    ui->optimizedAssemblyViewer->setPlainText(QString::fromStdString(optimizedAssembly));

    buildAST(code);
}

void MainWindow::setOptimizedAssemblyText(const std::string &assemblyCode) {
    ui->optimizedAssemblyViewer->setPlainText(QString::fromStdString(assemblyCode));
}

void MainWindow::setPlainAssemblyText(const std::string &assemblyCode) {
    ui->plainAssemblyViewer->setPlainText(QString::fromStdString(assemblyCode));
}

void MainWindow::setSSAIntermediateText(const std::string &ssaCode) {
    ui->ssaViewer->setPlainText(QString::fromStdString(ssaCode));
}

void MainWindow::setErrorLogText(const std::string &errorLog) {
    ui->errorConsole->setPlainText(QString::fromStdString(errorLog));
}

void MainWindow::clearErrorLog() {
    ui->errorConsole->clear();
}

void MainWindow::buildAST(const std::string &code) const
{
    astScene->clear();
    if (code.empty()) return;

    antlr4::ANTLRInputStream input(code);
    CppLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    CppParser parser(&tokens);

    tree::ParseTree *tree = parser.translationUnit();

    const double nodeRadius = 12.0;
    const double levelDistance = 60.0;
    const double siblingSpacing = 15.0;

    auto isValidNode = [&](tree::ParseTree *node) -> bool {
        if (!node) return false;
        if (auto *ruleContext = dynamic_cast<antlr4::ParserRuleContext*>(node)) {
            std::string ruleName = parser.getRuleNames()[ruleContext->getRuleIndex()];
            if (ruleName == "declarationModifiers" && node->children.empty()) return false;
            return true;
        }
        if (auto *terminalNode = dynamic_cast<tree::TerminalNode*>(node)) {
            std::string tokenText = terminalNode->getText();
            return (tokenText.find_first_not_of(" \t\r\n") != std::string::npos);
        }
        return false;
    };

    std::function<double(tree::ParseTree*)> getSubtreeWidth;
    std::unordered_map<tree::ParseTree*, double> cachedWidths;

    getSubtreeWidth = [&](tree::ParseTree *node) -> double {
        if (!isValidNode(node)) return 0.0;
        double childrenWidth = 0.0;
        size_t visibleChildren = 0;
        for (auto *child : node->children) {
            if (isValidNode(child)) {
                childrenWidth += getSubtreeWidth(child);
                visibleChildren++;
            }
        }
        if (visibleChildren > 1) childrenWidth += (visibleChildren - 1) * siblingSpacing;
        double nodeWidth = std::max(nodeRadius * 2.5, childrenWidth);
        cachedWidths[node] = nodeWidth;
        return nodeWidth;
    };
    getSubtreeWidth(tree);

    std::function<ASTNodeItem*(tree::ParseTree*, double, double, ASTNodeItem*, double, double)> drawTree;
    drawTree = [&](tree::ParseTree *node, double x, double y, ASTNodeItem *parentItem, double pX, double pY) -> ASTNodeItem*
    {
        if (!isValidNode(node)) return nullptr;

        ASTNodeItem *circle = new ASTNodeItem(x, y, nodeRadius);
        astScene->addItem(circle);

        QString nodeText;
        QColor nodeBgColor = Qt::black;
        QColor nodeTextColor = Qt::white;

        if (auto *ruleContext = dynamic_cast<antlr4::ParserRuleContext*>(node)) {
            nodeText = QString::fromStdString(parser.getRuleNames()[ruleContext->getRuleIndex()]);
            nodeBgColor = QColor(230, 242, 255);
            nodeTextColor = QColor(39, 111, 143);
        }
        else if (auto *terminalNode = dynamic_cast<tree::TerminalNode*>(node)) {
            nodeText = QString::fromStdString(terminalNode->getText());
            if (nodeText == "<EOF>") {
                nodeBgColor = QColor(255, 230, 230);
                nodeTextColor = QColor(153, 0, 0);
            } else {
                nodeBgColor = QColor(230, 255, 230);
                nodeTextColor = QColor(0, 102, 51);
            }
        }

        circle->setBrush(QBrush(nodeBgColor));
        circle->setPen(QPen(nodeTextColor, 1.5));

        if (parentItem) {
            QGraphicsLineItem *line = astScene->addLine(pX, pY + nodeRadius, x, y - nodeRadius);
            line->setPen(QPen(QColor(170, 170, 170), 1.5));
            line->setZValue(-1);

            circle->parentLine = line;
            parentItem->childLines.push_back(line);
        }

        QGraphicsTextItem *textItem = new QGraphicsTextItem(nodeText, circle);
        QFont font = textItem->font();
        font.setPointSize(7);
        font.setBold(true);
        textItem->setFont(font);
        textItem->setDefaultTextColor(nodeTextColor);

        double tW = textItem->boundingRect().width();
        double tH = textItem->boundingRect().height();
        textItem->setPos(x - tW / 2.0, y - tH / 2.0);

        double currentLeftX = x - cachedWidths[node] / 2.0;
        for (auto *child : node->children) {
            if (isValidNode(child)) {
                double childWidth = cachedWidths[child];
                double childTargetX = currentLeftX + childWidth / 2.0;
                drawTree(child, childTargetX, y + levelDistance, circle, x, y);
                currentLeftX += childWidth + siblingSpacing;
            }
        }
        return circle;
    };

    if (cachedWidths[tree] > 0) {
        drawTree(tree, 0.0, nodeRadius + 10.0, nullptr, 0.0, -1.0);
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    QGraphicsView *view = reinterpret_cast<QGraphicsView*>(ui->astTree);

    if (view && watched == view->viewport()) {
        switch (event->type()) {
            case QEvent::Wheel: {
                QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
                double scaleFactor = 1.15;
                if (wheelEvent->angleDelta().y() > 0) {
                    view->scale(scaleFactor, scaleFactor);
                } else {
                    view->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
                }
                wheelEvent->accept();
                return true;
            }
            case QEvent::MouseButtonPress: {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::MiddleButton) {
                    isPanning = true;
                    panLastMousePos = mouseEvent->pos();
                    view->setCursor(Qt::ClosedHandCursor);
                    mouseEvent->accept();
                    return true;
                }
                break;
            }
            case QEvent::MouseMove: {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                if (isPanning) {
                    QPoint delta = mouseEvent->pos() - panLastMousePos;
                    panLastMousePos = mouseEvent->pos();
                    view->horizontalScrollBar()->setValue(view->horizontalScrollBar()->value() - delta.x());
                    view->verticalScrollBar()->setValue(view->verticalScrollBar()->value() - delta.y());
                    mouseEvent->accept();
                    return true;
                }
                break;
            }
            case QEvent::MouseButtonRelease: {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::MiddleButton) {
                    isPanning = false;
                    view->unsetCursor();
                    mouseEvent->accept();
                    return true;
                }
                break;
            }
            default:
                break;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}