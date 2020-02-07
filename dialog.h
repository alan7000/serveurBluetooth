#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

//Include dialog socket et file
#include "socketdialog.h"
#include "filedialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT
//! [Déclaration file transfert]
public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();
private slots:
    void on_pushButtonSocket_clicked();

    void on_pushButtonFile_clicked();

private:
    Ui::Dialog *ui;
    FileDialog fileDialog;
    SocketDialog socketDialog;
};
#endif // DIALOG_H
