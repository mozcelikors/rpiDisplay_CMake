#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMovie>

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
	ui->graphicsView->setGeometry(147, 10, 40, 40);
	ui->graphicsView->setStyleSheet("background-image: url(:/gui/home.png)");

	ui->graphicsView_2->setGeometry(198, 10, 40, 40);
	ui->graphicsView_2->setStyleSheet("background-image: url(:/gui/cloud.png)");

	ui->graphicsView_3->setGeometry(261, 8, 40, 40);
	ui->graphicsView_3->setStyleSheet("background-image: url(:/gui/settings.png)");

	QMovie *movie = new QMovie(":/gui/anim.gif");
	ui->label_3->setMovie(movie);
	ui->label_3->setGeometry(8,231,301,9);
	movie->start();
}

MainWindow::~MainWindow()
{
	delete ui;
}
