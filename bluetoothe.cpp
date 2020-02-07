#include "bluetoothe.h"
#include "ui_bluetoothe.h"

#include <qbluetoothdeviceinfo.h>
#include <qbluetoothaddress.h>
#include <qbluetoothtransferrequest.h>
#include <qbluetoothtransferreply.h>
#include <qbluetoothlocaldevice.h>

#include <QMovie>
#include <QMessageBox>
#include <QFileDialog>
#include <QCheckBox>

QT_USE_NAMESPACE

Bluetoothe::Bluetoothe(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Bluetoothe),
    m_localDevice(new QBluetoothLocalDevice),
    m_pairingError(false)
{
    ui->setupUi(this);

    //Using default Bluetooth adapter
    QBluetoothAddress adapterAddress = m_localDevice->address();

    /*
     * In case of multiple Bluetooth adapters it is possible to
     * set which adapter will be used by providing MAC Address.
     * Example code:
     *
     * QBluetoothAddress adapterAddress("XX:XX:XX:XX:XX:XX");
     * m_discoveryAgent = new QBluetoothServiceDiscoveryAgent(adapterAddress);
     */

    m_discoveryAgent = new QBluetoothServiceDiscoveryAgent(adapterAddress);

    connect(m_discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),
            this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));
    connect(m_discoveryAgent, SIGNAL(finished()), this, SLOT(discoveryFinished()));
    connect(m_discoveryAgent, SIGNAL(canceled()), this, SLOT(discoveryFinished()));

    ui->tableWidget->setColumnWidth(3, 75);
    ui->tableWidget->setColumnWidth(4, 100);

    connect(m_localDevice, SIGNAL(pairingFinished(QBluetoothAddress,QBluetoothLocalDevice::Pairing)),
            this, SLOT(pairingFinished(QBluetoothAddress,QBluetoothLocalDevice::Pairing)));
    connect(m_localDevice, SIGNAL(error(QBluetoothLocalDevice::Error)),
            this, SLOT(pairingError(QBluetoothLocalDevice::Error)));

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    this->startDiscovery();
    this->setFlag(false);
}

Bluetoothe::~Bluetoothe()
{
    delete ui;
    delete m_discoveryAgent;
    delete m_localDevice;
}

void Bluetoothe::startDiscovery(const QBluetoothUuid &uuid)
{
    qWarning("start discovery");
    ui->StopButton->setDisabled(false);
    if (m_discoveryAgent->isActive())
        m_discoveryAgent->stop();

    m_discoveryAgent->setUuidFilter(uuid);
    m_discoveryAgent->start();

    if (!m_discoveryAgent->isActive() ||
            m_discoveryAgent->error() != QBluetoothServiceDiscoveryAgent::NoError) {
        ui->Status->setText(tr("Cannot find remote services."));
    } else {
        ui->Status->setText(tr("Scanning..."));
    }
}

QBluetoothServiceInfo Bluetoothe::service() const
{
    return m_service;
}

void Bluetoothe::startDiscovery()
{
    startDiscovery(QBluetoothUuid(QBluetoothUuid::ObexObjectPush));
}

void Bluetoothe::on_pushButton_clicked()
{
    reject();
}

void Bluetoothe::on_StopButton_clicked()
{
    m_discoveryAgent->stop();
}

void Bluetoothe::on_ActualiserButton_clicked()
{
    startDiscovery();
    ui->StopButton->setDisabled(false);
}

void Bluetoothe::serviceDiscovered(const QBluetoothServiceInfo &serviceInfo)
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

    QString remoteName;
    if (serviceInfo.device().name().isEmpty())
        remoteName = serviceInfo.device().address().toString();
    else
        remoteName = serviceInfo.device().name();

    //    QListWidgetItem *item =
    //        new QListWidgetItem(QString::fromLatin1("%1\t%2\t%3").arg(serviceInfo.device().address().toString(),
    //                                                             serviceInfo.device().name(), serviceInfo.serviceName()));

    QMutableMapIterator<int, QBluetoothServiceInfo> i(m_discoveredServices);
    while (i.hasNext()){
        i.next();
        if (serviceInfo.device().address() == i.value().device().address()){
            i.setValue(serviceInfo);
            return;
        }
    }

    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    QTableWidgetItem *item = new QTableWidgetItem(serviceInfo.device().address().toString());
    ui->tableWidget->setItem(row, 0, item);
    item = new QTableWidgetItem(serviceInfo.device().name());
    ui->tableWidget->setItem(row, 1, item);
    item = new QTableWidgetItem(serviceInfo.serviceName());

    ui->tableWidget->setItem(row, 2, item);

    QBluetoothLocalDevice::Pairing p;

    p = m_localDevice->pairingStatus(serviceInfo.device().address());

    ui->tableWidget->blockSignals(true);

    item = new QTableWidgetItem();
    if ((p&QBluetoothLocalDevice::Paired) || (p&QBluetoothLocalDevice::AuthorizedPaired))
        item->setCheckState(Qt::Checked);
    else
        item->setCheckState(Qt::Unchecked);
    ui->tableWidget->setItem(row, 3, item);

    item = new QTableWidgetItem();
    if (p&QBluetoothLocalDevice::AuthorizedPaired)
        item->setCheckState(Qt::Checked);
    else
        item->setCheckState(Qt::Unchecked);

    ui->tableWidget->setItem(row, 4, item);

    ui->tableWidget->blockSignals(false);


    m_discoveredServices.insert(row, serviceInfo);
}

void Bluetoothe::discoveryFinished()
{
    ui->Status->setText(tr("Select the device to send to."));
    ui->StopButton->setDisabled(true);
}

void Bluetoothe::pairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing)
{
    QBluetoothServiceInfo service;
    int row = 0;

    ui->tableWidget->blockSignals(true);

    for (int i = 0; i < m_discoveredServices.count(); i++){
        if (m_discoveredServices.value(i).device().address() == address){
            service = m_discoveredServices.value(i);
            row = i;
            break;
        }
    }

    QMessageBox msgBox;
    if (m_pairingError) {
        msgBox.setText("Pairing failed with " + address.toString());
    } else if (pairing == QBluetoothLocalDevice::Paired
               || pairing == QBluetoothLocalDevice::AuthorizedPaired) {
        msgBox.setText("Paired successfully with " + address.toString());
    } else {
        msgBox.setText("Pairing released with " + address.toString());
    }

    if (service.isValid()){
        if (pairing == QBluetoothLocalDevice::AuthorizedPaired){
            ui->tableWidget->item(row, 3)->setCheckState(Qt::Checked);
            ui->tableWidget->item(row, 4)->setCheckState(Qt::Checked);
        }
        else if (pairing == QBluetoothLocalDevice::Paired){
            ui->tableWidget->item(row, 3)->setCheckState(Qt::Checked);
            ui->tableWidget->item(row, 4)->setCheckState(Qt::Unchecked);
        }
        else {
            ui->tableWidget->item(row, 3)->setCheckState(Qt::Unchecked);
            ui->tableWidget->item(row, 4)->setCheckState(Qt::Unchecked);
        }
    }

    m_pairingError = false;
    msgBox.exec();

    ui->tableWidget->blockSignals(false);
}

void Bluetoothe::pairingError(QBluetoothLocalDevice::Error error)
{
    if (error != QBluetoothLocalDevice::PairingError)
        return;

    m_pairingError = true;
    pairingFinished(m_service.device().address(), QBluetoothLocalDevice::Unpaired);
}


void Bluetoothe::displayConfReject()
{
    m_localDevice->pairingConfirmation(false);
}

void Bluetoothe::displayConfAccepted()
{
    m_localDevice->pairingConfirmation(true);
}

QString Bluetoothe::addressToName(const QBluetoothAddress &address)
{
    QMapIterator<int, QBluetoothServiceInfo> i(m_discoveredServices);
    while (i.hasNext()){
        i.next();
        if (i.value().device().address() == address)
            return i.value().device().name();
    }
    return address.toString();
}

bool Bluetoothe::getFlag() const
{
    return flag;
}

void Bluetoothe::setFlag(bool value)
{
    flag = value;
}
QString Bluetoothe::getTab() const
{
    return tab;
}

void Bluetoothe::setTab(const QString &value)
{
    tab = value;
}


void Bluetoothe::on_tableWidget_itemActivated(QTableWidgetItem *item)
{
    int row = item->row();
    int column = item->column();
    m_service = m_discoveredServices.value(row);

    if (column < 3)
        return;

    if (item->checkState() == Qt::Unchecked && column == 3){
        m_localDevice->requestPairing(m_service.device().address(), QBluetoothLocalDevice::Unpaired);
        return; // don't continue and start movie
    }
    else if ((item->checkState() == Qt::Checked && column == 3) ||
             (item->checkState() == Qt::Unchecked && column == 4)){
        m_localDevice->requestPairing(m_service.device().address(), QBluetoothLocalDevice::Paired);
        ui->tableWidget->blockSignals(true);
        ui->tableWidget->item(row, column)->setCheckState(Qt::PartiallyChecked);
        ui->tableWidget->blockSignals(false);
    }
    else if (item->checkState() == Qt::Checked && column == 4){
        m_localDevice->requestPairing(m_service.device().address(), QBluetoothLocalDevice::AuthorizedPaired);
        ui->tableWidget->blockSignals(true);
        ui->tableWidget->item(row, column)->setCheckState(Qt::PartiallyChecked);
        ui->tableWidget->blockSignals(false);
    }
}

void Bluetoothe::on_tableWidget_cellClicked(int row, int column)
{
    Q_UNUSED(column);

    m_service = m_discoveredServices.value(row);

    ui->tableWidget->selectRow(row);
}

void Bluetoothe::on_SelectDeviceButton_clicked()
{
    this->setTab(m_service.device().address().toString());
    qWarning("%s", getTab().toUtf8().data());
    ui->Status->setText("Appareil " + m_service.device().name());

    //mettre les boutons enable
    //    this->dialog->pushButtonTrame->setEnabled(true);
    //    this->dialog->pushButton_sendFile->setEnabled(true);
    //    this->dialog->pushButton_SelectFile->setEnabled(true);
    this->setFlag(true);
}
