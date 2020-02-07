#include "remoteselectordialog.h"
#include "ui_remoteselectordialog.h"

#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/qbluetoothservicediscoveryagent.h>

QT_USE_NAMESPACE

RemoteSelectorDialog::RemoteSelectorDialog(const QBluetoothAddress &localAdapter, QWidget *parent) : QDialog(parent), ui(new Ui::RemoteSelectorDialog)
{
    ui->setupUi(this);

    m_discoveryAgent = new QBluetoothServiceDiscoveryAgent(localAdapter);

    connect(m_discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)), this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));
    connect(m_discoveryAgent, SIGNAL(finished()), this, SLOT(discoveryFinished()));
    connect(m_discoveryAgent, SIGNAL(canceled()), this, SLOT(discoveryFinished()));
}

RemoteSelectorDialog::~RemoteSelectorDialog()
{
    delete ui;
    delete m_discoveryAgent;
}

void RemoteSelectorDialog::startDiscovery(const QBluetoothUuid &uuid)
{
    ui->status->setText(tr("Scanning..."));
    if (m_discoveryAgent->isActive())
        m_discoveryAgent->stop();

    ui->remoteDevices->clear();

    m_discoveryAgent->setUuidFilter(uuid);
    m_discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
}

void RemoteSelectorDialog::stopDiscovery()
{
    if (m_discoveryAgent){
        m_discoveryAgent->stop();
    }
}

QBluetoothServiceInfo RemoteSelectorDialog::service() const
{
    return m_service;
}

void RemoteSelectorDialog::serviceDiscovered(const QBluetoothServiceInfo &serviceInfo)
{
#if 0
    qDebug() << "Discovered service on"
             << serviceInfo.device().name() << serviceInfo.device().address().toString();
    qDebug() << "\tService name:" << serviceInfo.serviceName();
    qDebug() << "\tDescription:"
             << serviceInfo.attribute(QBluetoothServiceInfo::ServiceDescription).toString();
    qDebug() << "\tProvider:"
             << serviceInfo.attribute(QBluetoothServiceInfo::ServiceProvider).toString();
    qDebug() << "\tL2CAP protocol service multiplexer:"
             << serviceInfo.protocolServiceMultiplexer();
    qDebug() << "\tRFCOMM server channel:" << serviceInfo.serverChannel();
#endif
    QMapIterator<QListWidgetItem *, QBluetoothServiceInfo> i(m_discoveredServices);
    while (i.hasNext()){
        i.next();
        if (serviceInfo.device().address() == i.value().device().address()){
            return;
        }
    }

    QString remoteName;
    if (serviceInfo.device().name().isEmpty())
        remoteName = serviceInfo.device().address().toString();
    else
        remoteName = serviceInfo.device().name();

    QListWidgetItem *item =
        new QListWidgetItem(QString::fromLatin1("%1 %2").arg(remoteName,
                                                             serviceInfo.serviceName()));

    m_discoveredServices.insert(item, serviceInfo);
    ui->remoteDevices->addItem(item);
}

void RemoteSelectorDialog::discoveryFinished()
{
    ui->status->setText(tr("Selectionner l'appareil que vous voulez connecter."));
}

void RemoteSelectorDialog::on_remoteDevices_itemActivated(QListWidgetItem *item)
{
    qDebug() << "got click" << item->text();
    m_service = m_discoveredServices.value(item);
    if (m_discoveryAgent->isActive())
        m_discoveryAgent->stop();

    accept();
}

void RemoteSelectorDialog::on_cancelButton_clicked()
{
    reject();
}


