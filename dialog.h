#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QFileDialog>

/****************************************
 * partie file transfert
 ****************************************/
#include "bluetoothe.h"

/***************************************
 * partie socket transfert
 **************************************/
#include <QtBluetooth/qbluetoothhostinfo.h>

QT_USE_NAMESPACE
class Socketbluetooth;

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT
//! [Déclaration file transfert]
public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private slots:
    void on_pushButton_selectDevice_clicked();

    void on_pushButton_SelectFile_clicked();

    void on_pushButton_sendFile_clicked();

private:
    Ui::Dialog *ui;
    Bluetoothe bluetoothe;

    QBluetoothServiceInfo m_service;
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    QMap<int, QBluetoothServiceInfo> m_discoveredServices;
    QFile *m_file;
    QBluetoothLocalDevice *m_localDevice;

//! [Déclaration file transfert]

//! [Déclaration socket transfert]

signals:
    void sendMessage(const QString &message);

private slots:
    void connectClicked();
    void sendClicked();

    void showMessage(const QString &sender, const QString &message);

    void clientConnected(const QString &name);
    void clientDisconnected(const QString &name);
    void clientDisconnected();
    void connected(const QString &name);
    void reactOnSocketError(const QString &error);

    void newAdapterSelected();

private:
    int adapterFromUserSelection() const;
    int currentAdapterIndex = 0;
//! [Déclaration socket transfert]
};
#endif // DIALOG_H
