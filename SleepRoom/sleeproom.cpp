#include "sleeproom.h"
#include <QDebug>

SleepRoom::SleepRoom(QWidget *parent)
    : QOpenGLWidget(parent), mTimer(new QTimer(this))
{
    data.asset.pixBedEmpty = QPixmap(":/bed/src/bedEmpty.png");
    data.asset.pixBedBoy = QPixmap(":/bed/src/bedBoy.png");
    data.asset.pixBedGirl = QPixmap(":/bed/src/bedGirl.png");

    mTimer->setTimerType(Qt::TimerType::PreciseTimer);
    mTimer->start(20);
    connect(mTimer, &QTimer::timeout, this, &SleepRoom::onTimerTimeout);
}

int SleepRoom::floorMod(int val, int mod) {
    return val >= 0 ? val % mod : ((-val) % mod == 0 ? 0 : (mod - (-val) % mod));
}

void SleepRoom::onTimerTimeout() {
//    QPoint mouse = mapFromGlobal(cursor().pos());
    update();
}

double SleepRoom::GLXToWinX(double glx) {
    return (glx + 1) / 2 * width();
}
double SleepRoom::GLYToWinY(double gly) {
    return (-gly + 1) / 2 * height();
}
double SleepRoom::winXToGLX(double winx) {
    return winx * 2 / width() - 1;
}
double SleepRoom::winYToGLY(double winy) {
    return 1 - winy * 2 / height();
}

double SleepRoom::winXToViewX(double winx) {
    return data.player.x + data.view.xOffset + data.view.scaleFactor * (winx - width() / 2);
}
double SleepRoom::winYToViewY(double winy) {
    return data.player.y + data.view.yOffset + data.view.scaleFactor * (winy - height() / 2);
}
double SleepRoom::viewXToWinX(double viewx) {
    return (viewx - data.player.x - data.view.xOffset) / data.view.scaleFactor + width() / 2;
}
double SleepRoom::viewYToWinY(double viewy) {
    return (viewy - data.player.y - data.view.yOffset) / data.view.scaleFactor + height() / 2;
}

int SleepRoom::viewXToBedX(double viewx) {
    return qFloor((viewx + wBed / 2.0) / (wBed + wSpc));
}
int SleepRoom::viewYToBedY(double viewy) {
    return qFloor((viewy + hBed / 2.0) / (hBed + hSpc));
}
double SleepRoom::bedXToViewX(int bedx) {
    return bedx * (wBed + wSpc);
}
double SleepRoom::bedYToViewY(int bedy) {
    return bedy * (hBed + hSpc);
}

void SleepRoom::initializeGL() {
    if(initializeOpenGLFunctions())
        data.glState = true;
    glClearColor(1, 1, 1, 0);
}
void SleepRoom::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, width(), height());

    glBegin(GL_TRIANGLES);
    glColor3f(1, 0, 0);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glColor3f(0, 1, 0);
    glVertex3f(1.0f, -1.0f, 0.0f);
    glColor3f(0, 0, 1);
    glVertex3f(-1.0f, -1.0f, 0.0f);
    glEnd();
}
void SleepRoom::resizeGL(int w, int h) {
    Q_UNUSED(w)
    Q_UNUSED(h)
}

//void SleepRoom::paintEvent(QPaintEvent *) {
//    QPainter p(this);
//    p.setRenderHint(QPainter::Antialiasing);

//    p.fillRect(rect(), QColor(178, 190, 180));

//    int bx1 = viewXToBedX(winXToViewX(0)), bx2 = viewXToBedX(winXToViewX(width()));
//    int by1 = viewYToBedY(winYToViewY(0)), by2 = viewYToBedY(winYToViewY(height()));

//    // 绘制床
//    p.setCompositionMode(QPainter::CompositionMode_SoftLight);
//    for(int i = bx1; i <= bx2; ++i) {
//        for(int j = by1; j <= by2; ++j) {
//            if(floorMod(i, 2) == floorMod(j, 2))
//                continue;
//            double x = viewXToWinX(bedXToViewX(i)), y = viewYToWinY(bedYToViewY(j));
//            double w = data.asset.pixBedEmpty.width() * data.view.scaleFactor;
//            double h = data.asset.pixBedEmpty.height() * data.view.scaleFactor;
//            QRectF bedRect(x - w / 2, y - h / 2, w, h);
//            p.drawPixmap(bedRect, data.asset.pixBedEmpty, data.asset.pixBedEmpty.rect());
//        }
//    }

////    p.setCompositionMode(QPainter::CompositionMode_Darken);
////    p.setBrush(QColor(207, 221, 209));
////    p.setPen(Qt::NoPen);
////    p.drawRect(rect());
//}
