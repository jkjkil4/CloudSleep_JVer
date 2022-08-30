#include "roledisplay.h"

RoleDisplay::RoleDisplay(QWidget *parent)
    : QWidget(parent), mRoles({
                              QPixmap(":/role/src/Boy.png"),
                              QPixmap(":/role/src/Girl.png")
                              })
{
#ifdef Q_OS_ANDROID
    setMinimumSize(100, 100);
    setMaximumSize(100, 100);
#else
    setMinimumSize(282, 282);
    setMaximumSize(282, 282);
#endif
}

void RoleDisplay::setInd(int ind) {
    mInd = qBound(0, ind, mRoles.length() - 1);
    mOffset = -mInd * width();
}

void RoleDisplay::adjust(int indOffset) {
    static QPropertyAnimation *anim = nullptr;
    if(anim) {
        anim->stop();
        mOffset = -mInd * width();
    }
    mInd = qBound(0, mInd + indOffset, mRoles.length() - 1);
    anim = new QPropertyAnimation(this, "mOffset", this);
    anim->setStartValue(mOffset);
    anim->setEndValue(-mInd * width());
    anim->setDuration(400);
    anim->setEasingCurve(QEasingCurve::InOutQuart);
    connect(anim, &QPropertyAnimation::finished, [] {
        delete anim;
        anim = nullptr;
    });
    anim->start();
}

void RoleDisplay::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    for(int i = qMax(0, mInd - 1); i <= qMin(mRoles.length() - 1, mInd + 1); i++) {
        p.drawPixmap(QRect(QPoint(i * width() + mOffset, 0), size()), mRoles[i]);
    }
}

void RoleDisplay::setOffset(int offset) {
    mOffset = offset;
    update();
}
