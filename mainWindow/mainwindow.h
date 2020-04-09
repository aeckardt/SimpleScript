#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

#include "script/engine.h"

namespace Ui {
class MainWindow;
}

class SyntaxHighlighter;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void keyPressEvent(QKeyEvent *) override;

private slots:
    void run();
    void clearLog();

private:
    Ui::MainWindow *ui;
    SyntaxHighlighter *highlighter;

    ScriptEngine se;

    QSettings settings;
};

#endif // MAINWINDOW_H
