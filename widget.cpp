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
    setStyleSheet("#Widget{background:#98a39a;}");
    resize(1000, 660);

    connect(mMainpage, &Mainpage::enter, this, &Widget::onEnter);

    connect(mSocket, &QTcpSocket::connected, this, &Widget::onConnected);
    connect(mSocket, &QTcpSocket::disconnected, this, &Widget::onDisconnected);
    connect(mSocket, &QTcpSocket::readyRead, this, &Widget::onReadyRead);
    connect(mSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, [this](QAbstractSocket::SocketError err) {
        if(err != QAbstractSocket::RemoteHostClosedError)
            QMessageBox::critical(this, "错误", QString("连接失败\n") + QMetaEnum::fromType<QAbstractSocket::SocketError>().key(err));
    });

    connect(mSleepRoom, &SleepRoom::sPos, this, &Widget::pos);
    connect(mSleepRoom, &SleepRoom::sMove, this, &Widget::move);
    connect(mSleepRoom, &SleepRoom::sSleep, this, &Widget::sleep);
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
    QList<QByteArray> codes = mSocket->readAll().split(EOF);
    for(const QByteArray &code : qAsConst(codes)) {
        QJsonDocument doc = QJsonDocument::fromJson(code);
        if(doc.isNull())
            return;
        QJsonObject root = doc.object();
        QString type = root.value("type").toString();

        if(type == "ConnErr") {
            QMessageBox::critical(this, "错误", "连接失败\n" + root.value("str").toString());
        } else if(type == "ConnSucc") {
            mSleepRoom->clear();
            mSleepRoom->setName(mName);
            mSleepRoom->setRole(mRole);
            mSleepRoom->setId(root.value("id").toString().toULongLong());
            mStkLayout->setCurrentWidget(mSleepRoom);
        } else if(type == "pos") {
            mSleepRoom->onPos(
                        root.value("id").toString().toULongLong(),
                        root.value("x").toDouble(),
                        root.value("y").toDouble()
                        );
        } else if(type == "move") {
            mSleepRoom->onMove(
                        root.value("id").toString().toULongLong(),
                        root.value("x").toDouble(),
                        root.value("y").toDouble()
                        );
        } else if(type == "sleep") {
            mSleepRoom->onSleep(
                        root.value("id").toString().toULongLong(),
                        root.value("bx").toInt(),
                        root.value("by").toInt()
                        );
        }
    }
}

void Widget::pos(double x, double y) {
    QJsonDocument doc;
    QJsonObject root;
    root.insert("type", "pos");
    root.insert("x", x);
    root.insert("y", y);
    doc.setObject(root);
    mSocket->write(doc.toJson(QJsonDocument::Compact));
    mSocket->write(QByteArray(1, EOF));
}
void Widget::move(double x, double y) {
    QJsonDocument doc;
    QJsonObject root;
    root.insert("type", "move");
    root.insert("x", x);
    root.insert("y", y);
    doc.setObject(root);
    mSocket->write(doc.toJson(QJsonDocument::Compact));
    mSocket->write(QByteArray(1, EOF));
}
void Widget::sleep(int x, int y) {
    QJsonDocument doc;
    QJsonObject root;
    root.insert("type", "sleep");
    root.insert("bx", x);
    root.insert("by", y);
    doc.setObject(root);
    mSocket->write(doc.toJson(QJsonDocument::Compact));
    mSocket->write(QByteArray(1, EOF));
}

void Widget::closeEvent(QCloseEvent *) {
    disconnect(mSocket, &QTcpSocket::disconnected, this, &Widget::onDisconnected);
}

