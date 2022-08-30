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

#include <QElapsedTimer>

class Overlay;

class SleepRoom : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    explicit SleepRoom(QWidget *parent = nullptr);
    ~SleepRoom() override;

protected:
    void resizeEvent(QResizeEvent *ev) override;

private:
    Overlay *mOverlay;

public:
    int floorMod(int val, int mod);
    double sqr(double v);

    void setName(const QString &name);
    void setRole(int role);
    void setId(qulonglong id);

    void clear();

private:
    struct Sleeper {
        QString name;
        int role;
        qulonglong id = 0;
        double x, y;
        int bedX, bedY;
        bool inBed = false;

        struct Chat {
            QString str;
            int timer;
        };
        QList<Chat> chats;

        QList<QPointF> path;
    };

public:
    QList<QPointF> pathTo(const QPointF &start, QPointF end);
    bool doPath(QList<QPointF> &path, double &x, double &y, double step);

protected:
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void wheelEvent(QWheelEvent *ev) override;

public:
    void onPos(qulonglong id, double x, double y);
    void onMove(qulonglong id, double x, double y);
    void onSleep(qulonglong id, int bx, int by);
    void onBlocked(int bx, int by);
    void onSleeper(const QString &name, int role, qulonglong id, double x, double y, int bx, int by, bool inBed);
    void onChat(qulonglong id, const QString &str);
    void onLeave(qulonglong id);

    void onBtnChatClicked();

signals:
    void sPos(double x, double y);
    void sMove(double x, double y);
    void sSleep(int bx, int by);
    void sChat(const QString &str);

private:
    QPoint mMousePrev;
    bool clickFlag = true;

public slots:
    void onTimerTimeout();

private:
    QTimer *mTimer;
    QElapsedTimer mElapsedTimer;

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
    bool isBed(int bx, int by);

public:
    void paintTexture(QOpenGLTexture *texture, const QPointF &pos, const QSizeF &scale, double rotation);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

public slots:
    void onPaint(QPainter *p);

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
        QMap<qulonglong, Sleeper> otherSleeper;
        qulonglong counter;

        QVector<Sleeper*> sortedSleepers;

        struct {
            QOpenGLTexture *textureBedEmpty;

            struct SleeperImage { QImage walk, bed; };
            QVector<SleeperImage> sleeperImages;

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
