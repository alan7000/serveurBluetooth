#ifndef SOCKETDIALOG_H
#define SOCKETDIALOG_H
#include "ui_socketdialog.h"

#include <QDialog>

#include <QtBluetooth/qbluetoothhostinfo.h>

QT_USE_NAMESPACE
class socketBluetooth;
class ClientSocketBluetooth;

namespace Ui {
class SocketDialog;
}

class SocketDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SocketDialog(QWidget *parent = nullptr);
    ~SocketDialog();

private:
    Ui::SocketDialog *ui;

signals:
    void sendMessage(const QString &message);

private slots:
    void connectClicked();
    void sendClicked();

    void showMessage(const QString &sender, const QString &message);

    void clientConnected(const QString &name);
    void clientDisconnected(const QString &name);
    void clientDisconnected();
    void connected(const QString &name);
    void reactOnSocketError(const QString &error);

    void newAdapterSelected();

private:
    int adapterFromUserSelection() const;
    int currentAdapterIndex = 0;

    socketBluetooth *server;
    QList<ClientSocketBluetooth *> clients;
    QList<QBluetoothHostInfo> localAdapters;

    QString localName;
};

#endif // SOCKETDIALOG_H
