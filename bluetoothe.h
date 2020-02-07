#ifndef BLUETOOTHE_H
#define BLUETOOTHE_H

#include <QDialog>
#include <QPointer>

#include <qbluetoothuuid.h>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothlocaldevice.h>

#include <ui_dialog.h>

QT_FORWARD_DECLARE_CLASS(QModelIndex)
QT_FORWARD_DECLARE_CLASS(QTableWidgetItem)
QT_FORWARD_DECLARE_CLASS(QFile)



QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE

namespace Ui {
class Bluetoothe;
}

class Bluetoothe : public QDialog
{
    Q_OBJECT

public:
    explicit Bluetoothe(QWidget *parent = 0);
    ~Bluetoothe();

    void startDiscovery(const QBluetoothUuid &uuid);
    QBluetoothServiceInfo service() const;
    QBluetoothServiceInfo m_service;

    Ui_Dialog *dialog;

    QString getTab() const;
    void setTab(const QString &value);

    bool getFlag() const;
    void setFlag(bool value);

public Q_SLOTS:
    void startDiscovery();

private slots:
    void on_pushButton_clicked();

    void on_StopButton_clicked();

    void on_ActualiserButton_clicked();


    void serviceDiscovered(const QBluetoothServiceInfo &serviceInfo);
    void discoveryFinished();


    void pairingFinished(const QBluetoothAddress &address,QBluetoothLocalDevice::Pairing pairing);
    void pairingError(QBluetoothLocalDevice::Error error);
    void displayConfReject();
    void displayConfAccepted();

    void on_tableWidget_itemActivated(QTableWidgetItem *item);

    void on_tableWidget_cellClicked(int row, int column);

    void on_SelectDeviceButton_clicked();

private:
    Ui::Bluetoothe *ui;

    QBluetoothServiceDiscoveryAgent *m_discoveryAgent;

    QMap<int, QBluetoothServiceInfo> m_discoveredServices;
    QFile *m_file;
    QBluetoothLocalDevice *m_localDevice;
    bool m_pairingError;

    QString addressToName(const QBluetoothAddress &address);

    QString tab;

    bool flag;
};

#endif // BLUETOOTHE_H
