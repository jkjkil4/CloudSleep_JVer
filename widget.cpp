#include "widget.h"

#include "Mainpage/mainpage.h"
#include "SleepRoom/sleeproom.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent), mStkLayout(new QStackedLayout),
      mMainpage(new Mainpage(this)), mSleepRoom(new SleepRoom(this)),
      mSocket(new QTcpSocket(this))
{
    mSleepRoom->repaint();

    mStkLayout->setContentsMargins(QMargins());
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(mSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), this, [this](QAbstractSocket::SocketError err) {
#else
    connect(mSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, [this](QAbstractSocket::SocketError err) {
#endif
        if(err != QAbstractSocket::RemoteHostClosedError)
            QMessageBox::critical(this, "错误", QString("连接失败\n") + QMetaEnum::fromType<QAbstractSocket::SocketError>().key(err));
    });

    connect(mSleepRoom, &SleepRoom::sPos, this, &Widget::pos);
    connect(mSleepRoom, &SleepRoom::sMove, this, &Widget::move);
    connect(mSleepRoom, &SleepRoom::sSleep, this, &Widget::sleep);
    connect(mSleepRoom, &SleepRoom::sChat, this, &Widget::chat);
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
    mSocketBuffer.clear();
}

void Widget::onReadyRead() {
    QByteArray all = mSocket->readAll();
    QList<QByteArray> codes = all.split((char)EOF);
    if(!mSocketBuffer.isEmpty() && !codes.isEmpty())
        codes[0].insert(0, mSocketBuffer);

    if(!all.isEmpty() && all.at(all.length() - 1) != (char)EOF && !codes.isEmpty()) {
        mSocketBuffer = *codes.crbegin();
        codes.removeLast();
    } else mSocketBuffer.clear();

    for(const QByteArray &code : qAsConst(codes)) {
        if(code.isEmpty())
            continue;
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
        } else if(type == "blocked") {
            mSleepRoom->onBlocked(root.value("bx").toInt(), root.value("by").toInt());
        } else if(type == "sleeper") {
            mSleepRoom->onSleeper(
                        root.value("name").toString(), root.value("role").toInt(),
                        root.value("id").toString().toULongLong(),
                        root.value("x").toDouble(0), root.value("y").toDouble(0),
                        root.value("bx").toInt(0), root.value("by").toInt(0),
                        root.value("inBed").toBool(false)
                        );
        } else if(type == "chat") {
            mSleepRoom->onChat(
                        root.value("id").toString().toULongLong(),
                        root.value("str").toString()
                        );
        } else if(type == "leave") {
            mSleepRoom->onLeave(root.value("id").toString().toULongLong());
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
    mSocket->write(QByteArray(1, (char)EOF));
}
void Widget::move(double x, double y) {
    QJsonDocument doc;
    QJsonObject root;
    root.insert("type", "move");
    root.insert("x", x);
    root.insert("y", y);
    doc.setObject(root);
    mSocket->write(doc.toJson(QJsonDocument::Compact));
    mSocket->write(QByteArray(1, (char)EOF));
}
void Widget::sleep(int bx, int by) {
    QJsonDocument doc;
    QJsonObject root;
    root.insert("type", "sleep");
    root.insert("bx", bx);
    root.insert("by", by);
    doc.setObject(root);
    mSocket->write(doc.toJson(QJsonDocument::Compact));
    mSocket->write(QByteArray(1, (char)EOF));
}
void Widget::chat(const QString &str) {
    QJsonDocument doc;
    QJsonObject root;
    root.insert("type", "chat");
    root.insert("str", str);
    doc.setObject(root);
    mSocket->write(doc.toJson(QJsonDocument::Compact));
    mSocket->write(QByteArray(1, (char)EOF));
}

void Widget::closeEvent(QCloseEvent *) {
    disconnect(mSocket, &QTcpSocket::disconnected, this, &Widget::onDisconnected);
}

