#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QStackedLayout>
#include <QTcpSocket>
#include <QMessageBox>
#include <QMetaEnum>
#include <QJsonArray>

class Mainpage;
class SleepRoom;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget() override;

public slots:
    void onEnter();

    void onConnected();
    void onDisconnected();
    void onReadyRead();

    void pos(double x, double y);
    void move(double x, double y);
    void sleep(int bx, int by);
    void chat(const QString &str);

protected:
    void closeEvent(QCloseEvent *) override;

private:
    QStackedLayout *mStkLayout;
    Mainpage *mMainpage;
    SleepRoom *mSleepRoom;

    QTcpSocket *mSocket;
    QByteArray mSocketBuffer;

    QString mName;
    int mRole = 0;
    qulonglong mId = 0;
};
#endif // WIDGET_H
