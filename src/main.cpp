#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    QApplication::setApplicationName("LogParser");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("LogParser");
    
    // 设置应用程序样式
    app.setStyle("Fusion");
    
    // 创建并显示主窗口
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}
