#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //备份文件夹 ./backup, 不存在则创建
    QDir dir;
    if(!dir.exists("backup")){
        dir.mkdir("backup");
    }


    MainWindow w;
    w.show();
    return a.exec();
}
