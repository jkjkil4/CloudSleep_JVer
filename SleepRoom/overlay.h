#ifndef OVERLAY_H
#define OVERLAY_H

#include <QWidget>
#include <QPainter>
#include <QMessageBox>

namespace Ui {
class Overlay;
}

class Overlay : public QWidget
{
    Q_OBJECT

public:
    explicit Overlay(QWidget *parent = nullptr);
    ~Overlay() override;

public slots:
    void onBtnAssistClicked();

protected:
    void paintEvent(QPaintEvent *) override;

signals:
    void paint(QPainter *p);

private:
    friend class SleepRoom;

    Ui::Overlay *ui;
};

#endif // OVERLAY_H
