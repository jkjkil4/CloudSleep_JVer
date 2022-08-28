#include "widget.h"

#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    int fontId = QFontDatabase::addApplicationFont(":/font/src/NotoSansSChineseMedium-7.ttf");
    QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);

    QFont font = a.font();
    font.setPointSize(10);
    if(!family.isEmpty())
        font.setFamily(family);
    a.setFont(font);

    Widget w;
    w.show();
    return a.exec();
}
