#include "socketbluetooth.h"

#include <QtBluetooth/QBluetoothServer>
#include <QtBluetooth/QBluetoothSocket>

//! [Service UUID]
static const QLatin1String serviceUuid("e8e10f95-1a70-4b27-9ccf-02010264e9c9"); //e8e10f95-1a70-4b27-9ccf-02010264e9c8
//! [Service UUID]

socketBluetooth::socketBluetooth(QObject *parent) : QObject (parent)
{

}

socketBluetooth::~socketBluetooth()
{
    stopServer();
}

void socketBluetooth::startServer(const QBluetoothAddress &localAdapter)
{
    if (rfcommServer) {
        return;
    }

    //! [Create the server]
    rfcommServer = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(rfcommServer, &QBluetoothServer::newConnection, this, QOverload<>::of(&socketBluetooth::clientConnected));
    bool result = rfcommServer->listen(localAdapter);
    if (!result) {
        qWarning() << "Cannot bind server to" << localAdapter.toString();
        return;
    }
    //! [Create server]

    QBluetoothServiceInfo::Sequence profileSequence;
    QBluetoothServiceInfo::Sequence classId;
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
    classId << QVariant::fromValue(quint16(0x100));
    profileSequence.append(QVariant::fromValue(classId));
    serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList, profileSequence);
    classId.clear();
    classId << QVariant::fromValue(QBluetoothUuid(serviceUuid));
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));

    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);

    //! [Service name, description and provider]
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, tr("Station Météo Server"));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription, tr("Station météo serveur bluetooth"));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, tr("ir.lml"));
    //! [Service name, description and provider]

    //! [Service UUID set]
    serviceInfo.setServiceUuid(QBluetoothUuid(serviceUuid));
    //! [Service UUID set]

    //! [Service Discoverability]
    QBluetoothServiceInfo::Sequence publicBrowse;
    publicBrowse << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));
    serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, publicBrowse);
    //! [Service Discoverability]

    //! [Protocol descriptor list]
    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    protocol.clear();
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm)) << QVariant::fromValue(quint8(rfcommServer->serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);
    //! [Protocol descriptor list]

    //! [Register service]
    serviceInfo.registerService(localAdapter);
    //! [Register service]
}

void socketBluetooth::stopServer()
{
    //Unregister service
    serviceInfo.unregisterService();

    //Close sockets
    qDeleteAll(clientSockets);

    //Close Server
    delete rfcommServer;
    rfcommServer = nullptr;
}

void socketBluetooth::sendMessage(const QString &message)
{
    QByteArray text = message.toUtf8() + '\n';

    for (QBluetoothSocket *socket : qAsConst(clientSockets)) {
        socket->write(text);
    }
}

void socketBluetooth::clientConnected()
{
    QBluetoothSocket *socket = rfcommServer->nextPendingConnection();
    if (!socket) {
        return;
    }

    connect(socket, &QBluetoothSocket::readyRead, this, &socketBluetooth::readSocket);
    connect(socket, &QBluetoothSocket::disconnected, this, QOverload<>::of(&socketBluetooth::clientDisconnected));
    clientSockets.append(socket);
    emit clientConnected(socket->peerName());
}

void socketBluetooth::clientDisconnected()
{
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket *>(sender());
    if (!socket) {
        return;
    }

    emit clientDisconnected(socket->peerName());

    clientSockets.removeOne(socket);

    socket->deleteLater();
}

void socketBluetooth::readSocket()
{
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket *>(sender());
    if (!socket) {
        return;
    }

    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();
        emit messageReceived(socket->peerName(), QString::fromUtf8(line.constData(), line.length()));
    }
}
