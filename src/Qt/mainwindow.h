#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    void setOptimizedAssemblyText(const std::string &assemblyCode);
    void setPlainAssemblyText(const std::string &assemblyCode);
    void setSSAIntermediateText(const std::string &ssaCode);
protected:
    // Intercepts middle mouse clicks and scroll events on the canvas

    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onTextChanged();

private:
    void buildAST(const std::string &code) const;

    Ui::MainWindow *ui;
    QGraphicsScene *astScene;

    // Tracking points for panning state
    bool isPanning = false;
    QPoint panLastMousePos;
};

#endif // MAINWINDOW_H
