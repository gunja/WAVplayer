#include <QApplication>
#include <QTextCodec>
#include "CLI/class.h"


int main( int argc, char *argv[] )
{
    QApplication a( argc, argv );
    QApplication::addLibraryPath("./plugins");
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB2312"));
	CLIClass m( argc, argv, &a);
	QObject::connect(&m, SIGNAL(finished()), &a, SLOT(quit()));
	QTimer::singleShot(0, &m, SLOT(run()));

    return a.exec();
}
