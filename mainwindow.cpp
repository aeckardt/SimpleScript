#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "script/script.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonBox_clicked(QAbstractButton *button)
{
    QPushButton *pbtn = reinterpret_cast<QPushButton *>(button);
    if (pbtn == ui->buttonBox->button(QDialogButtonBox::Ok)) {
        ;
    } else if (pbtn == ui->buttonBox->button(QDialogButtonBox::Cancel)) {
        ;
    }
}
