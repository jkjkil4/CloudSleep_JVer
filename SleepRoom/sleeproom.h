#ifndef SLEEPROOM_H
#define SLEEPROOM_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_1_1>
#include <QOpenGLTexture>
#include <QPainter>
#include <QtMath>
#include <QTimer>
#include <QWheelEvent>

class SleepRoom : public QOpenGLWidget, protected QOpenGLFunctions_1_1
{
    Q_OBJECT
public:
    explicit SleepRoom(QWidget *parent = nullptr);
    ~SleepRoom() override;

    int floorMod(int val, int mod);

protected:
    void wheelEvent(QWheelEvent *ev) override;

public slots:
    void onTimerTimeout();

private:
    QTimer *mTimer;

public:
    double GLXToWinX(double glx);
    double GLYToWinY(double gly);
    double winXToGLX(double winx);
    double winYToGLY(double winy);

    double winXToViewX(double winx);
    double winYToViewY(double winy);
    double viewXToWinX(double viewx);
    double viewYToWinY(double viewy);

    int viewXToBedX(double viewx);
    int viewYToBedY(double viewy);
    double bedXToViewX(int bedx);
    double bedYToViewY(int bedy);

protected:
    void textureCoord(const QRectF &rect);

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    static constexpr int wSpc = 16;
    static constexpr int hSpc = 16;
    static constexpr int wBed = 162;
    static constexpr int hBed = 260;

    struct {
        struct {
            double scaleFactor = 0, adjustedScaleFactor = qPow(2, 0);
            double xOffset, yOffset;
        } view;

        struct {
            double x, y;
        } player;

        struct {
            QImage pixBedEmpty, pixBedBoy, pixBedGirl;
            QOpenGLTexture *textureBedEmpty;
            QVector<QOpenGLTexture *> textureBeds;
        } asset;

        bool glState = false;
    } data;
};

#endif // SLEEPROOM_H
