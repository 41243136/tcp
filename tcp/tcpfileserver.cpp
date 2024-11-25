#include "tcpfileserver.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#define tr QStringLiteral
TcpFileServer::TcpFileServer(QWidget *parent)
    : QDialog(parent)
{
    totalBytes = 0;
    byteReceived = 0;
    fileNameSize = 0;
    serverProgressBar = new QProgressBar;
    serverStatusLabel = new QLabel(QStringLiteral("伺服器端就緒"));
    startButton = new QPushButton(QStringLiteral("接收"));
    quitButton = new QPushButton(QStringLiteral("退出"));
    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton,QDialogButtonBox::RejectRole);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(serverProgressBar);
    mainLayout->addWidget(serverStatusLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setWindowTitle(QStringLiteral("接收檔案"));
    connect(startButton, SIGNAL(clicked()), this, SLOT(start()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(&tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    connect(&tcpServer, SIGNAL(acceptError(QAbstractSocket::SocketError)),this,
            SLOT(displayError(QAbstractSocket::SocketError)));
 }

TcpFileServer::~TcpFileServer()
{

}
void TcpFileServer::start()
{
    startButton->setEnabled(false);
    byteReceived = 0;
    fileNameSize = 0;

    bool ok;
    QString ipAddress = QInputDialog::getText(this, tr("輸入IP地址"),
                                              tr("IP地址:"), QLineEdit::Normal,
                                              "127.0.0.1", &ok);
    if (!ok) {
        startButton->setEnabled(true);
        return;
    }

    quint16 port = QInputDialog::getInt(this, tr("輸入Port號碼"),
                                        tr("Port號碼:"), 16998, 1, 65535, 1, &ok);
    if (!ok) {
        startButton->setEnabled(true);
        return;
    }

    this->ipAddress = ipAddress;
    this->port = port;

    listen();
}

void TcpFileServer::listen()
{
    while(!tcpServer.isListening() &&
          !tcpServer.listen(QHostAddress(ipAddress), port))
    {
        QMessageBox::StandardButton ret = QMessageBox::critical(this,
                                                                QStringLiteral("迴圈"),
                                                                QStringLiteral("無法啟動伺服器: %1.").arg(tcpServer.errorString()),
                                                                QMessageBox::Retry | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel)
        {
            startButton->setEnabled(true);
            return;
        }
    }
    serverStatusLabel->setText(QStringLiteral("監聽中..."));
}

void TcpFileServer::acceptConnection()
{
    tcpServerConnection = tcpServer.nextPendingConnection();//取得已接受的客戶端連線
    connect(tcpServerConnection, SIGNAL(readyRead()),
            this, SLOT(updateServerProgress()));//連接readyRead()訊號至updateServerProgress()槽函數
    connect(tcpServerConnection, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));//連接error()訊號至displayError()槽函數
    serverStatusLabel->setText(QStringLiteral("接受連線"));
}

void TcpFileServer::updateServerProgress()
{
    QDataStream in(tcpServerConnection);
    in.setVersion(QDataStream::Qt_4_6);
    if(byteReceived <= sizeof(qint64)*2)
      {
         if((fileNameSize == 0) && (tcpServerConnection->bytesAvailable() >=
                  sizeof(qint64)*2))
          {
              in >> totalBytes >> fileNameSize;
              byteReceived += sizeof(qint64)*2;
          }
          if((fileNameSize != 0) && (tcpServerConnection->bytesAvailable() >=
                  fileNameSize)  )
          {
              in >> fileName;
              byteReceived += fileNameSize;
              localFile = new QFile(fileName);
              if(!localFile->open(QFile::WriteOnly))
              {
                  QMessageBox::warning(this, QStringLiteral("應用程式"),
                                    QStringLiteral("無法讀取檔案 %1：\n%2.").arg(fileName)
                                    .arg(localFile->errorString()));
                  return;
              }
          }else{
              return;
          }
       }

    if (byteReceived < totalBytes)
      {
          byteReceived += tcpServerConnection->bytesAvailable();
          inBlock = tcpServerConnection->readAll();
          localFile->write(inBlock);
          inBlock.resize(0);
      }

    serverProgressBar->setMaximum(totalBytes);
    serverProgressBar->setValue(byteReceived);
    qDebug() << byteReceived;
    serverStatusLabel->setText(QStringLiteral("已接收 %1 Bytes")
                               .arg(byteReceived));

    if (byteReceived == totalBytes)
      {
          tcpServerConnection->close();
          startButton->setEnabled(true);
          localFile->fileName();
          localFile->close();
          serverStatusLabel->setText(QStringLiteral("伺服器端就緒"));
          byteReceived = 0;
          fileNameSize = 0;
          totalBytes = 0;
      }



}

void TcpFileServer::displayError(QAbstractSocket::SocketError)
{
    QObject * server = qobject_cast<QObject *>(sender());
    if (server == tcpServerConnection) qDebug() << "Hi I am QTcpSocket";
    if (server == &tcpServer) qDebug() << "Hi I am QTcpServer";
   //if (socketError == QTcpSocket::RemoteHostClosedError)
        //return;

    QMessageBox::information(this, QStringLiteral("網絡錯誤"),
                             QStringLiteral("產生如下錯誤: %1.")
                             .arg(tcpServerConnection->errorString()));
    tcpServerConnection->close();
    serverProgressBar->reset();
    serverStatusLabel->setText(QStringLiteral("伺服器就緒"));
    startButton->setEnabled(true);

}






