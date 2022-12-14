#include "sleeproom.h"
#include "ui_overlay.h"

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
    connect(mOverlay->ui->btnChat, &QPushButton::clicked, this, &SleepRoom::onBtnChatClicked);

    data.asset.sleeperImages = {
        { QImage(":/role/src/Boy.png"), QImage(":/bed/src/bedBoy.png") },
        { QImage(":/role/src/Girl.png"), QImage(":/bed/src/bedGirl.png") }
    };

#ifdef Q_OS_ANDROID
    setAttribute(Qt::WA_AcceptTouchEvents);
#endif
}

SleepRoom::~SleepRoom() {
    if(data.glInitialized) {
        data.asset.textureBedEmpty->destroy();
        for(auto &st : data.asset.sleeperTextures) {
            st.bed->destroy();
            st.walk->destroy();
        }
    }
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
    data.player.path.clear();
    data.player.chats.clear();
    data.otherSleeper.clear();
    data.counter = 0;

    data.sortedSleepers.clear();
    data.sortedSleepers << &data.player;
}


QPointF SleepRoom::bedUL(int bx, int by) {
    return QPointF(bedXToViewX(bx) - (wBed + wSpc) / 2.0, bedYToViewY(by) - (hBed + hSpc) / 2.0);
}
QPointF SleepRoom::bedUR(int bx, int by) {
    return QPointF(bedXToViewX(bx) + (wBed + wSpc) / 2.0, bedYToViewY(by) - (hBed + hSpc) / 2.0);
}
QPointF SleepRoom::bedDL(int bx, int by) {
    return QPointF(bedXToViewX(bx) - (wBed + wSpc) / 2.0, bedYToViewY(by) + (hBed + hSpc) / 2.0);
}
QPointF SleepRoom::bedDR(int bx, int by) {
    return QPointF(bedXToViewX(bx) + (wBed + wSpc) / 2.0, bedYToViewY(by) + (hBed + hSpc) / 2.0);
}
double SleepRoom::bedRightEdge(int bx) {
    return bedXToViewX(bx) + wBed / 2.0;
}
double SleepRoom::bedDownEdge(int by) {
    return bedYToViewY(by) + hBed / 2.0;
}

void SleepRoom::pathTo(QList<QPointF> &path, const QPointF &start, QPointF end) {
    if(collisionBed(end.x(), end.y()))
        end = QPointF(bedXToViewX(viewXToBedX(end.x())), bedYToViewY(viewYToBedY(end.y())));

    int startBX = viewXToBedX(start.x()), startBY = viewYToBedY(start.y());
    int endBX = viewXToBedX(end.x()), endBY = viewYToBedY(end.y());

    if(endBX > startBX) {
        if(endBY > startBY) {
            // ??????
            path << bedDR(startBX, startBY);
            pathTo(path, startBX, startBY, endBX - 1, endBY - 1);
            if(isBed(endBX, endBY) && !collisionBed(end.x(), end.y()))
                path << (end.x() < bedRightEdge(endBX) ? bedDL(endBX, endBY) : bedUR(endBX, endBY));
            path << end;
        } else if(endBY < startBY) {
            // ??????
            if(isBed(startBX, startBY) && start.x() < bedRightEdge(startBX))
                path << bedDR(startBX, startBY);
            path << bedUR(startBX, startBY);
            pathTo(path, startBX, startBY - 1, endBX - 1, endBY);
            if(isBed(endBX, endBY) && !collisionBed(end.x(), end.y()) && end.y() < bedDownEdge(endBY))
                path << bedDR(endBX, endBY);
            path << end;
        } else {
            // ???
            if(startBX == endBX - 1 && (
                        (isBed(startBX, startBY) && start.x() >= bedRightEdge(startBX))
                        || collisionBed(end.x(), end.y())
                        )) {
                path << end;
            } else {
                if(start.y() < bedYToViewY(startBY)) {
                    path << bedUR(startBX, startBY);
                    pathTo(path, startBX, startBY - 1, endBX - 1, endBY - 1);
                    if(isBed(endBX, endBY) && !collisionBed(end.x(), end.y()))
                        path << (end.x() < bedRightEdge(endBX) ? bedDL(endBX, endBY) : bedUR(endBX, endBY));
                    path << end;
                } else {
                    path << bedDR(startBX, startBY);
                    pathTo(path, startBX, startBY, endBX - 1, endBY);
                    if(isBed(endBX, endBY) && !collisionBed(end.x(), end.y()) && end.y() < bedDownEdge(endBY))
                        path << bedDR(endBX, endBY);
                    path << end;
                }
            }
        }
    } else if(endBX < startBX) {
        if(endBY > startBY) {
            // ??????
            if(isBed(startBX, startBY) && start.y() < bedDownEdge(startBY))
                path << bedDR(startBX, startBY);
            path << bedDL(startBX, startBY);
            pathTo(path, startBX - 1, startBY, endBX, endBY - 1);
            if(isBed(endBX, endBY) && !collisionBed(end.x(), end.y()) && end.x() < bedRightEdge(endBX))
                path << bedDR(endBX, endBY);
            path << end;
        } else if(endBY < startBY) {
            // ??????
            if(isBed(startBX, startBY))
                path << (start.x() < bedRightEdge(startBX) ? bedDL(startBX, startBY) : bedUR(startBX, startBY));
            path << bedUL(startBX, startBY);
            pathTo(path, startBX - 1, startBY - 1, endBX, endBY);
            path << end;
        } else {
            // ???
            if(startBX == endBX + 1 && isBed(endBX, endBY) && end.y() < bedDownEdge(endBY)) {
                path << end;
            } else {
                if(start.y() < bedYToViewY(startBY)) {
                    if(isBed(startBX, startBY))
                        path << bedUR(startBX, startBY);
                    path << bedUL(startBX, startBY);
                    pathTo(path, startBX - 1, startBY - 1, endBX, endBY - 1);
                    if(isBed(endBX, endBY) && !collisionBed(end.x(), end.y()) && end.x() < bedRightEdge(endBX))
                        path << bedDR(endBX, endBY);
                    path << end;
                } else {
                    if(isBed(startBX, startBY) && start.y() < bedDownEdge(startBY))
                        path << bedDR(startBX, startBY);
                    path << bedDL(startBX, startBY);
                    pathTo(path, startBX - 1, startBY, endBX, endBY);
                    path << end;
                }
            }
        }
    } else {
        if(endBY > startBY) {
            // ???
            if(startBY == endBY - 1 && (
                        (isBed(startBX, startBY) && start.y() >= bedDownEdge(startBY))
                        || collisionBed(end.x(), end.y())
                        )) {
                path << end;
            } else {
                if(start.x() < bedXToViewX(startBX)) {
                    path << bedDL(startBX, startBY);
                    pathTo(path, startBX - 1, startBY, endBX - 1, endBY - 1);
                    if(isBed(endBX, endBY) && !collisionBed(end.x(), end.y()))
                        path << (end.x() < bedRightEdge(endBX) ? bedDL(endBX, endBY) : bedUR(endBX, endBY));
                    path << end;
                } else {
                    path << bedDR(startBX, startBY);
                    pathTo(path, startBX, startBY, endBX, endBY - 1);
                    if(isBed(endBX, endBY) && !collisionBed(end.x(), end.y()) && end.x() < bedRightEdge(endBX))
                        path << bedDR(endBX, endBY);
                    path << end;
                }
            }
        } else if(endBY < startBY) {
            // ???
            if(startBY == endBY + 1 && isBed(endBX, endBY) && end.x() < bedRightEdge(endBX)) {
                path << end;
            } else {
                if(start.x() < bedXToViewX(startBX)) {
                    if(isBed(startBX, startBY))
                        path << bedDL(startBX, startBY);
                    path << bedUL(startBX, startBY);
                    pathTo(path, startBX - 1, startBY - 1, endBX - 1, endBY);
                    if(isBed(endBX, endBY) && !collisionBed(end.x(), end.y()) && endBY < bedDownEdge(endBY))
                        path << bedDR(endBX, endBY);
                    path << end;
                } else {
                    if(isBed(startBX, startBY) && start.x() < bedRightEdge(startBX))
                        path << bedDR(startBX, startBY);
                    path << bedUR(startBX, startBY);
                    pathTo(path, startBX, startBY - 1, endBX, endBY);
                    path << end;
                }
            }
        } else {
            // ???
            if(isBed(startBX, startBY) && !collisionBed(end.x(), end.y())) {
                bool flag1 = start.x() < bedRightEdge(startBX) && end.y() < bedDownEdge(startBY);
                bool flag2 = start.y() < bedDownEdge(startBY) && end.x() < bedRightEdge(startBX);
                if(flag1 || flag2)
                    path << bedDR(startBX, startBY);
            }
            path << end;
        }
    }
}
void SleepRoom::pathTo(QList<QPointF> &path, int xStart, int yStart, int xEnd, int yEnd) {
    if(xEnd > xStart) {
        if((yEnd > yStart && isBed(xStart, yStart)) || (yEnd < yStart && !isBed(xStart, yStart))) {
            ++xStart;
            path << bedDR(xStart, yStart);
        }
    } else if(xEnd < xStart) {
        if((yEnd > yStart && !isBed(xStart, yStart)) || (yEnd < yStart && isBed(xStart, yStart))) {
            --xStart;
            path << bedDR(xStart, yStart);
        }
    }
    int step = qMin(qAbs(xEnd - xStart), qAbs(yEnd - yStart));
    xStart += (xEnd > xStart ? step : -step);
    yStart += (yEnd > yStart ? step : -step);
    path << bedDR(xStart, yStart);
    if((xEnd != xStart && yEnd == yStart) || (xEnd == xStart && yEnd != yStart))
        path << bedDR(xEnd, yEnd);
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
//#ifdef Q_OS_ANDROID
//    if(mTouchHolding[0] && mTouchHolding[1])
//        return;
//#endif
    if(!(ev->buttons() & Qt::LeftButton))
        return;

    mMousePrev = ev->pos();
    clickFlag = true;
}
void SleepRoom::mouseMoveEvent(QMouseEvent *ev) {
#ifdef Q_OS_ANDROID
    if(mTouchHolding[0] && mTouchHolding[1])
        return;
#endif
    if(!(ev->buttons() & Qt::LeftButton))
        return;


    int dx = ev->x() - mMousePrev.x();
    int dy = ev->y() - mMousePrev.y();
    if(abs(dx) > 16 || abs(dy) > 16)
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
#ifdef Q_OS_ANDROID
    mTouchHolding[0] = false;
    mTouchHolding[1] = false;
    if(mBlockRelease) {
        mBlockRelease = false;
        return;
    }
#endif

    if(ev->button() == Qt::LeftButton && clickFlag) {
        if(collisionBed(data.player.x, data.player.y)) {
            double bedCenter = bedXToViewX(viewXToBedX(data.player.x));
            if(data.player.inBed) {
                double target = bedCenter - wBed / 2.0 - wSpc - 2;
                double wLimit = qMax(1, width() - 80) / data.view.adjustedScaleFactor / 2;
                data.view.xOffset = qBound<double>(-wLimit, data.view.xOffset + data.player.x - target, wLimit);
                data.player.x = target;
            }
            emit sPos(data.player.x, data.player.y);
        }
        if(!data.player.inBed) {
            double viewx = winXToViewX(ev->x()), viewy = winYToViewY(ev->y());
            data.player.path.clear();
            pathTo(data.player.path, QPointF(data.player.x, data.player.y), QPointF(viewx, viewy));
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

#ifdef Q_OS_ANDROID
bool SleepRoom::event(QEvent *ev) {
    switch (ev->type()) {
    case QEvent::TouchBegin:
        return touchBeginEventProcess((QTouchEvent*)ev);
    case QEvent::TouchUpdate:
        return touchUpdateEventProcess((QTouchEvent*)ev);
    default:;
    }
    return QOpenGLWidget::event(ev);
}

bool SleepRoom::touchBeginEventProcess(QTouchEvent *ev) {
    const QList<QEventPoint> &points = ev->points();
    for(const QEventPoint &point : points) {
        if(point.id() > 1)
            continue;
        mTouchHolding[point.id()] = true;
        mTouchPos[point.id()] = point.position();
    }

    if(!mTouchHolding[0] || !mTouchHolding[1])
        return false;

    mBlockRelease = true;
    mTouchDis = qSqrt(sqr(mTouchPos[0].x() - mTouchPos[1].x()) + sqr(mTouchPos[0].y() - mTouchPos[1].y()));

    ev->accept();
    return true;
}
bool SleepRoom::touchUpdateEventProcess(QTouchEvent *ev) {
    const QList<QEventPoint> &points = ev->points();
    for(const QEventPoint &point : points) {
        if(point.id() > 1)
            continue;
        mTouchPos[point.id()] = point.position();
    }

    double dis = qSqrt(sqr(mTouchPos[0].x() - mTouchPos[1].x()) + sqr(mTouchPos[0].y() - mTouchPos[1].y()));
    data.view.scaleFactor = qBound<double>(-2.1, data.view.scaleFactor + (dis - mTouchDis) / 100, 1.8);
    data.view.adjustedScaleFactor = qPow(2, data.view.scaleFactor);
    mTouchDis = dis;

    ev->accept();
    return true;
}
bool SleepRoom::touchEndEventProcess(QTouchEvent *ev) {
    const QList<QEventPoint> &points = ev->points();
    for(const QEventPoint &point : points) {
        if(point.id() > 1)
            continue;
        mTouchHolding[point.id()] = false;
    }

    ev->accept();
    return true;
}
#endif

void SleepRoom::onCommand(const QString &command, const QString &content) {
    if(command == "tp") {
        bool found = false;
        for(const Sleeper &sleeper : qAsConst(data.otherSleeper)) {
            if(sleeper.name == content) {
                QPointF pos = bedDR(viewXToBedX(sleeper.x), viewYToBedY(sleeper.y));
                data.player.x = pos.x();
                data.player.y = pos.y();
                data.player.inBed = false;
                data.player.path.clear();
                data.view.xOffset = 0;
                data.view.yOffset = 0;
                emit sPos(data.player.x, data.player.y);
                found = true;
                break;
            }
        }
        if(!found)
            data.player.chats << Sleeper::Chat{ "/????????????:???????????????/", 5000 };
        return;
    }
    if(command == "animScaleFromFar") {
        QPropertyAnimation *anim = new QPropertyAnimation(this, "scaleFactor");
        int duration = content.toInt();
        anim->setDuration(duration <= 0 ? 2000 : qBound(0, duration, 1000 * 60));
        anim->setStartValue(-4.0);
        anim->setEndValue(1.2);
        anim->start(QPropertyAnimation::DeleteWhenStopped);
        return;
    }
    data.player.chats << Sleeper::Chat{ "/????????????:????????????/", 5000 };
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
    iter->path.clear();
    pathTo(iter->path, QPointF(iter->x, iter->y), QPointF(x, y));
}
void SleepRoom::onSleep(qulonglong id, int bx, int by) {
    if(id == data.player.id) {
        data.player.bedX = bx;
        data.player.bedY = by;
        data.player.inBed = true;
        data.player.path.clear();

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
            iter->path.clear();
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
    Sleeper &sleeper = data.otherSleeper[id] = Sleeper{ name, qBound(0, role, (int)data.asset.sleeperImages.length() - 1), id, x, y, bx, by, inBed, {}, {} };
    data.sortedSleepers << &sleeper;
}
void SleepRoom::onChat(qulonglong id, const QString &str) {
    if(id == data.player.id) {
        data.player.chats << Sleeper::Chat{ str, 5000 };
        data.chatStack.insert(0, "[" + data.player.name + "] " + str);
    } else {
        auto iter = data.otherSleeper.find(id);
        if(iter == data.otherSleeper.end())
            return;
        iter->chats << Sleeper::Chat{ str, 5000 };
        if(qSqrt(sqr(iter->x - data.player.x) + sqr(iter->y - data.player.y)) < 4096)
            data.chatStack.insert(0, "[" + iter->name + "] " + str);
    }
    if(data.chatStack.length() > 14) {
        data.chatStack = data.chatStack.mid(0, 14);
    }
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

void SleepRoom::onBtnChatClicked() {
    QString str = mOverlay->ui->editChat->text();
    if(str.isEmpty())
        return;
    mOverlay->ui->editChat->clear();
    // ???????????????
    if(str.startsWith('/')) {
        int index = (int)str.indexOf(' ');
        if(index == -1) {
            onCommand(str.mid(1), "");
        } else {
            onCommand(str.mid(1, index - 1), str.mid(index + 1).trimmed());
        }
        return;
    }
    emit sChat(str);
}

void SleepRoom::onTimerTimeout() {
    data.counter++;

    // ??????????????????Timeout?????????
    int elapsed = qMax(0, (int)mElapsedTimer.elapsed());
    mElapsedTimer.start();
    double step = (double)elapsed / 4.5;

    // ??? data.sortedSleepers ???y??????????????????
    std::stable_sort(data.sortedSleepers.begin(), data.sortedSleepers.end(), [](Sleeper *a, Sleeper *b) {
        return a->y < b->y;
    });

    // ??????Chat???timer
    for(Sleeper *sleeper : qAsConst(data.sortedSleepers)) {
        for(auto iter = sleeper->chats.begin(); iter != sleeper->chats.end(); ++iter) {
            iter->timer -= elapsed;
        }
        while(!sleeper->chats.isEmpty() && sleeper->chats[0].timer <= 0) {
            sleeper->chats.removeFirst();
        }
        if(sleeper->chats.length() > 4)
            sleeper->chats = sleeper->chats.mid(sleeper->chats.length() - 4);
    }

    // ????????????
    if(!data.player.path.isEmpty()) {
        bool sendSleep = false;
        int bx = viewXToBedX(data.player.x);
        int by = viewYToBedY(data.player.y);
        if(doPath(data.player.path, data.player.x, data.player.y, step) && isBed(bx, by))
            sendSleep = true;
        if(data.player.path.isEmpty())
            emit sPos(data.player.x, data.player.y);
        if(sendSleep)
            emit sSleep(bx, by);
    }
    // ????????????????????????
    for(Sleeper &sleeper : data.otherSleeper) {
        if(!sleeper.path.isEmpty())
            doPath(sleeper.path, sleeper.x, sleeper.y, step);
    }

    // ????????????
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


void SleepRoom::paintTexture(QOpenGLTexture *texture, const QPointF &pos, const QPointF &orig, const QSizeF &scale, double rotation) {
    texture->bind(0);
    glUniform2f(shader.locationTexPos, (float)winXToGLX(pos.x()), (float)winYToGLY(pos.y()));
    glUniform2f(shader.locationTexOrig, (float)orig.x(), -(float)orig.y());
    glUniform2f(shader.locationTexSize, (float)texture->width(), -(float)texture->height());
    glUniform2f(shader.locationTexScale, (float)scale.width(), (float)scale.height());
    glUniform1f(shader.locationRotation, (float)rotation);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void SleepRoom::initializeGL() {
    initializeOpenGLFunctions();

    GLfloat vert[] = {
        //??????                   ????????????
        1.0f,  1.0f, 0.0f,      1.0f, 1.0f,
        1.0f, -1.0f, 0.0f,      1.0f, 0.0f,
       -1.0f, -1.0f, 0.0f,      0.0f, 0.0f,
       -1.0f,  1.0f, 0.0f,      0.0f, 1.0f
    };

    // ????????????
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    shader.program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/src/shaderImage.vsh");
    shader.program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/src/shaderImage.fsh");
    shader.program.link();

    glGenVertexArrays(1, &shader.vao);
    glGenBuffers(1, &shader.vbo);

    //??????VAO??????????????????????????????
    glBindVertexArray(shader.vao);

    glBindBuffer(GL_ARRAY_BUFFER, shader.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);

    GLuint location;

    // ??????Attrib
    location = (GLuint)glGetAttribLocation(shader.program.programId(), "in_Position");
    glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(location);
    location = (GLuint)glGetAttribLocation(shader.program.programId(), "in_TexCoord");
    glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(location);

    //?????????????????????
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //??????????????????
    glGenBuffers(1, &shader.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shader.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

//    shader.program.setUniformValue("u_vBlendColor", 0.80f, 0.85f, 0.81f, 1.0f);
    shader.locationWindowSize = glGetUniformLocation(shader.program.programId(), "u_vWindowSize");
    shader.locationTexPos = glGetUniformLocation(shader.program.programId(), "u_vTexPos");
    shader.locationTexOrig = glGetUniformLocation(shader.program.programId(), "u_vTexOrig");
    shader.locationTexSize = glGetUniformLocation(shader.program.programId(), "u_vTexSize");
    shader.locationTexScale = glGetUniformLocation(shader.program.programId(), "u_vTexScale");
    shader.locationRotation = glGetUniformLocation(shader.program.programId(), "u_fRotation");

    shader.program.bind();
    shader.program.setUniformValue("u_sTexImg", 0);
    shader.program.setUniformValue(shader.locationWindowSize, (float)width(), (float)height());

    data.asset.textureBedEmpty = new QOpenGLTexture(QImage(":/bed/src/bedEmpty.png"));
    data.asset.sleeperTextures.reserve(data.asset.sleeperImages.length());
    for(const auto &image : qAsConst(data.asset.sleeperImages)) {
        data.asset.sleeperTextures.append(SleeperTexture{ new QOpenGLTexture(image.walk), new QOpenGLTexture(image.bed) });
    }
    data.asset.sleeperTextures = {
        {new QOpenGLTexture(QImage(":/role/src/Boy.png")), new QOpenGLTexture(QImage(":/bed/src/bedBoy.png"))},
        {new QOpenGLTexture(QImage(":/role/src/Girl.png")), new QOpenGLTexture(QImage(":/bed/src/bedGirl.png"))}
    };

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glClearColor(0.70f, 0.75f, 0.71f, 1.0f);

    data.glInitialized = true;
}
void SleepRoom::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(shader.vao);
    shader.program.bind();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    {   // ?????????
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
                paintTexture(
                    texture, QPointF(x, y), QPointF(), QSizeF(data.view.adjustedScaleFactor, data.view.adjustedScaleFactor), 0
                );
            }
        }
    }

    // ????????????
    for(const Sleeper *sleeper : qAsConst(data.sortedSleepers)) {
        if(sleeper->inBed)
            continue;
        paintTexture(
            data.asset.sleeperTextures[sleeper->role].walk, QPointF(viewXToWinX(sleeper->x), viewYToWinY(sleeper->y)), QPointF(0, 60),
            sleeper->path.isEmpty()
                ? QSizeF(data.view.adjustedScaleFactor, data.view.adjustedScaleFactor * (qSin(data.counter / 50.0) / 40 + 0.95))
                : QSizeF(data.view.adjustedScaleFactor, data.view.adjustedScaleFactor),
            sleeper->path.isEmpty() ? 0 : 0.18 * qSin(data.counter / 5.5)
        );
    }

}

void SleepRoom::resizeGL(int w, int h) {
    shader.program.setUniformValue(shader.locationWindowSize, (float)w, (float)h);
}

void SleepRoom::onPaint(QPainter *p) {
    p->setRenderHint(QPainter::Antialiasing);

    QFont font;

    // ???????????????????????????
    QRectF winRect = rect();
    font.setPointSize(10);
    QFontMetrics fm(font);
    p->setFont(font);
    for(const Sleeper *sleeper : qAsConst(data.sortedSleepers)) {
        QPointF pos;
        if(sleeper->inBed) {
            pos = QPointF(
                        viewXToWinX(bedXToViewX(sleeper->bedX)),
                        viewYToWinY(bedYToViewY(sleeper->bedY) - hBed * 0.4)
                        );
        } else {
            pos = QPointF(viewXToWinX(sleeper->x), viewYToWinY(sleeper->y - 96));
        }

        // ????????????
        QSizeF size = fm.boundingRect(sleeper->name).size() + QSizeF(14, 10);
        QRectF nameRect(pos - QPointF(size.width() / 2.0, size.height()), size);
        pos.ry() -= size.height() + 2;
        if(winRect.contains(nameRect) || winRect.intersects(nameRect)) {
            p->setPen(Qt::NoPen);
            p->setBrush(QColor(0, 0, 0, 128));
            p->drawRect(nameRect);
            p->setPen(Qt::white);
            p->drawText(nameRect, Qt::AlignCenter, sleeper->name);
        }

        // ????????????
        for(auto iter = sleeper->chats.crbegin(); iter != sleeper->chats.crend(); ++iter) {
            const Sleeper::Chat &chat = *iter;
            QSizeF size = fm.boundingRect(chat.str).size() + QSizeF(10, 6);
            QRectF strRect(pos - QPointF(size.width() / 2.0, size.height()), size);
            pos.ry() -= size.height() + 2;
            if(!winRect.contains(strRect) && !winRect.intersects(strRect))
                continue;

            p->setPen(Qt::black);
            p->setBrush(chat.str.startsWith('/') ? QColor(255, 228, 231) : QColor(238, 255, 241, 200));
            p->drawRect(strRect);
            p->setPen(Qt::black);
            p->drawText(strRect, Qt::AlignCenter, chat.str);
        }
    }

    // ????????????
    if(!data.player.path.isEmpty()) {
        p->setPen(QPen(QColor(255, 255, 255, 100), 2));
        QPointF prev(viewXToWinX(data.player.x), viewYToWinY(data.player.y + 20));
        for(const QPointF &pos : qAsConst(data.player.path)) {
            QPointF cur(viewXToWinX(pos.x()), viewYToWinY(pos.y() + 20));
            p->drawLine(prev, cur);
            prev = cur;
        }
    }

    // ??????????????????
    if(mOverlay->ui->btnHistory->isChecked()) {
        QPoint dl = mOverlay->ui->editChat->geometry().topLeft();
        dl.ry() -= 4;
        p->setBrush(QColor(0, 0, 0, 128));
        QSize size(
            mOverlay->ui->editChat->width(),
            fm.height() + 4);
        for(const QString &str : qAsConst(data.chatStack)) {
            dl.ry() -= size.height();
            QRect rect(dl, size);

            p->setPen(Qt::NoPen);
            p->drawRect(rect);
            p->setPen(Qt::white);
            p->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, str);
        }
    }
}
