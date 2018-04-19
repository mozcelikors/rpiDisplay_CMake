#include "mainwindow.h"
#include <QApplication>
#include <QObject>
#include <mainwidget.h>

#include <QFile>
#include <QFontDatabase>

#include <QString>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;

	/* Set a stylesheet file */
	QFile File(":/gui/stylesheet.qss");
	File.open(QFile::ReadOnly);
	QString StyleSheet = QLatin1String(File.readAll());
	a.setStyleSheet(StyleSheet);

	/* Load Ubuntu font */
	QFontDatabase::addApplicationFont(":/gui/Ubuntu-R.ttf");

	w.show();

	return a.exec();
}
