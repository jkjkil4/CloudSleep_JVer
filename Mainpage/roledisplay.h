#ifndef ROLEDISPLAY_H
#define ROLEDISPLAY_H

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>

class RoleDisplay : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int mOffset READ offset WRITE setOffset)
public:
    explicit RoleDisplay(QWidget *parent = nullptr);

    int ind() { return mInd; }
    void setInd(int ind);

    void adjust(int indOffset);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    int offset() { return mOffset; }
    void setOffset(int offset);

    QVector<QPixmap> mRoles;

    int mInd = 0;
    int mOffset = 0;
};

#endif // ROLEDISPLAY_H
