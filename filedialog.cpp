#include "filedialog.h"
#include "ui_filedialog.h"
#include "remoteselectorfiledialog.h"

#include <QTextStream>
#include <QFile>
#include <QDataStream>

#include <QtCore/qdebug.h>
#include <qbluetoothtransferrequest.h>
#include <qbluetoothtransferreply.h>

FileDialog::FileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileDialog)
{
    ui->setupUi(this);
}

FileDialog::~FileDialog()
{
    delete ui;
}

QBluetoothServiceInfo FileDialog::service() const
{
    return m_service;
}

void FileDialog::on_pushButton_selectDevice_clicked()
{
    this->remoteselectorfiledialog.show();

    if (this->remoteselectorfiledialog.getFlag()) {
        ui->pushButton_sendFile->setEnabled(true);
        ui->pushButton_SelectFile->setEnabled(true);
    }

}

void FileDialog::on_pushButton_SelectFile_clicked()
{
    ui->lineEditTrame->setText(QFileDialog::getOpenFileName());
    if (m_service.isValid()) {
        ui->pushButton_SelectFile->setDisabled(false);
    }
}

void FileDialog::on_pushButton_sendFile_clicked()
{
    QBluetoothTransferManager mgr;
    QBluetoothTransferRequest req(this->remoteselectorfiledialog.service().device().address());
    qDebug() << "Récuperation de l'addresse";
    m_file = new QFile(ui->lineEditTrame->text());

    for (int var = 0; var < 100; ++var) {
        QBluetoothTransferReply *reply = mgr.put(req, m_file);
        reply->setParent(this);

        if (reply->error()) {
            qDebug() << "Failed to send file";
            reply->deleteLater();
            return;
        }


    }

}
