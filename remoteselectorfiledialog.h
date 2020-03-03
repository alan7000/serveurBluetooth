#ifndef REMOTESELECTORFILEDIALOG_H
#define REMOTESELECTORFILEDIALOG_H

#include <QDialog>
#include <QPointer>

#include <qbluetoothuuid.h>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothlocaldevice.h>

QT_FORWARD_DECLARE_CLASS(QModelIndex)
QT_FORWARD_DECLARE_CLASS(QTableWidgetItem)
QT_FORWARD_DECLARE_CLASS(QFile)

class pinDisplay;

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE

namespace Ui {
class RemoteSelectorFileDialog;
}
QT_END_NAMESPACE

class RemoteSelectorFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteSelectorFileDialog(QWidget *parent = nullptr);
    ~RemoteSelectorFileDialog();

    void startDiscovery(const QBluetoothUuid &uuid);
    QBluetoothServiceInfo service() const;

    QString getTab() const;
    void setTab(const QString &value);

    bool getFlag() const;
    void setFlag(bool value);

public Q_SLOTS:
    void startDiscovery();

private slots:
    void serviceDiscovered(const QBluetoothServiceInfo &serviceInfo);
    void discoveryFinished();

    void on_pushButton_clicked();
    void on_StopButton_clicked();
    void on_ActualiserButton_clicked();

    void pairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing status);
    void pairingError(QBluetoothLocalDevice::Error error);
    void displayPin(const QBluetoothAddress &address, QString pin);
    void displayConfirmation(const QBluetoothAddress &address, QString pin);
    void displayConfReject();
    void displayConfAccepted();

    void on_tableWidget_cellClicked(int row, int column);

    void on_SelectDeviceButton_clicked();

    void on_tableWidget_itemChanged(QTableWidgetItem *item);

private:
    Ui::RemoteSelectorFileDialog *ui;

    QBluetoothServiceDiscoveryAgent *m_discoveryAgent;
    QBluetoothServiceInfo m_service;
    QMap<int, QBluetoothServiceInfo> m_discoveredServices;
    QFile *m_file;
    QBluetoothLocalDevice *m_localDevice;
    QPointer<pinDisplay> m_pindisplay;
    bool m_pairingError;

    QString addressToName(const QBluetoothAddress &address);

    QString tab;

    bool flag;
};

#endif // REMOTESELECTORFILEDIALOG_H
