#ifndef SLEEPROOM_H
#define SLEEPROOM_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QPainter>
#include <QtMath>
#include <QTimer>
#include <QWheelEvent>

class SleepRoom : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    explicit SleepRoom(QWidget *parent = nullptr);
    ~SleepRoom() override;

    int floorMod(int val, int mod);
    double sqr(double v);

    void setName(const QString &name);
    void setRole(int role);
    void setId(qulonglong id);

    void clear();

private:
    QString mName;
    int mRole;
    qulonglong mId;

    struct Sleeper {
        double x, y;
        QList<QPointF> path;
        int bedX, bedY;
        bool inBed = false;
    };

public:
    QList<QPointF> pathTo(const QPointF &start, QPointF end);
    bool doPath(QList<QPointF> &path, double &x, double &y);

protected:
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void wheelEvent(QWheelEvent *ev) override;

public:
    void onPos(qulonglong id, double x, double y);
    void onMove(qulonglong id, double x, double y);
    void onSleep(qulonglong id, int x, int y);

signals:
    void sPos(double x, double y);
    void sMove(double x, double y);
    void sSleep(int x, int y);

private:
    QPoint mMousePrev;
    bool clickFlag = true;

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

    bool collisionBed(double viewx, double viewy);

public:
    void paintTexture(QOpenGLTexture *texture, const QPointF &pos, const QSizeF &scale, double rotation);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    static constexpr int wSpc = 40;
    static constexpr int hSpc = 40;
    static constexpr int wBed = 151;
    static constexpr int hBed = 247;

    struct {
        struct {
            double scaleFactor = 0, adjustedScaleFactor = qPow(2, 0);
            double xOffset, yOffset;
        } view;

        Sleeper player;
        QList<Sleeper> otherSleeper;
        qulonglong counter;

        struct {
            QOpenGLTexture *textureBedEmpty;

            struct SleeperTexture { QOpenGLTexture *walk, *bed; };
            QVector<SleeperTexture> sleeperTextures;
        } asset;

        bool glInitialized;
    } data;

    struct {
        GLuint vbo;
        GLuint vao;
        GLuint ebo;
        QOpenGLShaderProgram program;
        GLint locationWindowSize, locationTexPos, locationTexSize, locationRotation;
    } shader;
};

#endif // SLEEPROOM_H
