#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QStackedLayout>
#include <QTcpSocket>
#include <QMessageBox>
#include <QMetaEnum>

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

protected:
    void closeEvent(QCloseEvent *) override;

private:
    QStackedLayout *mStkLayout;
    Mainpage *mMainpage;
    SleepRoom *mSleepRoom;

    QTcpSocket *mSocket;

    QString mName;
    int mRole = 0;
    qulonglong mId = 0;
};
#endif // WIDGET_H
