#ifndef SLEEPROOM_H
#define SLEEPROOM_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_1_1>
#include <QPainter>
#include <QtMath>
#include <QTimer>

class SleepRoom : public QOpenGLWidget, protected QOpenGLFunctions_1_1
{
    Q_OBJECT
public:
    explicit SleepRoom(QWidget *parent = nullptr);

    int floorMod(int val, int mod);

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
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
//    void paintEvent(QPaintEvent *) override;

private:
    static constexpr int wSpc = 10;
    static constexpr int hSpc = 10;
    static constexpr int wBed = 162;
    static constexpr int hBed = 260;

    struct {
        struct {
            double scaleFactor = 1;
            double xOffset, yOffset;
        } view;

        struct {
            double x, y;
        } player;

        struct {
            QPixmap pixBedEmpty;
            QPixmap pixBedBoy, pixBedGirl;
        } asset;

        bool glState = false;
    } data;
};

#endif // SLEEPROOM_H
