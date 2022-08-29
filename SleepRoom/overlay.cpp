#include "overlay.h"
#include "ui_overlay.h"

Overlay::Overlay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Overlay)
{
    ui->setupUi(this);
}

Overlay::~Overlay()
{
    delete ui;
}

void Overlay::paintEvent(QPaintEvent *) {
    QPainter p(this);
    emit paint(&p);
}
