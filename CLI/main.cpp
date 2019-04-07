#include <QCoreApplication>
#include <QTextCodec>
#include "CLI/class.h"
#include <QCommandLineParser>



int main( int argc, char *argv[] )
{
    QCoreApplication a( argc, argv );
    QCoreApplication::addLibraryPath("./plugins");
    QCoreApplication::setApplicationName("command line interface player");
    QCoreApplication::setApplicationVersion("1.0");
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB2312"));

    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption fileOption(QStringList() << "f" << "file",
               QCoreApplication::translate("main", "read file as Wav"),
               QCoreApplication::translate("main", "fileName"));
    parser.addOption(fileOption);

    QCommandLineOption outfileOption(QStringList() << "c" << "config",
               QCoreApplication::translate("main", "use <cf> file as description of interveale source"),
               QCoreApplication::translate("main", "cf"));
    parser.addOption(outfileOption);
    // Process the actual command line arguments given by the user
    parser.process(a);

	CLIClass m( argc, argv, &a);
	QObject::connect(&m, SIGNAL(finished()), &a, SLOT(quit()));
	QTimer::singleShot(0, &m, SLOT(run()));

    return a.exec();
}
