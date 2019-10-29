#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qscrollbar.h"

#include <QtWidgets>
#include "script/syntaxhighlighter.h"

#include <sstream>

struct LogLine {
    QString style;
    QString text;
};

class LogLines
{
public:
    void newLine(QString color, QString text);
    void printLine(size_t index, QString &html) const;

    void clear() { lines.clear(); }
    size_t size() const { return lines.size(); }

private:
    std::vector<LogLine> lines;
};

static Ui::MainWindow *main_ui;
static LogLines *lines;

void LogLines::newLine(QString color, QString text)
{
    lines.push_back(LogLine());
    lines.rbegin()->style = "color: " + color + ";";
    lines.rbegin()->text = text;
}

void LogLines::printLine(size_t index, QString &html) const
{
    const LogLine &line = lines[index];
    html += "<span style=\"margin-top:0px;margin-bottom:0px;;" + line.style + "\">";
    if (line.text.length() > 0)
    {
        html += line.text;
    }
    else
    {
        html += "&nbsp;";
    }
    html += "</span>";
}

void __WindowPrint(const Parameter &param, const char *font_color)
{
    std::stringstream ss;
    ss << param;

    if (lines->size() > 0) {
        main_ui->textBrowser->insertHtml("<br>");
    }

    lines->newLine(font_color, ss.str().c_str());

    QString qstr;
    QString qstr_last;
    qstr = "<html><head></head><body>";

    size_t i;
    for (i = 0; i < lines->size(); ++i)
    {
        lines->printLine(i, qstr);
    }

    qstr += "</body></html>";

    lines->printLine(lines->size() - 1, qstr_last);

    main_ui->textBrowser->insertHtml(qstr_last);
}

void WindowPrint(const Parameter &param)
{
    __WindowPrint(param, "#111");
}

void WindowPrintError(const Parameter &param)
{
    __WindowPrint(param, "darkred");
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    se(this)
{
    ui->setupUi(this);
    ui->textEdit->setTabChangesFocus(true);

    se.setOutput(WindowPrint);
    se.setErrorOutput(WindowPrintError);
    main_ui = ui;

    lines = new LogLines();

    highlighter = new SyntaxHighlighter(main_ui->textEdit->document());

    ui->textEdit->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete lines;
    delete highlighter;
}

void MainWindow::run()
{
    se.run(ui->textEdit->toPlainText().toStdString());

    QScrollBar *sb = ui->textBrowser->verticalScrollBar();
    sb->setValue(INT_MAX);
}

void MainWindow::clearLog()
{
    ui->textBrowser->setText(nullptr);
    lines->clear();
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
                "print(str(\"Hallo\") + str(\" Welt!\"))\n"
                "\n"
                "# Das ist ein Kommentar\n"
                "# Damit du weisst, wie sie aussehen");
            QTextCursor cursor = ui->textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->textEdit->setTextCursor(cursor);
        }
        else if (event->key() == Qt::Key_3)
        {
            ui->textEdit->setText(
                "fullscreen = 0\n"
                "if fullscreen == 0:\n"
                "    rect=select()\n"
                "    print(rect)\n"
                "    image=capture(rect)\n"
                "else:\n"
                "    image=capture()\n"
                "display(image)");
            QTextCursor cursor = ui->textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->textEdit->setTextCursor(cursor);
        }
    }
}

