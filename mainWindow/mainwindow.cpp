#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qscrollbar.h"

#include <QtWidgets>
#include "script/highlighter.h"

#include <sstream>

using namespace tw;

static Ui::MainWindow *main_ui;

void BrowserFormattedPrint(const Parameter &param, const QBrush &brush)
{
    std::stringstream ss;
    ss << param;

    QTextCharFormat format;
    format.setForeground(brush);

    const bool atBottom = main_ui->textBrowser->verticalScrollBar()->value() ==
        main_ui->textBrowser->verticalScrollBar()->maximum();
    QTextDocument* doc = main_ui->textBrowser->document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);
    cursor.beginEditBlock();
    if (!doc->isEmpty())
        cursor.insertBlock();
    cursor.setCharFormat(format);
    cursor.insertText(ss.str().c_str());
    cursor.endEditBlock();

    // Move scrollarea to bottom if it was at bottom when the output was printed
    // (not to force scrolling to bottom if user is looking at a higher position)
    if (atBottom) {
        QScrollBar* bar = main_ui->textBrowser->verticalScrollBar();
        bar->setValue(bar->maximum());
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    se(this)
{
    ui->setupUi(this);

    se.setOutput(BrowserFormattedPrint);
    main_ui = ui;

    highlighter = new SyntaxHighlighter(main_ui->textEdit->document());

    ui->textEdit->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete highlighter;
}

void MainWindow::run()
{
    se.run(ui->textEdit->toPlainText().toStdString());
}

void MainWindow::clearLog()
{
    ui->textBrowser->setText(nullptr);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier) {
        if (event->key() == Qt::Key_R) {
            run();
        } else if (event->key() == Qt::Key_L) {
            clearLog();
        } else if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9) {
            QString key;
            key.setNum(event->key() - Qt::Key_1 + 1);
            key = "script" + key;
            ui->textEdit->setText(settings.value(key).toString());
            QTextCursor cursor = ui->textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->textEdit->setTextCursor(cursor);
        }
    } else if (event->modifiers() == (Qt::ControlModifier | Qt::MetaModifier)) {
        if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9) {
            QString key;
            key.setNum(event->key() - Qt::Key_1 + 1);
            key = "script" + key;
            settings.setValue(key, ui->textEdit->toPlainText());
        }
    }
}

