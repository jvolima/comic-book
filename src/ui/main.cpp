#include "comicbookreader.h"
#include <QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    ComicBookReader w;
    w.show();
    return a.exec();
}