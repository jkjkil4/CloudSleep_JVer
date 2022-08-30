#include "sleeproom.h"

#include "overlay.h"

SleepRoom::SleepRoom(QWidget *parent)
    : QOpenGLWidget(parent),
      mOverlay(new Overlay(this)),
      mTimer(new QTimer(this))
{
    mTimer->setTimerType(Qt::TimerType::PreciseTimer);
    mTimer->start(17);
    connect(mTimer, &QTimer::timeout, this, &SleepRoom::onTimerTimeout);

    mElapsedTimer.start();

    connect(mOverlay, &Overlay::paint, this, &SleepRoom::onPaint);

    data.asset.sleeperImages = {
        { QImage(":/role/src/Boy.png"), QImage(":/bed/src/bedBoy.png") },
        { QImage(":/role/src/Girl.png"), QImage(":/bed/src/bedGirl.png") }
    };
}

SleepRoom::~SleepRoom() {

}

void SleepRoom::resizeEvent(QResizeEvent *ev) {
    QOpenGLWidget::resizeEvent(ev);
    mOverlay->setGeometry(rect());
}

int SleepRoom::floorMod(int val, int mod) {
    return val >= 0 ? val % mod : ((-val) % mod == 0 ? 0 : (mod - (-val) % mod));
}
double SleepRoom::sqr(double v) { return v * v; }

void SleepRoom::setName(const QString &name) { data.player.name = name; }
void SleepRoom::setRole(int role) { data.player.role = role; }
void SleepRoom::setId(qulonglong id) { data.player.id = id; }

void SleepRoom::clear() {
    data.player.name = "";
    data.player.role = 0;
    data.player.id = 0;

    data.view.scaleFactor = 0;
    data.view.adjustedScaleFactor = qPow(2, 0);
    data.view.xOffset = 0;
    data.view.yOffset = 0;

    data.player.x = 0;
    data.player.y = 0;
    data.player.bedX = 0;
    data.player.bedY = 0;
    data.player.inBed = 0;
    data.otherSleeper.clear();
    data.counter = 0;

    data.sortedSleepers.clear();
    data.sortedSleepers << &data.player;
}

// 这段真的是大粪，不予置评
QList<QPointF> SleepRoom::pathTo(const QPointF &start, QPointF end) {
    int startBX = viewXToBedX(start.x()), startBY = viewYToBedY(start.y());
    int endBX = viewXToBedX(end.x()), endBY = viewYToBedY(end.y());

    bool collisionEnd = collisionBed(end.x(), end.y());
    if(collisionEnd)
        end = QPointF(bedXToViewX(endBX), bedYToViewY(endBY));

    if(startBX == endBX && startBY == endBY)
        return { end };
    bool collisionStart = collisionBed(start.x(), start.y());
    if(collisionStart != collisionEnd && ((startBX == endBX && qAbs(startBY - endBY) <= 1) || (qAbs(startBX - endBX) <= 1 && startBY == endBY)))
        return { end };

    QList<QPointF> path;
    if(endBX > startBX) {
        if(endBY < startBY) {
            // 右上方

            if(!collisionStart && isBed(startBX, startBY) && start.x() < bedXToViewX(startBX) + wBed / 2.0)
                path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
            path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
            ++startBX;
            --startBY;
            if(startBX != endBX && startBY != endBY && isBed(startBX, startBY)) {
                path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
                ++startBX;
            }
            if(startBX != endBX) {
                int steps = qMin(endBX - startBX, startBY - endBY);
                startBX += steps;
                startBY -= steps;
                path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
            }
            path << QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
            if(!collisionEnd && isBed(endBX, endBY) && end.x() >= bedXToViewX(endBX) + wBed / 2.0)
                path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
            path << end;
        }
        else if(endBY > startBY) {
            // 右下方

            path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
            ++startBX;
            ++startBY;
            if(startBX != endBX && startBY != endBY && isBed(startBX, startBY)) {
                path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
                ++startBX;
            }
            if(startBX != endBX) {
                int steps = qMin(endBX - startBX, endBY - startBY);
                startBX += steps;
                startBY += steps;
                path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
            }
            path << QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) - hBed / 2.0);
            if(!collisionEnd && isBed(endBX, endBY))
                path << (end.x() < bedXToViewX(endBX) + wBed / 2.0
                         ? QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0)
                         : QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) - hBed / 2.0));
            path << end;
        } else {
            // 右方
            if(startBX == endBX - 1 && !collisionStart && start.x() >= bedXToViewX(startBX) + wBed / 2.0) {
                path << end;
            } else {
                if(start.y() < bedYToViewY(startBY)) {
                    path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
                    path << QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) - hBed / 2.0);
                    if(!collisionEnd && isBed(endBX, endBY))
                        path << (end.x() < bedXToViewX(endBX) + wBed / 2.0
                                 ? QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0)
                                 : QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) - hBed / 2.0));
                    path << end;
                } else {
                    path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
                    path << QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
                    if(!collisionEnd && isBed(endBX, endBY) && end.x() >= bedXToViewX(endBX) + wBed / 2.0)
                        path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
                    path << end;
                }
            }
        }
    } else if(endBX < startBX) {
        if(endBY < startBY) {
            // 左上方

            if(!collisionStart && isBed(startBX, startBY))
                path << (start.x() < bedXToViewX(startBX) + wBed / 2.0
                         ? QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0)
                         : QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0));
            path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
            --startBX;
            --startBY;
            if(startBX != endBX && startBY != endBY && isBed(startBX, startBY)) {
                path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
                --startBX;
            }
            if(startBX != endBX) {
                int steps = qMin(startBX - endBX, startBY - endBY);
                startBX -= steps;
                startBY -= steps;
                path << QPointF(bedXToViewX(startBX) + wBed / 2.0 + wSpc, bedYToViewY(startBY) + hBed / 2.0);
            }
            path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
            path << end;
        } else if(endBY > startBY) {
            // 左下方

            if(!collisionStart && isBed(startBX, startBY) && start.x() >= bedXToViewX(startBX) + wBed / 2.0)
                path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
            path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
            --startBX;
            ++startBY;
            if(startBX != endBX && startBY != endBY && isBed(startBX, startBY)) {
                path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
                --startBX;
            }
            if(startBX != endBX) {
                int steps = qMin(startBX - endBX, endBY - startBY);
                startBX -= steps;
                startBY += steps;
                path << QPointF(bedXToViewX(startBX) + wBed / 2.0 + wSpc, bedYToViewY(startBY) - hBed / 2.0);
            }
            path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) - hBed / 2.0);
            if(!collisionEnd && isBed(endBX, endBY) && end.x() < bedXToViewX(endBX) + wBed / 2.0)
                path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
            path << end;
        } else {
            // 左方
            if(startBX == endBX + 1 && !collisionStart && end.x() >= bedXToViewX(endBX) + wBed / 2.0) {
                path << end;
            } else {
                if(start.y() < bedYToViewY(startBY)) {
                    if(!collisionStart && isBed(startBX, startBY) && start.x() >= bedXToViewX(startBX) + wBed / 2.0)
                        path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
                    path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
                    path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) - hBed / 2.0);
                    if(!collisionEnd && isBed(endBX, endBY) && end.x() < bedXToViewX(endBX) + wBed / 2.0)
                        path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
                    path << end;
                } else {
                    if(!collisionStart && isBed(startBX, startBY) && start.x() >= bedXToViewX(startBX) + wBed / 2.0)
                        path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
                    path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
                    path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
                    path << end;
                }
            }
        }
    } else {
        if(endBY < startBY) {
            // 上方
            if(startBY == endBY + 1 && !collisionStart && end.y() >= bedYToViewY(endBY) + hBed / 2.0) {
                path << end;
            } else {
                if(start.x() < bedXToViewX(startBX)) {
                    if(!collisionStart && isBed(startBX, startBY) && start.y() >= bedYToViewY(startBY) + hBed / 2.0)
                        path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
                    path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
                    path << QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
                    if(!collisionEnd && isBed(endBX, endBY) && end.y() < bedYToViewY(endBY) + hBed / 2.0)
                        path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
                    path << end;
                } else {
                    if(!collisionStart && isBed(startBX, startBY) && start.y() >= bedYToViewY(startBY) + hBed / 2.0)
                        path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
                    path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
                    path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
                    path << end;
                }
            }
        } else {
            // 下方
            if(startBY == endBY - 1 && !collisionStart && start.y() >= bedYToViewY(startBY) + hBed / 2.0) {
                path << end;
            } else {
                if(start.x() < bedXToViewX(startBX)) {
                    path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
                    path << QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) - hBed / 2.0);
                    if(!collisionEnd && isBed(endBX, endBY))
                        path << (end.y() < bedYToViewY(endBY) + hBed / 2.0
                                 ? QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) - hBed / 2.0)
                                 : QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0));
                    path << end;
                } else {
                    path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
                    path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) - hBed / 2.0);
                    if(!collisionEnd && isBed(endBX, endBY) && end.y() >= bedYToViewY(endBY) + hBed / 2.0)
                        path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
                    path << end;
                }
            }
        }
    }
    return path;
}
bool SleepRoom::doPath(QList<QPointF> &path, double &a, double &b, double step) {
    if(path.isEmpty())
        return false;

    double len = qSqrt(sqr(a - path[0].x()) + sqr(b - path[0].y()));
    while(step > len) {
        step -= len;
        a = path[0].x();
        b = path[0].y();
        path.removeFirst();
        if(path.isEmpty())
            return collisionBed(a, b);
        len = qSqrt(sqr(a - path[0].x()) + sqr(b - path[0].y()));
    }

    double k = step / len;
    a = a * (1 - k) + path[0].x() * k;
    b = b * (1 - k) + path[0].y() * k;

    return false;
}

void SleepRoom::mousePressEvent(QMouseEvent *ev) {
    if(ev->button() != Qt::LeftButton)
        return;

    mMousePrev = ev->pos();
    clickFlag = true;
}
void SleepRoom::mouseMoveEvent(QMouseEvent *ev) {
    if(!(ev->buttons() & Qt::LeftButton))
        return;

    int dx = ev->x() - mMousePrev.x();
    int dy = ev->y() - mMousePrev.y();
    if(abs(dx) > 10 || abs(dy) > 10)
        clickFlag = false;

    if(!clickFlag) {
        double wLimit = qMax(1, width() - 80) / data.view.adjustedScaleFactor / 2;
        double hLimit = qMax(1, height() - 80) / data.view.adjustedScaleFactor / 2;
        data.view.xOffset = qBound<double>(-wLimit, data.view.xOffset - dx / data.view.adjustedScaleFactor, wLimit);
        data.view.yOffset = qBound<double>(-hLimit, data.view.yOffset - dy / data.view.adjustedScaleFactor, hLimit);
        mMousePrev = ev->pos();
    }
}
void SleepRoom::mouseReleaseEvent(QMouseEvent *ev) {
    if(ev->button() == Qt::LeftButton && clickFlag) {
        if(collisionBed(data.player.x, data.player.y)) {
            double bedCenter = bedXToViewX(viewXToBedX(data.player.x));
            if(data.player.inBed || data.player.x < bedCenter) {
                double target = bedCenter - wBed / 2.0 - wSpc - 2;
                if(data.player.inBed) {
                    double wLimit = qMax(1, width() - 80) / data.view.adjustedScaleFactor / 2;
                    data.view.xOffset = qBound<double>(-wLimit, data.view.xOffset + data.player.x - target, wLimit);
                }
                data.player.x = target;
            } else {
                data.player.x = bedCenter + wBed / 2.0 + wSpc + 2;
            }
            emit sPos(data.player.x, data.player.y);
        }
        if(!data.player.inBed) {
            double viewx = winXToViewX(ev->x()), viewy = winYToViewY(ev->y());
            data.player.path = pathTo(QPointF(data.player.x, data.player.y), QPointF(viewx, viewy));
            emit sPos(data.player.x, data.player.y);
            emit sMove(viewx, viewy);
        }
        data.player.inBed = false;
    }
}


void SleepRoom::wheelEvent(QWheelEvent *ev) {
#ifdef Q_OS_ANDROID
    Q_UNUSED(ev)
#else
    data.view.scaleFactor = qBound<double>(-2.1, data.view.scaleFactor + (ev->delta() > 0 ? 0.15 : -0.15), 1.8);
    data.view.adjustedScaleFactor = qPow(2, data.view.scaleFactor);
#endif
}

void SleepRoom::onPos(qulonglong id, double x, double y) {
    auto iter = data.otherSleeper.find(id);
    if(iter == data.otherSleeper.end())
        return;
    iter->path.clear();
    iter->x = x;
    iter->y = y;
    iter->inBed = false;
}
void SleepRoom::onMove(qulonglong id, double x, double y) {
    auto iter = data.otherSleeper.find(id);
    if(iter == data.otherSleeper.end())
        return;
    iter->path = pathTo(QPointF(iter->x, iter->y), QPointF(x, y));
}
void SleepRoom::onSleep(qulonglong id, int bx, int by) {
    if(id == data.player.id) {
        data.player.bedX = bx;
        data.player.bedY = by;
        data.player.inBed = true;

        double bedCenterX = bedXToViewX(bx), bedCenterY = bedYToViewY(by);
        double wLimit = qMax(1, width() - 80) / data.view.adjustedScaleFactor / 2;
        double hLimit = qMax(1, height() - 80) / data.view.adjustedScaleFactor / 2;
        data.view.xOffset = qBound<double>(-wLimit, data.view.xOffset + data.player.x - bedCenterX, wLimit);
        data.view.yOffset = qBound<double>(-hLimit, data.view.yOffset + data.player.y - bedCenterY, hLimit);
        data.player.x = bedCenterX;
        data.player.y = bedCenterY;
    } else {
        auto iter = data.otherSleeper.find(id);
        if(iter != data.otherSleeper.end()) {
            iter->bedX = bx;
            iter->bedY = by;
            iter->inBed = true;
        }
    }
}
void SleepRoom::onBlocked(int bx, int by) {
    int aBX = viewXToBedX(data.player.x), aBY = viewYToBedY(data.player.y);
    if(aBX == bx && aBY == by && collisionBed(data.player.x, data.player.y)) {
        data.player.x = bedXToViewX(bx) - wBed / 2.0 - wSpc - 2;
        emit sPos(data.player.x, data.player.y);
    }
}
void SleepRoom::onSleeper(const QString &name, int role, qulonglong id, double x, double y, int bx, int by, bool inBed) {
    auto iter = data.otherSleeper.constFind(id);
    if(iter != data.otherSleeper.cend())
        return;
    Sleeper &sleeper = data.otherSleeper[id] = Sleeper{ name, qBound(0, role, data.asset.sleeperImages.length() - 1), id, x, y, bx, by, inBed, {} };
    data.sortedSleepers << &sleeper;
}
void SleepRoom::onLeave(qulonglong id) {
    auto iter = data.otherSleeper.find(id);
    if(iter == data.otherSleeper.end())
        return;
    for(auto iter = data.sortedSleepers.begin(); iter != data.sortedSleepers.end(); ++iter) {
        if((*iter)->id == id) {
            data.sortedSleepers.erase(iter);
            break;
        }
    }
    data.otherSleeper.erase(iter);
}

void SleepRoom::onTimerTimeout() {
    data.counter++;

    std::sort(data.sortedSleepers.begin(), data.sortedSleepers.end(), [](Sleeper *a, Sleeper *b) {
        return a->y < b->y;
    });

    int elapsed = qMax(0, (int)mElapsedTimer.elapsed());
    mElapsedTimer.start();
    double step = (double)elapsed / 4.5;

    if(!data.player.path.isEmpty()) {
        bool sendSleep = false;
        int bx = viewXToBedX(data.player.x);
        int by = viewYToBedY(data.player.y);
        if(doPath(data.player.path, data.player.x, data.player.y, step) && isBed(bx, by)) {
//            data.player.bedX = bx;
//            data.player.bedY = by;
//            data.player.inBed = true;
            sendSleep = true;
        }
        if(data.player.path.isEmpty())
            emit sPos(data.player.x, data.player.y);
        if(sendSleep)
            emit sSleep(bx, by);
    }
    for(Sleeper &sleeper : data.otherSleeper) {
        if(!sleeper.path.isEmpty())
            doPath(sleeper.path, sleeper.x, sleeper.y, step);
    }
    update();
    mOverlay->update();
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

bool SleepRoom::collisionBed(double viewx, double viewy) {
    int bx = viewXToBedX(viewx), by = viewYToBedY(viewy);
    if(!isBed(bx, by))
        return false;
    double dx = viewx + wBed / 2.0 - bx * (wBed + wSpc);
    double dy = viewy + hBed / 2.0 - by * (hBed + hSpc);
    return dx <= wBed && dy <= hBed;
}
bool SleepRoom::isBed(int bx, int by) { return floorMod(bx, 2) != floorMod(by, 2); }


void SleepRoom::paintTexture(QOpenGLTexture *texture, const QPointF &pos, const QSizeF &scale, double rotation) {
    texture->bind(0);
    glUniform2f(shader.locationTexPos, (float)winXToGLX(pos.x()), (float)winYToGLY(pos.y()));
    glUniform2f(shader.locationTexSize, (float)(scale.width() * texture->width() / width()), -(float)(scale.height() * texture->height() / height()));
    glUniform1f(shader.locationRotation, (float)rotation);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void SleepRoom::initializeGL() {
    initializeOpenGLFunctions();

    GLfloat vert[] = {
        //位置                   纹理坐标
        1.0f,  1.0f, 0.0f,      1.0f, 1.0f,
        1.0f, -1.0f, 0.0f,      1.0f, 0.0f,
       -1.0f, -1.0f, 0.0f,      0.0f, 0.0f,
       -1.0f,  1.0f, 0.0f,      0.0f, 1.0f
    };

    // 索引数据
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    shader.program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/src/shaderImage.vsh");
    shader.program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/src/shaderImage.fsh");
    shader.program.link();

    glGenVertexArrays(1, &shader.vao);
    glGenBuffers(1, &shader.vbo);

    //使用VAO开始记录数据属性操作
    glBindVertexArray(shader.vao);

    glBindBuffer(GL_ARRAY_BUFFER, shader.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);

    GLuint location;

    // 定义Attrib
    location = (GLuint)glGetAttribLocation(shader.program.programId(), "in_Position");
    glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(location);
    location = (GLuint)glGetAttribLocation(shader.program.programId(), "in_TexCoord");
    glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(location);

    //解除绑定缓存区
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //索引缓存对象
    glGenBuffers(1, &shader.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shader.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    shader.program.setUniformValue("u_sTexImg", 0);
    shader.program.setUniformValue("u_vWindowSize", (float)width(), (float)height());
//    shader.program.setUniformValue("u_vBlendColor", 0.80f, 0.85f, 0.81f, 1.0f);
    shader.locationWindowSize = glGetUniformLocation(shader.program.programId(), "u_vWindowSize");
    shader.locationTexPos = glGetUniformLocation(shader.program.programId(), "u_vTexPos");
    shader.locationTexSize = glGetUniformLocation(shader.program.programId(), "u_vTexSize");
    shader.locationRotation = glGetUniformLocation(shader.program.programId(), "u_fRotation");

    data.asset.textureBedEmpty = new QOpenGLTexture(QImage(":/bed/src/bedEmpty.png"));
    data.asset.sleeperTextures.reserve(data.asset.sleeperImages.length());
    for(const auto &image : qAsConst(data.asset.sleeperImages)) {
        data.asset.sleeperTextures.append({ new QOpenGLTexture(image.walk), new QOpenGLTexture(image.bed) });
    }
    data.asset.sleeperTextures = {
        {new QOpenGLTexture(QImage(":/role/src/Boy.png")), new QOpenGLTexture(QImage(":/bed/src/bedBoy.png"))},
        {new QOpenGLTexture(QImage(":/role/src/Girl.png")), new QOpenGLTexture(QImage(":/bed/src/bedGirl.png"))}
    };

    glClearColor(0.70f, 0.75f, 0.71f, 1.0f);

    data.glInitialized = true;
}
void SleepRoom::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(shader.vao);
    shader.program.bind();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    {   // 绘制床
        int bx1 = viewXToBedX(winXToViewX(0)), bx2 = viewXToBedX(winXToViewX(width()));
        int by1 = viewYToBedY(winYToViewY(0)), by2 = viewYToBedY(winYToViewY(height()));
        for(int i = bx1; i <= bx2; ++i) {
            for(int j = by1; j <= by2; ++j) {
                if(!isBed(i, j))
                    continue;

                QOpenGLTexture *texture = data.asset.textureBedEmpty;
                for(const Sleeper *sleeper : qAsConst(data.sortedSleepers)) {
                    if(sleeper->inBed && sleeper->bedX == i && sleeper->bedY == j) {
                        texture = data.asset.sleeperTextures.value(sleeper->role).bed;
                        break;
                    }
                }
                double x = viewXToWinX(bedXToViewX(i)), y = viewYToWinY(bedYToViewY(j));
                paintTexture(texture, QPointF(x, y), QSizeF(data.view.adjustedScaleFactor, data.view.adjustedScaleFactor), 0);
            }
        }
    }

    // 绘制玩家
    for(const Sleeper *sleeper : qAsConst(data.sortedSleepers)) {
        if(sleeper->inBed)
            continue;
        paintTexture(
            data.asset.sleeperTextures[sleeper->role].walk, QPointF(viewXToWinX(sleeper->x), viewYToWinY(sleeper->y - 50)),
            QSizeF(data.view.adjustedScaleFactor, data.view.adjustedScaleFactor), 0
        );
    }
}

void SleepRoom::resizeGL(int w, int h) {
    shader.program.setUniformValue(shader.locationWindowSize, (float)w, (float)h);
}

void SleepRoom::onPaint(QPainter *p) {
    p->setRenderHint(QPainter::Antialiasing);

    QFont font;
    font.setPointSize(12);
    p->setFont(font);

    QFontMetrics fm(font);

    // 绘制玩家名称
    p->setBrush(QColor(0, 0, 0, 128));
    for(const Sleeper *sleeper : data.sortedSleepers) {
        QSizeF size = fm.boundingRect(sleeper->name).size() + QSizeF(10, 10);
        QPointF pos;
        if(sleeper->inBed) {
            pos = QPointF(
                        viewXToWinX(bedXToViewX(sleeper->bedX)) - size.width() / 2.0,
                        viewYToWinY(bedYToViewY(sleeper->bedY) - hBed * 0.4) - size.height()
                        );
        } else {
            pos = QPointF(viewXToWinX(sleeper->x) - size.width() / 2.0, viewYToWinY(sleeper->y - 115) - size.height());
        }
        QRectF nameRect(pos, size);
        QRectF winRect = rect();
        if(!winRect.contains(nameRect) && !winRect.intersects(nameRect))
            continue;

        p->setPen(Qt::NoPen);
        p->drawRect(nameRect);
        p->setPen(Qt::white);
        p->drawText(nameRect, Qt::AlignCenter, sleeper->name);
    }

    // 绘制路线
    if(!data.player.path.isEmpty()) {
        p->setPen(QPen(QColor(255, 255, 255, 100), 2));
        QPointF prev(viewXToWinX(data.player.x), viewYToWinY(data.player.y));
        for(const QPointF &pos : qAsConst(data.player.path)) {
            QPointF cur(viewXToWinX(pos.x()), viewYToWinY(pos.y()));
            p->drawLine(prev, cur);
            prev = cur;
        }
    }
}
