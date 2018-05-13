// To run on a framebuffer
// export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1
// export QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/qt5/plugins/platforms
// Provided that linuxfb plugin for Qt5 is installed.
//
// To run an app in the running DISPLAY (Xorg),
// LD_LIBRARY_PATH=. DISPLAY=":0" ./frontend_app

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <wiringPi.h>

#include <QMovie>
#include <QTimer>
#include "QDebug"
#include <QFocusEvent>
#include <QSplashScreen>

#include <unistd.h>
#include "BackendReceiveThread.h"
#include "main.h"

#include "common.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	/* Hidden toolbar */
	QMainWindow::setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

	/* Fullscreen, Set fixed size for main window */
	QMainWindow::showFullScreen();
	this->setFixedSize(QSize(320, 240));

	/* Netz Image */
	ui->graphicsView_netz->setGeometry(10, 10, 99,34);
	ui->graphicsView_netz->setStyleSheet("background-image: url(:/gui/netz.png)");
	ui->graphicsView_netz->setVisible(true);

	/* Menu graphics */
	ui->graphicsView->setGeometry(134, 10, 40, 40);
	ui->graphicsView->setStyleSheet("background-image: url(:/gui/home.png)");

	ui->graphicsView_2->setGeometry(198, 10, 40, 40);
	ui->graphicsView_2->setStyleSheet("background-image: url(:/gui/cloud.png)");

	ui->graphicsView_3->setGeometry(261, 8, 40, 40);
	ui->graphicsView_3->setStyleSheet("background-image: url(:/gui/settings.png)");

	QMovie *movie = new QMovie(":/gui/anim.gif");
	ui->label_3->setMovie(movie);
	ui->label_3->setGeometry(8,231,301,9);
	movie->start();

	connect(ui->pushButton_shutdown, SIGNAL(clicked()), this, SLOT(shutdownSystem()));
	connect(ui->pushButton_reboot, SIGNAL(clicked()), this, SLOT(rebootSystem()));

	BackendReceiveThread *backendrecv_thread = new BackendReceiveThread();
	backendrecv_thread->start();

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
	timer->start(200);

	this->screenUnblank();

	QTimer *timer2 = new QTimer(this);
	connect(timer2, SIGNAL(timeout()), this, SLOT(screenUnblank()));
	timer2->start(5000);
}

void MainWindow::shutdownSystem (void)
{
	this->splashScreen();
	system("halt");
}

void MainWindow::splashScreen (void)
{
	QPixmap pixmap(":/gui/splash.png");
	ui->label_splashquit->setGeometry(0,0,320,240);
	ui->label_splashquit->setPixmap(pixmap);
	ui->label_splashquit->showFullScreen();
	ui->label_splashquit->repaint();
	usleep (3 * 1e6);
}

void MainWindow::rebootSystem (void)
{
	this->splashScreen();
	system("reboot");
}

void MainWindow::downFocus (void)
{
	this->focusNextPrevChild(true);
}

void MainWindow::upFocus (void)
{
	this->focusNextPrevChild(false);
}

void MainWindow::switchTab (void)
{
	if (ui->tabWidget->currentIndex() == 2)
		ui->tabWidget->setCurrentIndex(1);
	else if (ui->tabWidget->currentIndex() == 1)
		ui->tabWidget->setCurrentIndex(0);
	else if (ui->tabWidget->currentIndex() == 0)
		ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::screenUnblank(void)
{
	FILE *fd;
	fd = popen("echo 0 > /sys/class/graphics/fb0/blank", "w");  //fb0 is intentional
	if (fd)
	{
		qDebug() << "Updated screen light";
	}
}

void MainWindow::okayOperation (void)
{
	QWidget * fw = qApp->focusWidget(); // get Focused widget
	QString classname = QString::fromUtf8(fw->metaObject()->className());

	if (QString::compare("QPushButton", classname) == 0)
	{
		QPushButton *pb = (QPushButton*) fw;
		pb->animateClick();
	}
}

void MainWindow::cancelOperation (void)
{

}

void MainWindow::timerUpdate(void)
{
	if (gpio_interrupt == BUTTON_0_PRESSED_MSG)
	{
		this->switchTab();
		gpio_interrupt = BUTTON_IDLE_MSG;
	}

	if (gpio_interrupt == BUTTON_1_PRESSED_MSG)
	{
		this->upFocus();
		gpio_interrupt = BUTTON_IDLE_MSG;
	}

	if (gpio_interrupt == BUTTON_2_PRESSED_MSG)
	{
		this->downFocus();
		gpio_interrupt = BUTTON_IDLE_MSG;
	}

	if (gpio_interrupt == BUTTON_3_PRESSED_MSG)
	{
		this->cancelOperation();
		gpio_interrupt = BUTTON_IDLE_MSG;
	}

	if (gpio_interrupt == BUTTON_4_PRESSED_MSG)
	{
		this->okayOperation();
		gpio_interrupt = BUTTON_IDLE_MSG;
	}
}

MainWindow::~MainWindow()
{
	delete ui;
}
