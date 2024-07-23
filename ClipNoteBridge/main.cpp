#include <QCoreApplication>
#include <QJsonObject>
#include <QWebSocket>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString url = QString::fromUtf8(argv[1]);
    QWebSocket webSocket;
    QObject::connect(&webSocket, &QWebSocket::connected, [&webSocket, url]() {
        QString msg = "{\"action\":\"cn\",\"url\":\"%1\"}";
        webSocket.sendTextMessage(msg.arg(url));
        QCoreApplication::exit(0);
    });
    QObject::connect(&webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred), []() {
        QCoreApplication::exit(1);
    });
    const QUrl serverUrl = QUrl("ws://127.0.0.1:5488");
    webSocket.open(serverUrl);
    return a.exec();
}
