#include "socketdialog.h"
#include "ui_socketdialog.h"
#include "socketbluetooth.h"
#include "clientsocketbluetooth.h"
#include "remoteselectordialog.h"

#include <QtCore/qdebug.h>

#include <QtBluetooth/qbluetoothdeviceinfo.h>
#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/qbluetoothuuid.h>

/***************
 * condition en cas de compilation pour android, faire l'include suivant
 */
#ifdef Q_OS_ANDROID
#include <QtAndroidExtras/QtAndroid>
#endif

static const QLatin1String serviceUuid("e8e10f95-1a70-4b27-9ccf-02010264e9c8");
#ifdef Q_OS_ANDROID
static const QLatin1String reverseUuid("c8e96402-0102-cf9c-274b-701a950fe1e8"); //UUid utiliser pour android
#endif

SocketDialog::SocketDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SocketDialog)
{
    ui->setupUi(this);
    //! [Connect pour socket]
    connect(ui->pushButton_Quit, &QPushButton::clicked, this, &SocketDialog::accept);
    connect(ui->pushButton_connect_device_socket, &QPushButton::clicked, this, &SocketDialog::connectClicked);
    connect(ui->pushButtonTrame, &QPushButton::clicked, this, &SocketDialog::sendClicked);
    //! [Connect pour socket]

    //! [Déclaration socket]
    localAdapters = QBluetoothLocalDevice::allDevices();
    if (localAdapters.count() < 2) {
        ui->localAdapters->setVisible(false);
    } else {
        //On oublie lorsqu'il y a plus de deux adaptateurs bluetooth
        ui->localAdapters->setVisible(true);
        ui->firstAdapter->setText(tr("Default (%1)", "%1 = Bluetooth address").arg(localAdapters.at(0).address().toString()));
        ui->secondAdapter->setText(localAdapters.at(1).address().toString());
        ui->firstAdapter->setChecked(true);
        connect(ui->firstAdapter, &QRadioButton::clicked, this, &SocketDialog::newAdapterSelected);
        connect(ui->secondAdapter, &QRadioButton::clicked, this, &SocketDialog::newAdapterSelected);
        QBluetoothLocalDevice adapter(localAdapters.at(1).address());
        adapter.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
    }


    //! [Déclaration socket]

    // Create socket server
    server = new socketBluetooth(this);
    connect(server, QOverload<const QString &>::of(&socketBluetooth::clientConnected), this, &SocketDialog::clientConnected);
    connect(server, QOverload<const QString &>::of(&socketBluetooth::clientDisconnected), this, QOverload<const QString &>::of(&SocketDialog::clientDisconnected));
    connect(server, &socketBluetooth::messageReceived, this, &SocketDialog::showMessage);
    connect(this, &SocketDialog::sendMessage, server, &socketBluetooth::sendMessage);
    server->startServer();

    // Create socket server

    // Get local device name
    localName = QBluetoothLocalDevice().name();
    // Get local device name
}

SocketDialog::~SocketDialog()
{
    qDeleteAll(clients);
    delete ui;
}

void SocketDialog::connectClicked()
{
    ui->pushButton_connect_device_socket->setEnabled(false);

    // scan for services
    const QBluetoothAddress adapter = localAdapters.isEmpty() ? QBluetoothAddress() : localAdapters.at(currentAdapterIndex).address();

    RemoteSelectorDialog remoteSelector(adapter);
#ifdef Q_OS_ANDROID
    if (QtAndroid::androidSdkVersion() >= 23)
        remoteSelector.startDiscovery(QBluetoothUuid(reverseUuid));
    else
        remoteSelector.startDiscovery(QBluetoothUuid(serviceUuid));
#else
    remoteSelector.startDiscovery(QBluetoothUuid(serviceUuid));
#endif
    if (remoteSelector.exec() == QDialog::Accepted) {
        QBluetoothServiceInfo service = remoteSelector.service();

        qDebug() << "Connecting to service 2" << service.serviceName()
                 << "on" << service.device().name();

        // Create client
        qDebug() << "Going to create client";
        ClientSocketBluetooth *client = new ClientSocketBluetooth(this);
qDebug() << "Connecting...";

        connect(client, &ClientSocketBluetooth::messageReceived, this, &SocketDialog::showMessage);
        connect(client, &ClientSocketBluetooth::disconnected, this, QOverload<>::of(&SocketDialog::clientDisconnected));
        connect(client, QOverload<const QString &>::of(&ClientSocketBluetooth::connected), this, &SocketDialog::connected);
        connect(client, &ClientSocketBluetooth::socketErrorOccurred, this, &SocketDialog::reactOnSocketError);
        connect(this, &SocketDialog::sendMessage, client, &ClientSocketBluetooth::sendMessage);
qDebug() << "Start client";
        client->startClient(service);

        clients.append(client);
    }

    ui->pushButton_connect_device_socket->setEnabled(true);
}

void SocketDialog::sendClicked()
{
    ui->pushButtonTrame->setEnabled(false);
    ui->lineEditTrame->setEnabled(false);

    showMessage(localName, ui->lineEditTrame->text());
    emit sendMessage(ui->lineEditTrame->text());

    ui->lineEditTrame->clear();

    ui->lineEditTrame->setEnabled(true);
    ui->pushButtonTrame->setEnabled(true);
}

void SocketDialog::showMessage(const QString &sender, const QString &message)
{
    ui->textEditRponse->insertPlainText(QString::fromUtf8("%1: %2\n").arg(sender, message));
    ui->textEditRponse->ensureCursorVisible();
}

void SocketDialog::clientConnected(const QString &name)
{
    ui->textEditRponse->insertPlainText(QString::fromUtf8("%1 est connecter au socket.\n").arg(name));
}

void SocketDialog::clientDisconnected(const QString &name)
{
    ui->textEditRponse->insertPlainText(QString::fromUtf8("%1 est déconnecter.\n").arg(name));
}

void SocketDialog::clientDisconnected()
{
    ClientSocketBluetooth *client = qobject_cast<ClientSocketBluetooth *>(sender());
    if (client) {
        clients.removeOne(client);
        client->deleteLater();
    }
}

void SocketDialog::connected(const QString &name)
{
    ui->textEditRponse->insertPlainText(QString::fromUtf8("L'appareil %1 a rejoint le socket.\n").arg(name));
}

void SocketDialog::reactOnSocketError(const QString &error)
{
    ui->textEditRponse->insertPlainText(error);
}

void SocketDialog::newAdapterSelected()
{
    const int newAdapterIndex = adapterFromUserSelection();
    if (currentAdapterIndex != newAdapterIndex) {
        server->stopServer();
        currentAdapterIndex = newAdapterIndex;
        const QBluetoothHostInfo info = localAdapters.at(currentAdapterIndex);
        QBluetoothLocalDevice adapter(info.address());
        adapter.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
        server->startServer(info.address());
        localName = info.name();
    }
}

int SocketDialog::adapterFromUserSelection() const
{
    int result = 0;
    QBluetoothAddress newAdapter = localAdapters.at(0).address();
    if (ui->secondAdapter->isChecked()) {
        newAdapter = localAdapters.at(1).address();
        result = 1;
    }
    return result;
}
