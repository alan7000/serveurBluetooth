#ifndef REMOTESELECTORDIALOG_H
#define REMOTESELECTORDIALOG_H

#include <QDialog>

#include <QtBluetooth/qbluetoothaddress.h>
#include <QtBluetooth/qbluetoothserviceinfo.h>
#include <QtBluetooth/qbluetoothuuid.h>

QT_FORWARD_DECLARE_CLASS(QBluetoothServiceDiscoveryAgent)
QT_FORWARD_DECLARE_CLASS(QListWidgetItem)

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui {
class RemoteSelectorDialog;
}
QT_END_NAMESPACE

class RemoteSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteSelectorDialog(const QBluetoothAddress &localAdapter, QWidget *parent = nullptr);
    ~RemoteSelectorDialog();

    void startDiscovery(const QBluetoothUuid &uuid);
    void stopDiscovery();
    QBluetoothServiceInfo service() const;

private:
    Ui::RemoteSelectorDialog *ui;

    QBluetoothServiceDiscoveryAgent *m_discoveryAgent;
    QBluetoothServiceInfo m_service;
    QMap<QListWidgetItem *, QBluetoothServiceInfo> m_discoveredServices;

private slots:
    void serviceDiscovered(const QBluetoothServiceInfo &serviceInfo);
    void discoveryFinished();
    void on_remoteDevices_itemActivated(QListWidgetItem *item);
    void on_cancelButton_clicked();
};

#endif // REMOTESELECTORDIALOG_H
