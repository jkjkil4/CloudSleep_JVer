#include "overlay.h"
#include "ui_overlay.h"

Overlay::Overlay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Overlay)
{
    ui->setupUi(this);

    QPalette pal = ui->editChat->palette();
    pal.setBrush(QPalette::PlaceholderText, Qt::gray);
    ui->editChat->setPalette(pal);

    connect(ui->btnAssist, &QPushButton::clicked, this, &Overlay::onBtnAssistClicked);
}

Overlay::~Overlay()
{
    delete ui;
}

void Overlay::onBtnAssistClicked() {
#ifdef Q_OS_ANDROID
    QMessageBox::information(this, "使用帮助",
                             "若应用切至后台，会在数分钟内失去连接\n"
                             "\n"
                             "点击 - 移动睡客\n"
                             "拖动 - 拖动视野\n"
                             "双指 - 缩放视野\n"
                             "\n"
                             "在聊天框输入\n"
                             "/tp 玩家名称\n"
                             "可以传送到他旁边");
#else
    QMessageBox::information(this, "使用帮助",
                             "若电脑进入睡眠，会在数分钟内失去连接\n"
                             "\n"
                             "左键点击 - 移动睡客\n"
                             "左键拖动 - 拖动视野\n"
                             "滚轮 - 缩放视野\n"
                             "\n"
                             "在聊天框输入\n"
                             "/tp 玩家名称\n"
                             "可以传送到他旁边");
#endif
}

void Overlay::paintEvent(QPaintEvent *) {
    QPainter p(this);
    emit paint(&p);
}
