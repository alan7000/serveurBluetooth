#include "remoteselectorfiledialog.h"
#include "ui_remoteselectorfiledialog.h"

#include <qbluetoothdeviceinfo.h>
#include <qbluetoothaddress.h>
#include <qbluetoothtransferrequest.h>
#include <qbluetoothtransferreply.h>
#include <qbluetoothlocaldevice.h>

#include <QMovie>
#include <QMessageBox>
#include <QFileDialog>
#include <QCheckBox>

#include "pindisplay.h"

QT_USE_NAMESPACE

RemoteSelectorFileDialog::RemoteSelectorFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoteSelectorFileDialog),
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

    connect(m_localDevice, SIGNAL(pairingDisplayPinCode(QBluetoothAddress,QString)),
            this, SLOT(displayPin(QBluetoothAddress,QString)));
    connect(m_localDevice, SIGNAL(pairingDisplayConfirmation(QBluetoothAddress,QString)),
            this, SLOT(displayConfirmation(QBluetoothAddress,QString)));
    connect(m_localDevice, SIGNAL(pairingFinished(QBluetoothAddress,QBluetoothLocalDevice::Pairing)),
            this, SLOT(pairingFinished(QBluetoothAddress,QBluetoothLocalDevice::Pairing)));
    connect(m_localDevice, SIGNAL(error(QBluetoothLocalDevice::Error)),
            this, SLOT(pairingError(QBluetoothLocalDevice::Error)));


    ui->busyWidget->setMovie(new QMovie(":/icons/busy.gif"));
    ui->busyWidget->movie()->start();

    ui->pairingBusy->setMovie(new QMovie(":/icons/pairing.gif"));
    ui->pairingBusy->hide();

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    this->startDiscovery();
    this->setFlag(false);
}

RemoteSelectorFileDialog::~RemoteSelectorFileDialog()
{
    delete ui;
    delete m_discoveryAgent;
    delete m_localDevice;
}

//void RemoteSelectorFileDialog::startDiscovery(const QBluetoothUuid &uuid)
//{
//    qWarning("start discovery");
//    ui->StopButton->setDisabled(false);
//    if (m_discoveryAgent->isActive())
//        m_discoveryAgent->stop();

//    m_discoveryAgent->setUuidFilter(uuid);
//    m_discoveryAgent->start();

//    if (!m_discoveryAgent->isActive() ||
//            m_discoveryAgent->error() != QBluetoothServiceDiscoveryAgent::NoError) {
//        ui->Status->setText(tr("Cannot find remote services."));
//    } else {
//        ui->Status->setText(tr("Scanning..."));
//        ui->busyWidget->show();
//        ui->busyWidget->movie()->start();
//    }
//}

QBluetoothServiceInfo RemoteSelectorFileDialog::service() const
{
    return m_service;
}

void RemoteSelectorFileDialog::startDiscovery(const QBluetoothUuid &uuid)
{
    qWarning("Start discovery");
    ui->StopButton->setDisabled(false);
    if (m_discoveryAgent->isActive()) {
        m_discoveryAgent->stop();
    }

    m_discoveryAgent->setUuidFilter(uuid);
    m_discoveryAgent->start();

    if (!m_discoveryAgent->isActive() ||
            m_discoveryAgent->error() != QBluetoothServiceDiscoveryAgent::NoError) {
        ui->Status->setText(tr("Cannot find remote services."));
    } else {
        ui->Status->setText(tr("Scanning..."));
        ui->busyWidget->show();
        ui->busyWidget->movie()->start();
    }
}

void RemoteSelectorFileDialog::on_pushButton_clicked()
{
    reject();
}

void RemoteSelectorFileDialog::on_StopButton_clicked()
{
    m_discoveryAgent->stop();
}

void RemoteSelectorFileDialog::on_ActualiserButton_clicked()
{
    startDiscovery();
    ui->StopButton->setDisabled(false);
}

void RemoteSelectorFileDialog::serviceDiscovered(const QBluetoothServiceInfo &serviceInfo)
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

void RemoteSelectorFileDialog::discoveryFinished()
{
    ui->Status->setText(tr("Select the device to send to."));
    ui->StopButton->setDisabled(true);
    ui->busyWidget->movie()->stop();
    ui->busyWidget->hide();
}

void RemoteSelectorFileDialog::pairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing status)
{
    QBluetoothServiceInfo service;
    int row = 0;

    ui->pairingBusy->hide();
    ui->pairingBusy->movie()->stop();

    ui->tableWidget->blockSignals(true);

    for (int i = 0; i < m_discoveredServices.count(); i++){
        if (m_discoveredServices.value(i).device().address() == address){
            service = m_discoveredServices.value(i);
            row = i;
            break;
        }
    }

    if (m_pindisplay) {
        delete m_pindisplay;
    }

    QMessageBox msgBox;
    if (m_pairingError) {
        msgBox.setText("Pairing failed with " + address.toString());
    } else if (status == QBluetoothLocalDevice::Paired
               || status == QBluetoothLocalDevice::AuthorizedPaired) {
        msgBox.setText("Paired successfully with " + address.toString());
    } else {
        msgBox.setText("Pairing released with " + address.toString());
    }

    if (service.isValid()){
        if (status == QBluetoothLocalDevice::AuthorizedPaired){
            ui->tableWidget->item(row, 3)->setCheckState(Qt::Checked);
            ui->tableWidget->item(row, 4)->setCheckState(Qt::Checked);
        }
        else if (status == QBluetoothLocalDevice::Paired){
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

void RemoteSelectorFileDialog::pairingError(QBluetoothLocalDevice::Error error)
{
    if (error != QBluetoothLocalDevice::PairingError)
        return;

    m_pairingError = true;
    pairingFinished(m_service.device().address(), QBluetoothLocalDevice::Unpaired);
}

void RemoteSelectorFileDialog::displayPin(const QBluetoothAddress &address, QString pin)
{
    if (m_pindisplay)
        m_pindisplay->deleteLater();
    m_pindisplay = new pinDisplay(QString("Enter pairing pin on: %1").arg(addressToName(address)), pin, this);
    m_pindisplay->show();
}

void RemoteSelectorFileDialog::displayConfirmation(const QBluetoothAddress &address, QString pin)
{
    Q_UNUSED(address);

    if (m_pindisplay)
        m_pindisplay->deleteLater();
    m_pindisplay = new pinDisplay(QString("Confirm this pin is the same"), pin, this);
    connect(m_pindisplay, SIGNAL(accepted()), this, SLOT(displayConfAccepted()));
    connect(m_pindisplay, SIGNAL(rejected()), this, SLOT(displayConfReject()));
    m_pindisplay->setOkCancel();
    m_pindisplay->show();
}


void RemoteSelectorFileDialog::displayConfReject()
{
    m_localDevice->pairingConfirmation(false);
}

void RemoteSelectorFileDialog::displayConfAccepted()
{
    m_localDevice->pairingConfirmation(true);
}

QString RemoteSelectorFileDialog::addressToName(const QBluetoothAddress &address)
{
    QMapIterator<int, QBluetoothServiceInfo> i(m_discoveredServices);
    while (i.hasNext()){
        i.next();
        if (i.value().device().address() == address)
            return i.value().device().name();
    }
    return address.toString();
}

bool RemoteSelectorFileDialog::getFlag() const
{
    return flag;
}

void RemoteSelectorFileDialog::setFlag(bool value)
{
    flag = value;
}

void RemoteSelectorFileDialog::startDiscovery()
{
    startDiscovery(QBluetoothUuid(QBluetoothUuid::ObexObjectPush));
}
QString RemoteSelectorFileDialog::getTab() const
{
    return tab;
}

void RemoteSelectorFileDialog::setTab(const QString &value)
{
    tab = value;
}

void RemoteSelectorFileDialog::on_tableWidget_cellClicked(int row, int column)
{
    Q_UNUSED(column);

    m_service = m_discoveredServices.value(row);

    ui->tableWidget->selectRow(row);
}

void RemoteSelectorFileDialog::on_SelectDeviceButton_clicked()
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

void RemoteSelectorFileDialog::on_tableWidget_itemChanged(QTableWidgetItem *item)
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
    ui->pairingBusy->show();
    ui->pairingBusy->movie()->start();
}
