#ifndef SOCKETBLUETOOTH_H
#define SOCKETBLUETOOTH_H

#include <QtCore/qobject.h>

#include <QtBluetooth/qbluetoothaddress.h>
#include <QtBluetooth/qbluetoothserviceinfo.h>
#include "bluetoothe.h"

QT_FORWARD_DECLARE_CLASS(QBluetoothServer)
QT_FORWARD_DECLARE_CLASS(QBluetoothSocket)

class socketBluetooth : public QObject
{
    Q_OBJECT

public:
    socketBluetooth();
    ~socketBluetooth();

    void startServer(const QBluetoothAddress &localAdapter = QBluetoothAddress());
    void stopServer();

public slots:
    void sendMessage(const QString &message);

signals:
    void messageReceived(const QString &sender, const QString &message);
    void clientConnected(const QString &name);
    void clientDisconnected(const QString &name);

private slots:
    void clientConnected();
    void clientDisconnected();
    void readSocket();


private:
    QBluetoothServer *rfcommServer = nullptr;
    QBluetoothServiceInfo serviceInfo;
    QList<QBluetoothSocket *> clientSockets;
    Bluetoothe *bluetooth;
};

#endif // SOCKETBLUETOOTH_H
