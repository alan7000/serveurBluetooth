#include "dialog.h"
#include "ui_dialog.h"
#include "time.h"
#include "bluetoothe.h"
#include "stdio.h"
#include "socketbluetooth.h"

#include <QTextStream>
#include <QFile>
#include <QDataStream>

#include <QtCore/qdebug.h>

#include <qbluetoothtransferrequest.h>
#include <qbluetoothtransferreply.h>

#include <QtBluetooth/qbluetoothdeviceinfo.h>
#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/qbluetoothuuid.h>

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

void Dialog::on_pushButton_selectDevice_clicked()
{
    this->bluetoothe.show();
    if (this->bluetoothe.getFlag()) {
            ui->pushButton_sendFile->setEnabled(true);
            ui->pushButtonTrame->setEnabled(true);
            ui->pushButton_SelectFile->setEnabled(true);
    }

}

void Dialog::on_pushButton_SelectFile_clicked()
{
    ui->lineEditTrame->setText(QFileDialog::getOpenFileName());
    if (m_service.isValid()) {
        ui->pushButton_SelectFile->setDisabled(false);
    }
}

void Dialog::on_pushButton_sendFile_clicked()
{
    QBluetoothTransferManager mgr;
    QBluetoothTransferRequest req(this->bluetoothe.m_service.device().address());
    qDebug() << "Récuperation de l'addresse";
    m_file = new QFile(ui->lineEditTrame->text());

    QBluetoothTransferReply *reply = mgr.put(req, m_file);
    reply->setParent(this);
    if (reply->error()) {
        qDebug() << "Failed to send file";
        reply->deleteLater();
        return;
    }
}
