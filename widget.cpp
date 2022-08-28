#include "widget.h"

#include "Mainpage/mainpage.h"
#include "SleepRoom/sleeproom.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent), mStkLayout(new QStackedLayout),
      mMainpage(new Mainpage(this)), mSleepRoom(new SleepRoom(this)),
      mSocket(new QTcpSocket(this))
{
    mStkLayout->setMargin(0);
    mStkLayout->addWidget(mMainpage);
    mStkLayout->addWidget(mSleepRoom);
    mStkLayout->setCurrentWidget(mMainpage);
    setLayout(mStkLayout);

    setObjectName("Widget");
    setStyleSheet("#Widget{background:#444444;}");
    resize(1000, 660);

    connect(mMainpage, &Mainpage::enter, this, &Widget::onEnter);

    connect(mSocket, &QTcpSocket::connected, this, &Widget::onConnected);
    connect(mSocket, &QTcpSocket::disconnected, this, &Widget::onDisconnected);
    connect(mSocket, &QTcpSocket::readyRead, this, &Widget::onReadyRead);
    connect(mSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, [this](QAbstractSocket::SocketError err) {
        if(err != QAbstractSocket::RemoteHostClosedError)
            QMessageBox::critical(this, "错误", QString("连接失败\n") + QMetaEnum::fromType<QAbstractSocket::SocketError>().key(err));
    });
}

Widget::~Widget()
{
}

void Widget::onEnter() {
    Mainpage::Data data = mMainpage->data();
    mName = data.name;
    mRole = data.role;
    if(mSocket->state() != QTcpSocket::UnconnectedState)
        mSocket->disconnectFromHost();
    mSocket->connectToHost(QHostAddress(data.address), data.port);
    mMainpage->saveConfig();
}

void Widget::onConnected() {
    QJsonDocument doc;
    QJsonObject root;
    root.insert("name", mName);
    root.insert("role", mRole);
    doc.setObject(root);
    mSocket->write(doc.toJson(QJsonDocument::Compact));
}
void Widget::onDisconnected() {
    if(mStkLayout->currentWidget() == mSleepRoom) {
        QMessageBox::warning(this, "提示", "与服务器断开连接");
        mStkLayout->setCurrentWidget(mMainpage);
    }
}

void Widget::onReadyRead() {
    QJsonDocument doc = QJsonDocument::fromJson(mSocket->readAll());
    if(doc.isNull())
        return;
    QJsonObject root = doc.object();
    QString type = root.value("type").toString();

    if(type == "ConnErr") {
        QMessageBox::critical(this, "错误", "连接失败\n" + root.value("str").toString());
    } else if(type == "ConnSucc") {
        mStkLayout->setCurrentWidget(mSleepRoom);
    }
}

void Widget::closeEvent(QCloseEvent *) {
    disconnect(mSocket, &QTcpSocket::disconnected, this, &Widget::onDisconnected);
}

