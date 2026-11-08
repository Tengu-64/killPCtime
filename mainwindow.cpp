#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include "timeview.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Task srcTime = searchProc();
    ui->msgLabel->setText(srcTime.msg);
    ui->deleteProcess->setVisible(srcTime.find);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked() // установка времени
{
    uint8_t hrs = ui->spinBoxHours->value();
    uint8_t mins = ui->spinBoxMins->value();

    Task msg = timeViewMsg(hrs, mins);
    ui->msgLabel->setText(msg.msg);
    ui->deleteProcess->setVisible(msg.find);

}


void MainWindow::on_deleteProcess_clicked() // удаление времени
{
    Task del = killProcess();
    ui->deleteProcess->setVisible(!del.find); // убрать кнопку "удалить" если процесс удален
    ui->msgLabel->setText(del.msg);
}

