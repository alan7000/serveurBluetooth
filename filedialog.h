#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QDialog>
#include <QFileDialog>

#include "bluetoothe.h"

namespace Ui {
class FileDialog;
}

class FileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileDialog(QWidget *parent = nullptr);
    ~FileDialog();

private:
    Ui::FileDialog *ui;

private slots:
    void on_pushButton_selectDevice_clicked();

    void on_pushButton_SelectFile_clicked();

    void on_pushButton_sendFile_clicked();

private:
    Bluetoothe bluetoothe;

    QBluetoothServiceInfo m_service;
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    QMap<int, QBluetoothServiceInfo> m_discoveredServices;
    QFile *m_file;
    QBluetoothLocalDevice *m_localDevice;
};

#endif // FILEDIALOG_H
