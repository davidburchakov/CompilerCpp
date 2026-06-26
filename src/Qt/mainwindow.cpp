#include "mainwindow.h"
#include "ui_mainwindow.h"

// 1. Undefine 'emit' so it doesn't break ANTLR's Lexer::emit method
#ifdef emit
#undef emit
#endif

#include "antlr4-runtime.h"
#include "CppLexer.h"
#include "CppParser.h"

// 2. Redefine 'emit' back to nothing so Qt's framework continues working smoothly
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
#include <QLayout> // Added for dynamic widget swapping
#include <QPen>
#include <QBrush>
#include <QFont>
#include <cmath>
#include <QMouseEvent> // Added for click event tracking
#include <QWheelEvent> // Added for scroll event tracking
#include <QScrollBar>  // Added to shift scene positions manually

// ============================================================
// 3. BRING IN COMPILER MODULES FOR LIVE RE-COMPILATION
// ============================================================
import ASTOptimizer;
import AssemblyGenerator;

using namespace antlr4;

// ============================================================================
// 4. CUSTOM WRAPPER TO MAKE NODES INTERACTIVELY MOVABLE
// ============================================================================
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

// ============================================================
// 5. MAINWINDOW CLASS IMPLEMENTATIONS
// ============================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // DYNAMIC WIDGET SWAP
    QWidget *oldTree = ui->astTree;
    QLayout *parentLayout = oldTree->parentWidget() ? oldTree->parentWidget()->layout() : nullptr;

    QGraphicsView *graphicsView = new QGraphicsView(this);
    if (parentLayout) {
        parentLayout->replaceWidget(oldTree, graphicsView);
    }
    oldTree->deleteLater();

    ui->astTree = reinterpret_cast<QTreeWidget*>(graphicsView);

    // CANVAS ATTRIBUTES & CONTROLS SETUP
    astScene = new QGraphicsScene(this);
    graphicsView->setScene(astScene);
    graphicsView->setRenderHint(QPainter::Antialiasing);

    // Disable traditional scrollbars to keep a clean diagram workspace
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Enable scroll-hand drag modifications safely
    graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // Install the event filter directly onto the interactive viewport
    graphicsView->viewport()->installEventFilter(this);

    // Load file stream
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

void MainWindow::onTextChanged()
{
    std::string code = ui->codeEditor->toPlainText().toStdString();

    if (code.empty()) {
        astScene->clear();
        ui->optimizedAssemblyViewer->clear();
        ui->plainAssemblyViewer->clear();
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
        buildAST(code);
        return;
    }

    // ============================
    // 1. UNOPTIMIZED ASSEMBLY
    // ============================
    CppZero::AssemblyGenerator rawGenerator;
    std::string unoptimizedAssembly = rawGenerator.generateAssembly(tree);

    // ============================
    // 2. OPTIMIZER + OPTIMIZED ASM
    // ============================
    CppZero::ASTOptimizer optimizer;
    std::any optimizationResult = optimizer.optimize(tree);

    CppZero::AssemblyGenerator optimizedGenerator(optimizationResult);
    std::string optimizedAssembly = optimizedGenerator.generateAssembly(tree);

    // ============================
    // 3. UI OUTPUT
    // ============================
    ui->plainAssemblyViewer->setPlainText(
        QString::fromStdString(unoptimizedAssembly)
    );

    ui->optimizedAssemblyViewer->setPlainText(
        QString::fromStdString(optimizedAssembly)
    );

    // ============================
    // 4. AST VISUALIZATION
    // ============================
    buildAST(code);
}

void MainWindow::setOptimizedAssemblyText(const std::string &assemblyCode) {
    ui->optimizedAssemblyViewer->setPlainText(QString::fromStdString(assemblyCode));
}

void MainWindow::setPlainAssemblyText(const std::string &assemblyCode)
{
    ui->plainAssemblyViewer->setPlainText(
        QString::fromStdString(assemblyCode)
    );
}

void MainWindow::setSSAIntermediateText(const std::string &ssaCode)
{
    ui->ssaViewer->setPlainText(
        QString::fromStdString(ssaCode)
    );
}

// ============================================================================
// 6. BUILD AST IMPLEMENTATION
// ============================================================================
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
            nodeTextColor = QColor(39, 111, 143);//QColor(1, 51, 153);
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

// ============================================================================
// CANVAS PANNING AND ZOOMING CONTROLS (MIDDLE MOUSE CLICK & WHEEL)
// ============================================================================
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // Access our runtime view via the safety cast pointer
    QGraphicsView *view = reinterpret_cast<QGraphicsView*>(ui->astTree);

    if (view && watched == view->viewport()) {
        switch (event->type()) {

            // 1. ZOOM IN & ZOOM OUT VIA MOUSE WHEEL SCROLL
            case QEvent::Wheel: {
                QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
                double scaleFactor = 1.15;

                if (wheelEvent->angleDelta().y() > 0) {
                    // Zoom In
                    view->scale(scaleFactor, scaleFactor);
                } else {
                    // Zoom Out
                    view->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
                }
                wheelEvent->accept();
                return true;
            }

            // 2. DETECT MIDDLE CLICK PRESS TO INITIATE PANNING
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

            // 3. SHIFT CANVAS COORDINATES DYNAMICALLY DURING MIDDLE-DRAG
            case QEvent::MouseMove: {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                if (isPanning) {
                    QPoint delta = mouseEvent->pos() - panLastMousePos;
                    panLastMousePos = mouseEvent->pos();

                    // Shift view scroll bars relative to frame movement scale
                    view->horizontalScrollBar()->setValue(view->horizontalScrollBar()->value() - delta.x());
                    view->verticalScrollBar()->setValue(view->verticalScrollBar()->value() - delta.y());

                    mouseEvent->accept();
                    return true;
                }
                break;
            }

            // 4. RELEASE PANNING STATE ON MIDDLE MOUSE BUTTON RELEASE
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
