#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qscrollbar.h"

#include <QtWidgets>
#include "script/highlighter.h"

#include <sstream>

using namespace std;
using namespace tw;

static Ui::MainWindow *main_ui;

void BrowserFormattedPrint(const Parameter &param, const QBrush &brush)
{
    stringstream ss;
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

    //scroll scrollarea to bottom if it was at bottom when we started
    //(we don't want to force scrolling to bottom if user is looking at a
    //higher position)
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
    if (event->modifiers() == Qt::ControlModifier)
    {
        if (event->key() == Qt::Key_R)
        {
            run();
        }
        else if (event->key() == Qt::Key_L)
        {
            clearLog();
        }
        else if (event->key() == Qt::Key_1)
        {
            ui->textEdit->setText(
                "print(str(\"Hello \") + str(\"world!\"))\n"
                "\n"
                "# This is a comment\n"
                "\t  # You stupid banana!");
            QTextCursor cursor = ui->textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->textEdit->setTextCursor(cursor);
        }
        else if (event->key() == Qt::Key_2)
        {
            ui->textEdit->setText(
                "# Expression calculation\n"
                "\n"
                "value = 1+(2+3+9-10/2)*5-31\n"
                "print(\"The sum of numbers from one to five is: \" + str(value))");
            QTextCursor cursor = ui->textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->textEdit->setTextCursor(cursor);
        }
        else if (event->key() == Qt::Key_3)
        {
            ui->textEdit->setText(
                "# Screenshot time measurement\n"
                "\n"
                "start=now()\n"
                "image=capture()\n"
                "print(\"It took \" + str(msecsbetween(start, now())) + \"ms to take a screenshot.\")");
            QTextCursor cursor = ui->textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->textEdit->setTextCursor(cursor);
        }
        else if (event->key() == Qt::Key_4)
        {
            ui->textEdit->setText(
                "# View screenshot of selected region\n"
                "\n"
                "fullscreen = 0\n"
                "if fullscreen == 0:\n"
                "\trect=select()\n"
                "\tprint(rect)\n"
                "\timage=capture(rect)\n"
                "else:\n"
                "\timage=capture()\n"
                "view(image)");
            QTextCursor cursor = ui->textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->textEdit->setTextCursor(cursor);
        }
        else if (event->key() == Qt::Key_5)
        {
            ui->textEdit->setText(
                "# Record selected region\n"
                "\n"
                "rect=select()\n"
                "video=record(rect, 1)\n"
                "showvid(video)");
            QTextCursor cursor = ui->textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->textEdit->setTextCursor(cursor);
        }
    }
}

