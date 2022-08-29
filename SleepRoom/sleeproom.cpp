#include "sleeproom.h"

SleepRoom::SleepRoom(QWidget *parent)
    : QOpenGLWidget(parent), mTimer(new QTimer(this))
{
    mTimer->setTimerType(Qt::TimerType::PreciseTimer);
    mTimer->start(17);
    connect(mTimer, &QTimer::timeout, this, &SleepRoom::onTimerTimeout);
}

SleepRoom::~SleepRoom() {

}

int SleepRoom::floorMod(int val, int mod) {
    return val >= 0 ? val % mod : ((-val) % mod == 0 ? 0 : (mod - (-val) % mod));
}
double SleepRoom::sqr(double v) { return v * v; }

void SleepRoom::setName(const QString &name) { mName = name; }
void SleepRoom::setRole(int role) { mRole = role; }
void SleepRoom::setId(qulonglong id) { mId = id; }

void SleepRoom::clear() {
    mName = "";
    mRole = 0;
    mId = 0;

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
}

// 这段真的是大粪，不予置评
QList<QPointF> SleepRoom::pathTo(const QPointF &start, QPointF end) {
    int startBX = viewXToBedX(start.x()), startBY = viewYToBedY(start.y());
    int endBX = viewXToBedX(end.x()), endBY = viewYToBedY(end.y());

    bool insideBed = collisionBed(end.x(), end.y());
    if(floorMod(endBX, 2) != floorMod(endBY, 2) && insideBed)
        end = QPointF(bedXToViewX(endBX), bedYToViewY(endBY));

    if((startBX == endBX && qAbs(startBY - endBY) <= 1) || (qAbs(startBX - endBX) <= 1 && startBY == endBY))
//    if(startBX == endBX && startBY == endBY)
        return { end };

    QList<QPointF> path;
    if(endBX > startBX) {
        if(endBY < startBY) {
            // 右上方

            path << QPointF(bedXToViewX(startBX) + wBed / 2.0 + wSpc, bedYToViewY(startBY) - hBed / 2.0);
            ++startBX;
            --startBY;
            if(startBX != endBX && startBY != endBY && floorMod(startBX, 2) != floorMod(startBY, 2)) {
                path << QPointF(bedXToViewX(startBX) + wBed / 2.0 + wSpc, bedYToViewY(startBY) + hBed / 2.0);
                ++startBX;
            }
            if(startBX != endBX) {
                int steps = qMin(endBX - startBX, startBY - endBY);
                startBX += steps;
                startBY -= steps;
                path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
            }
            path << QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
            path << end;
        }
        else if(endBY > startBY) {
            // 右下方

            path << QPointF(bedXToViewX(startBX) + wBed / 2.0 + wSpc, bedYToViewY(startBY) + hBed / 2.0);
            ++startBX;
            ++startBY;
            if(startBX != endBX && startBY != endBY && floorMod(startBX, 2) != floorMod(startBY, 2)) {
                path << QPointF(bedXToViewX(startBX) + wBed / 2.0 + wSpc, bedYToViewY(startBY) - hBed / 2.0);
                ++startBX;
            }
            if(startBX != endBX) {
                int steps = qMin(endBX - startBX, endBY - startBY);
                startBX += steps;
                startBY += steps;
                path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
            }
            path << QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) - hBed / 2.0);
            path << end;
        } else {
            // 右方
            if(start.y() < bedYToViewY(startBY)) {
                path << QPointF(bedXToViewX(startBX) + wBed / 2.0 + wSpc, bedYToViewY(startBY) - hBed / 2.0);
                path << QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) - hBed / 2.0);
                path << end;
            } else {
                path << QPointF(bedXToViewX(startBX) + wBed / 2.0 + wSpc, bedYToViewY(startBY) + hBed / 2.0);
                path << QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
                path << end;
            }
        }
    } else if(endBX < startBX) {
        if(endBY < startBY) {
            // 左上方

            path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
            --startBX;
            --startBY;
            if(startBX != endBX && startBY != endBY && floorMod(startBX, 2) != floorMod(startBY, 2)) {
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

            path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
            --startBX;
            ++startBY;
            if(startBX != endBX && startBY != endBY && floorMod(startBX, 2) != floorMod(startBY, 2)) {
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
            path << end;
        } else {
            // 左方
            if(start.y() < bedYToViewY(startBY)) {
                path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
                path << QPointF(bedXToViewX(endBX) + wBed / 2.0 + wSpc, bedYToViewY(endBY) - hBed / 2.0);
                path << end;
            } else {
                path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
                path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
                path << end;
            }
        }
    } else {
        if(endBY < startBY) {
            // 上方
            if(start.x() < bedXToViewX(startBX)) {
                path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
                path << QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
                path << end;
            } else {
                path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) - hBed / 2.0);
                path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) + hBed / 2.0);
                path << end;
            }
        } else {
            // 下方
            if(start.x() < bedXToViewX(startBX)) {
                path << QPointF(bedXToViewX(startBX) - wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
                path << QPointF(bedXToViewX(endBX) - wBed / 2.0, bedYToViewY(endBY) - hBed / 2.0);
                path << end;
            } else {
                path << QPointF(bedXToViewX(startBX) + wBed / 2.0, bedYToViewY(startBY) + hBed / 2.0);
                path << QPointF(bedXToViewX(endBX) + wBed / 2.0, bedYToViewY(endBY) - hBed / 2.0);
                path << end;
            }
        }
    }
    return path;
}
bool SleepRoom::doPath(QList<QPointF> &path, double &a, double &b) {
    if(path.isEmpty())
        return false;
    double step = 4;
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
    data.view.scaleFactor = qBound<double>(-2.1, data.view.scaleFactor + (ev->delta() > 0 ? 0.15 : -0.15), 1.8);
    data.view.adjustedScaleFactor = qPow(2, data.view.scaleFactor);
}

void SleepRoom::onPos(qulonglong id, double x, double y) {

}
void SleepRoom::onMove(qulonglong id, double x, double y) {

}
void SleepRoom::onSleep(qulonglong id, int x, int y) {
    if(id == mId) {
        data.player.bedX = x;
        data.player.bedY = y;
        data.player.inBed = true;

        double bedCenterX = bedXToViewX(x), bedCenterY = bedYToViewY(y);
        double wLimit = qMax(1, width() - 80) / data.view.adjustedScaleFactor / 2;
        double hLimit = qMax(1, height() - 80) / data.view.adjustedScaleFactor / 2;
        data.view.xOffset = qBound<double>(-wLimit, data.view.xOffset + data.player.x - bedCenterX, wLimit);
        data.view.yOffset = qBound<double>(-hLimit, data.view.yOffset + data.player.y - bedCenterY, hLimit);
        data.player.x = bedCenterX;
        data.player.y = bedCenterY;
    } else {
        // TODO
    }
}

void SleepRoom::onTimerTimeout() {
    data.counter++;

    if(!data.player.path.isEmpty()) {
        if(doPath(data.player.path, data.player.x, data.player.y)) {
            int bx = viewXToBedX(data.player.x);
            int by = viewYToBedY(data.player.y);
            if(floorMod(bx, 2) != floorMod(by, 2)) {
//                data.player.bedX = bx;
//                data.player.bedY = by;
//                data.player.inBed = true;
                emit sSleep(bx, by);
            }
        }
        if(data.player.path.isEmpty()) {
            emit sPos(data.player.x, data.player.y);
        }
    }
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

bool SleepRoom::collisionBed(double viewx, double viewy) {
    int bx = viewXToBedX(viewx), by = viewYToBedY(viewy);
    if(floorMod(bx, 2) == floorMod(by, 2))
        return false;
    double dx = viewx + wBed / 2.0 - bx * (wBed + wSpc);
    double dy = viewy + hBed / 2.0 - by * (hBed + hSpc);
    return dx < wBed && dy < hBed;
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
    initializeOpenGLFunctions();

    data.asset.textureBedEmpty = new QOpenGLTexture(QImage(":/bed/src/bedEmpty.png"));
    data.asset.sleeperTextures = {
        {new QOpenGLTexture(QImage(":/role/src/Boy.png")), new QOpenGLTexture(QImage(":/bed/src/bedBoy.png"))},
        {new QOpenGLTexture(QImage(":/role/src/Girl.png")), new QOpenGLTexture(QImage(":/bed/src/bedGirl.png"))}
    };

    glClearColor(0.70f, 0.75f, 0.71f, 1.0f);

//    data.glInitialized = true;
}
void SleepRoom::paintGL() {
//    QRect winRect = rect();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.beginNativePainting();

    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, width(), height());
    glLoadIdentity();

    if(data.asset.textureBedEmpty) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);

        // 绘制床
        int bx1 = viewXToBedX(winXToViewX(0)), bx2 = viewXToBedX(winXToViewX(width()));
        int by1 = viewYToBedY(winYToViewY(0)), by2 = viewYToBedY(winYToViewY(height()));
        for(int i = bx1; i <= bx2; ++i) {
            for(int j = by1; j <= by2; ++j) {
                if(floorMod(i, 2) == floorMod(j, 2))
                    continue;
                QOpenGLTexture *texture = data.asset.textureBedEmpty;
                if(data.player.inBed && data.player.bedX == i && data.player.bedY == j) {
                    texture = data.asset.sleeperTextures[mRole].bed;
                }
                texture->bind();
                double x = viewXToWinX(bedXToViewX(i)), y = viewYToWinY(bedYToViewY(j));
                double w = texture->width() * data.view.adjustedScaleFactor;
                double h = texture->height() * data.view.adjustedScaleFactor;
                glBegin(GL_QUADS);
                textureCoord(QRectF(x - w / 2, y - h / 2, w, h));
                glEnd();
            }
        }

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }

    QOpenGLTexture *roleTexture = data.asset.sleeperTextures[mRole].walk;
    if(!data.player.inBed) {
        if(roleTexture) {
            roleTexture->bind();

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);

            // 绘制玩家
            double w = roleTexture->width() * data.view.adjustedScaleFactor;
            double h = roleTexture->height() * data.view.adjustedScaleFactor;
            textureCoord(QRectF(viewXToWinX(data.player.x) - w / 2, viewYToWinY(data.player.y - 50) - h / 2, w, h));

            glEnd();
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
        }
    }

    p.endNativePainting();

    QFont font;
    font.setPointSize(12);
    p.setFont(font);

    QFontMetrics fm(font);

    // 绘制玩家名称
    if(roleTexture) {
        QSizeF size = fm.boundingRect(mName).size() + QSizeF(10, 10);
        QPointF pos;
        if(data.player.inBed) {
            pos = QPointF(
                        viewXToWinX(bedXToViewX(data.player.bedX)) - size.width() / 2.0,
                        viewYToWinY(bedYToViewY(data.player.bedY) - hBed * 0.4) - size.height()
                        );
        } else {
            pos = QPointF(viewXToWinX(data.player.x) - size.width() / 2.0, viewYToWinY(data.player.y - 115) - size.height());
        }
        QRectF rect(pos, size);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 128));
        p.drawRect(rect);
        p.setPen(Qt::white);
        p.drawText(rect, Qt::AlignCenter, mName);
    }

    // 绘制路线
    if(!data.player.path.isEmpty()) {
        p.setPen(QPen(QColor(255, 255, 255, 100), 2));
        QPointF prev(viewXToWinX(data.player.x), viewYToWinY(data.player.y));
        for(const QPointF &pos : qAsConst(data.player.path)) {
            QPointF cur(viewXToWinX(pos.x()), viewYToWinY(pos.y()));
            p.drawLine(prev, cur);
            prev = cur;
        }
    }
}
void SleepRoom::resizeGL(int w, int h) {
    Q_UNUSED(w)
    Q_UNUSED(h)
}