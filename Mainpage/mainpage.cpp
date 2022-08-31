#include "mainpage.h"
#include "ui_mainpage.h"

Mainpage::Mainpage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Mainpage), mConfig("config.ini", QSettings::IniFormat)
{
    ui->setupUi(this);

    ui->widgetRoles->setInd(mConfig.value("Cfg/Role").toInt());
    ui->editName->setText(mConfig.value("Cfg/Name").toString());
    ui->editServ->setText(mConfig.value("Cfg/Serv", "39.107.236.153:18180").toString());

    connect(ui->btnLeft, &QPushButton::clicked, ui->widgetRoles, [this] { ui->widgetRoles->adjust(-1); });
    connect(ui->btnRight, &QPushButton::clicked, ui->widgetRoles, [this] { ui->widgetRoles->adjust(1); });
    connect(ui->btnEnter, SIGNAL(clicked()), this, SIGNAL(enter()));
}

Mainpage::~Mainpage()
{
    delete ui;
}

Mainpage::Data Mainpage::data() {
    QString str = ui->editServ->text();
    int ind = (int)str.indexOf(':');
    return { ui->editName->text().trimmed(), ui->widgetRoles->ind(), QHostAddress(str.left(ind)), (quint16)str.mid(ind + 1).toInt() };
}

void Mainpage::saveConfig() {
    mConfig.setValue("Cfg/Role", ui->widgetRoles->ind());
    mConfig.setValue("Cfg/Name", ui->editName->text());
    mConfig.setValue("Cfg/Serv", ui->editServ->text());
}
