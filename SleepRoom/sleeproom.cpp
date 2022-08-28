#include "sleeproom.h"
#include <QDebug>

SleepRoom::SleepRoom(QWidget *parent)
    : QOpenGLWidget(parent), mTimer(new QTimer(this))
{
    mTimer->setTimerType(Qt::TimerType::PreciseTimer);
    mTimer->start(20);
    connect(mTimer, &QTimer::timeout, this, &SleepRoom::onTimerTimeout);
}

SleepRoom::~SleepRoom() {
    delete data.asset.textureBedEmpty;
    for(QOpenGLTexture *texture : data.asset.textureBeds) {
        delete texture;
    }
}

int SleepRoom::floorMod(int val, int mod) {
    return val >= 0 ? val % mod : ((-val) % mod == 0 ? 0 : (mod - (-val) % mod));
}

void SleepRoom::wheelEvent(QWheelEvent *ev) {
    data.view.scaleFactor = qBound<double>(-2.1, data.view.scaleFactor + (ev->delta() > 0 ? 0.15 : -0.15), 1.8);
    data.view.adjustedScaleFactor = qPow(2, data.view.scaleFactor);
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
    return data.player.x + data.view.xOffset + (winx - width() / 2.0) / data.view.adjustedScaleFactor;
}
double SleepRoom::winYToViewY(double winy) {
    return data.player.y + data.view.yOffset + (winy - height() / 2.0) / data.view.adjustedScaleFactor;
}
double SleepRoom::viewXToWinX(double viewx) {
    return (viewx - data.player.x - data.view.xOffset) * data.view.adjustedScaleFactor + width() / 2.0;
}
double SleepRoom::viewYToWinY(double viewy) {
    return (viewy - data.player.y - data.view.yOffset) * data.view.adjustedScaleFactor + height() / 2.0;
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

void SleepRoom::textureCoord(const QRectF &rect) {
    glTexCoord2d(1.0, 1.0);
    glVertex3f((float)winXToGLX(rect.x() + rect.width()), (float)winYToGLY(rect.y() + rect.height()), 0.0f);

    glTexCoord2d(0.0, 1.0);
    glVertex3f((float)winXToGLX(rect.x()), (float)winYToGLY(rect.y() + rect.height()), 0.0f);

    glTexCoord2d(0.0, 0.0);
    glVertex3f((float)winXToGLX(rect.x()), (float)winYToGLY(rect.y()), 0.0f);

    glTexCoord2d(1.0, 0.0);
    glVertex3f((float)winXToGLX(rect.x() + rect.width()), (float)winYToGLY(rect.y()), 0.0f);
}

void SleepRoom::initializeGL() {
    if(initializeOpenGLFunctions())
        data.glState = true;

    data.asset.textureBedEmpty = new QOpenGLTexture(QImage(":/bed/src/bedEmpty.png"));
    data.asset.textureBeds = {
        new QOpenGLTexture(QImage(":/bed/src/bedBoy.png")),
        new QOpenGLTexture(QImage(":/bed/src/bedGirl.png"))
    };

    glClearColor(0.80f, 0.85f, 0.81f, 1.0f);
}
void SleepRoom::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, width(), height());
    glLoadIdentity();

    if(data.asset.textureBedEmpty) {
        data.asset.textureBedEmpty->bind();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);

        // 绘制床
        int bx1 = viewXToBedX(winXToViewX(0)), bx2 = viewXToBedX(winXToViewX(width()));
        int by1 = viewYToBedY(winYToViewY(0)), by2 = viewYToBedY(winYToViewY(height()));
        for(int i = bx1; i <= bx2; ++i) {
            for(int j = by1; j <= by2; ++j) {
                if(floorMod(i, 2) == floorMod(j, 2))
                    continue;
                double x = viewXToWinX(bedXToViewX(i)), y = viewYToWinY(bedYToViewY(j));
                double w = data.asset.textureBedEmpty->width() * data.view.adjustedScaleFactor;
                double h = data.asset.textureBedEmpty->height() * data.view.adjustedScaleFactor;
                QRectF bedRect(x - w / 2, y - h / 2, w, h);
                textureCoord(bedRect);
            }
        }

        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }
}
void SleepRoom::resizeGL(int w, int h) {
    Q_UNUSED(w)
    Q_UNUSED(h)
}
