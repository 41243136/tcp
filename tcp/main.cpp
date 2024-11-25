#include "tcpfileserver.h"
#include "tcpfilesender.h"
#include <QApplication>
#include <QInputDialog>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDialog>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDialog dialog;
    QFormLayout form(&dialog);

    QLineEdit *ipLineEdit = new QLineEdit("127.0.0.1");
    QLineEdit *portLineEdit = new QLineEdit("16998");

    form.addRow("IP地址:", ipLineEdit);
    form.addRow("Port號碼:", portLineEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString ipAddress = ipLineEdit->text();
        quint16 port = portLineEdit->text().toUShort();

        TcpFileServer server;
        TcpFileSender client;

        server.show();
        client.setConnectionDetails(ipAddress, port);
        client.show();

        return a.exec();
    } else {
        return 1;
    }
}
