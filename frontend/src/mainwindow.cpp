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
#include <QDirModel>

#include <unistd.h>
#include "BackendReceiveThread.h"
#include "main.h"

#include "common.h"

#define TABWIDGET_HOMEPAGE_IDX 2
#define TABWIDGET_CLOUDPAGE_IDX 1
#define TABWIDGET_SETTINGSPAGE_IDX 0

#define APP_WIDTH 320
#define APP_HEIGHT 240

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

	/* Be sure to use layout and add the following. Important when using scrollArea */
	ui->scrollAreaWidgetContents_homepage->setLayout(ui->verticalLayout);

	//CustomLineEdit *cle_homepage = new CustomLineEdit(this);
	//ui->horizontalLayout_2->addWidget(cle_homepage);
	//cle_homepage->setText("asd");

	QDirModel *dir_model = new QDirModel();
	dir_model->setSorting(QDir::DirsFirst |
						  QDir::IgnoreCase |
						  QDir::Name);

	ui->treeView->setModel(dir_model);
	QModelIndex treeView_index = dir_model->index("/home/root/projects");
	ui->treeView->setRootIndex(treeView_index);
	ui->treeView->header()->resizeSection(0,170);
	ui->treeView->header()->resizeSection(1,30);
	ui->treeView->header()->resizeSection(2,50);

}

void MainWindow::shutdownSystem (void)
{
	this->splashScreen();
	system("halt");
}

void MainWindow::splashScreen (void)
{
	QPixmap pixmap(":/gui/splash.png");
	ui->label_splashquit->setGeometry(0,0,APP_WIDTH,APP_HEIGHT);
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
	/* Scroll to the focused widget */
	QWidget * fw = qApp->focusWidget(); // get Focused widget
	QString classname = QString::fromUtf8(fw->metaObject()->className());
	QPoint globalPos =fw->mapToGlobal(fw->rect().topLeft());
	this->focusNextPrevChild(true);

	if (ui->tabWidget->currentIndex() == TABWIDGET_HOMEPAGE_IDX)
	{
		// we added a spacer label at the bottom and scrolling 80px downward to make last button visible.
		//ui->scrollArea_homepage->ensureWidgetVisible(fw);

		if (globalPos.y() < APP_HEIGHT/2)
			ui->scrollArea_homepage->ensureVisible(0, (globalPos.y()<APP_HEIGHT)?(globalPos.y()-50):APP_HEIGHT, 0, 0);
		else if (globalPos.y() >= APP_HEIGHT/2)
			ui->scrollArea_homepage->ensureVisible(0, (globalPos.y()<=APP_HEIGHT)?(globalPos.y()+80):APP_HEIGHT, 0, 0);
	}
	else if (ui->tabWidget->currentIndex() == TABWIDGET_SETTINGSPAGE_IDX)
	{
		QModelIndex index_it = ui->treeView->indexBelow(ui->treeView->currentIndex());
		if (index_it.isValid())
		{
			ui->treeView->setCurrentIndex(index_it);
			ui->treeView->selectionModel()->select(index_it, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		}

		//qDebug() << "rowCount:" << this->countRowsOfIndex_treeview(ui->treeView->currentIndex());
		//QModelIndex index = ui->treeView->currentIndex();
		//QVariant data = ui->treeView->model()->data(index);
		//qDebug() << "currIndex:" << data.toString();
	}

}

void MainWindow::upFocus (void)
{
	/* Scroll to the focused widget */
	QWidget * fw = qApp->focusWidget(); // get Focused widget
	QString classname = QString::fromUtf8(fw->metaObject()->className());
	QPoint globalPos =fw->mapToGlobal(fw->rect().topLeft());
	this->focusNextPrevChild(false);

	if (ui->tabWidget->currentIndex() == TABWIDGET_HOMEPAGE_IDX)
	{
		ui->scrollArea_homepage->ensureWidgetVisible(fw);
	}
	else if (ui->tabWidget->currentIndex() == TABWIDGET_SETTINGSPAGE_IDX)
	{

		QModelIndex index_it = ui->treeView->indexAbove(ui->treeView->currentIndex());
		if (index_it.isValid())
		{
			ui->treeView->setCurrentIndex(index_it);
			ui->treeView->selectionModel()->select(index_it, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		}

		//qDebug() << "rowCount:" << this->countRowsOfIndex_treeview(ui->treeView->currentIndex());
		//QModelIndex index = ui->treeView->currentIndex();
		//QVariant data = ui->treeView->model()->data(index);
		//qDebug() << "currIndex:" << data.toString();

	}


}

void MainWindow::switchTab (void)
{
	if (ui->tabWidget->currentIndex() == TABWIDGET_HOMEPAGE_IDX)
		ui->tabWidget->setCurrentIndex(TABWIDGET_CLOUDPAGE_IDX);
	else if (ui->tabWidget->currentIndex() == TABWIDGET_CLOUDPAGE_IDX)
	{
		ui->tabWidget->setCurrentIndex(TABWIDGET_SETTINGSPAGE_IDX);
	}
	else if (ui->tabWidget->currentIndex() == TABWIDGET_SETTINGSPAGE_IDX)
		ui->tabWidget->setCurrentIndex(TABWIDGET_HOMEPAGE_IDX);
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

int MainWindow::checkIfFolder_treeview (void)
{
	/* Check if highlighted item is a folder by checking type name*/
	QModelIndex index_foldersearch = ui->treeView->currentIndex().sibling(ui->treeView->currentIndex().row(), 2);
	QVariant data_foldersearch = ui->treeView->model()->data(index_foldersearch);
	//qDebug() << "typeName:" << data_foldersearch.toString();
	if (QString::compare("Folder", data_foldersearch.toString()) == 0)
	{
		/* Selected item is a folder!*/
		return 0;
	}
	else
		return -1;
}

void MainWindow::okayOperation (void)
{
	if (ui->tabWidget->currentIndex() == TABWIDGET_HOMEPAGE_IDX)
	{
		/* Animate pressing a button */
		QWidget * fw = qApp->focusWidget(); // get Focused widget
		QString classname = QString::fromUtf8(fw->metaObject()->className());

		if (QString::compare("QPushButton", classname) == 0)
		{
			QPushButton *pb = (QPushButton*) fw;
			pb->animateClick();
		}
	}
	else if (ui->tabWidget->currentIndex() == TABWIDGET_SETTINGSPAGE_IDX)
	{
		/* Check if highlighted item is a folder by checking type name*/
		if (this->checkIfFolder_treeview() == 0)
		{
			/* Selected item is a folder!*/
			/* Expand if not expanded, collapse if expanded. */
			if (!ui->treeView->isExpanded(ui->treeView->currentIndex()))
			{
				ui->treeView->expand(ui->treeView->currentIndex());
			}
			else
			{
				ui->treeView->collapse(ui->treeView->currentIndex());
			}
		}
		else
		{
			/* Selected item is not a folder! */

			/* Open an item in treeview */
			QModelIndex index = ui->treeView->currentIndex();
			QVariant data = ui->treeView->model()->data(index);
			qDebug() << "currAppName:" << data.toString();
		}


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
