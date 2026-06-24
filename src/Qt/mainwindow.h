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
    // ✅ Keep this signature exactly as it was originally
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // ✅ Add this separate method to receive the assembly text safely
    void setAssemblyText(const std::string &assemblyCode);

private slots:
    void onTextChanged();
    void buildAST(const std::string &code) const;

private:
    Ui::MainWindow *ui;

    void addNode(QTreeWidgetItem *parent,
                 const std::string &text);
};

#endif