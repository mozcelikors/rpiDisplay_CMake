#include "mainwindow.h"
#include <QApplication>
#include <QObject>
#include <mainwidget.h>

#include <QFile>
#include <QFontDatabase>

#include <QSplashScreen>

#include "main.h"
#include <QString>

#include <unistd.h>

#include "common.h"

int gpio_interrupt = BUTTON_IDLE_MSG;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	/* Hide cursor */
	QApplication::setOverrideCursor(Qt::BlankCursor);

	QPixmap pixmap(":/gui/splash.png");
	QSplashScreen splash(pixmap);
	splash.show();

	MainWindow w;

	/* Set a stylesheet file */
	QFile File(":/gui/stylesheet.qss");
	File.open(QFile::ReadOnly);
	QString StyleSheet = QLatin1String(File.readAll());
	a.setStyleSheet(StyleSheet);

	/* Load Ubuntu font */
	QFontDatabase::addApplicationFont(":/gui/Ubuntu-R.ttf");

	usleep (2 * 1e6);

	w.show();
	splash.finish(&w);

	return a.exec();
}
