#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <string>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    void setOptimizedAssemblyText(const std::string &assemblyCode);

    void setPlainAssemblyText(const std::string &assemblyCode);

    void setSSAIntermediateText(const std::string &ssaCode);

    void setErrorLogText(const std::string &errorLog);

    void clearErrorLog();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onTextChanged();

    void onZoomInClicked();

    void onZoomOutClicked();

private:
    void buildAST(const std::string &code) const;

    Ui::MainWindow *ui;
    QGraphicsScene *astScene;

    bool isPanning = false;
    QPoint panLastMousePos;
};

#endif // MAINWINDOW_H
