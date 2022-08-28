#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>

namespace Ui {
class Mainpage;
}

class Mainpage : public QWidget
{
    Q_OBJECT

public:
    explicit Mainpage(QWidget *parent = nullptr);
    ~Mainpage() override;

    struct Data {
        QString name;
        int role;
        QHostAddress address;
        quint16 port;
    };
    Data data();

    void saveConfig();

signals:
    void enter();

private:
    Ui::Mainpage *ui;

    QSettings mConfig;
};

#endif // MAINPAGE_H
