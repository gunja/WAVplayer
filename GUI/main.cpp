#include <QApplication>
#include <QTextCodec>
#include "GUI/uiClass.h"


int main( int argc, char *argv[] )
{
    QApplication a( argc, argv );
    QApplication::addLibraryPath("./plugins");
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB2312"));
	Screen m;
    m.show();

    return a.exec();
}