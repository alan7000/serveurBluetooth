#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}


void Dialog::on_pushButtonSocket_clicked()
{
    this->socketDialog.show();
}

void Dialog::on_pushButtonFile_clicked()
{
    this->fileDialog.show();
}
