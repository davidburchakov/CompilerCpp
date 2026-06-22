#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onTextChanged();
    void buildAST(const std::string &code);

private:
    Ui::MainWindow *ui;

    void addNode(QTreeWidgetItem *parent,
                 const std::string &text);
};

#endif