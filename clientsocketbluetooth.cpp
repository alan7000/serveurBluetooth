#include "clientsocketbluetooth.h"

#include <QtCore/qmetaobject.h>

ClientSocketBluetooth::ClientSocketBluetooth(QObject *parent) : QObject (parent)
{

}

ClientSocketBluetooth::~ClientSocketBluetooth()
{
    stopClient();
}

void ClientSocketBluetooth::startClient(const QBluetoothServiceInfo &remoteService)
{
    if (socket) {
        return;
    }

    // Connect to service
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    qDebug() << "Create socket";
    socket->connectToService(remoteService);
    qDebug() << "ConnectToService done";

    connect(socket, &QBluetoothSocket::readyRead, this, &ClientSocketBluetooth::readSocket);
    connect(socket, &QBluetoothSocket::connected, this, QOverload<>::of(&ClientSocketBluetooth::connected));
    connect(socket, &QBluetoothSocket::disconnected, this, &ClientSocketBluetooth::disconnected);
    connect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error), this, &ClientSocketBluetooth::onSocketErrorOccurred);
}

void ClientSocketBluetooth::stopClient()
{
    delete socket;
    socket = nullptr;
}

void ClientSocketBluetooth::sendMessage(const QString &message)
{
    QByteArray text = message.toUtf8() + '\n';
    socket->write(text);
}

void ClientSocketBluetooth::readSocket()
{
    if (!socket) {
        return;
    }

    while (socket->canReadLine()) {
        QByteArray line = socket->readLine();
        emit messageReceived(socket->peerName(), QString::fromUtf8(line.constData(), line.length()));
    }
}

void ClientSocketBluetooth::connected()
{
    emit connected(socket->peerName());
}

void ClientSocketBluetooth::onSocketErrorOccurred(QBluetoothSocket::SocketError error)
{
    if (error == QBluetoothSocket::NoSocketError)
        return;

    QMetaEnum metaEnum = QMetaEnum::fromType<QBluetoothSocket::SocketError>();
    QString errorString = socket->peerName() + QLatin1Char(' ') + metaEnum.valueToKey(error) + QLatin1String(" occurred");

    emit socketErrorOccurred(errorString);
}
