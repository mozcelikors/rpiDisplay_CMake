// To run on a framebuffer
// export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1
// export QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/qt5/plugins/platforms
// Provided that linuxfb plugin for Qt5 is installed.
//
// To run an app in the running DISPLAY (Xorg),
// LD_LIBRARY_PATH=/home/root/projects DISPLAY=":0" /home/root/projects/frontend_app
//
//
// To debug a deployed app:
// export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1
// killall frontend_app
// LD_LIBRARY_PATH=/home/root/projects /home/root/projects/backend_app &
// LD_LIBRARY_PATH=/home/root/projects /home/root/projects/frontend_app
//
// Change device IP from Tools, also from Projects>Run>[remote commands]

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <wiringPi.h>

#include <QMovie>
#include <QTimer>
#include "QDebug"
#include <QFocusEvent>
#include <QSplashScreen>
#include <QDirModel>
#include <QGraphicsPixmapItem>

#include <unistd.h>
#include "BackendReceiveThread.h"
#include "CameraThread.h"
#include "main.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core.hpp>

#include "camera_interface.h"

#include <semaphore.h>

#include "common.h"

#define TABWIDGET_CAMERAPAGE_IDX 2
#define TABWIDGET_USBPAGE_IDX 1
#define TABWIDGET_MENUPAGE_IDX 0


MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	/* Hidden toolbar */
	QMainWindow::setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

	/* Fullscreen, Set fixed size for main window */
	QMainWindow::showFullScreen();
	this->setFixedSize(QSize(APP_WIDTH, APP_HEIGHT));

	/* Netz Image */
	ui->graphicsView_netz->setGeometry(10, 10, 99,34);
	ui->graphicsView_netz->setStyleSheet("background-image: url(:/gui/netz.png)");
	ui->graphicsView_netz->setVisible(true);

	/* Menu graphics */
	ui->graphicsView->setGeometry(137, 19, 39, 36);
	ui->graphicsView->setStyleSheet("background-image: url(:/gui/camera.png)");

	ui->graphicsView_2->setGeometry(198, 22, 40, 32);
	ui->graphicsView_2->setStyleSheet("background-image: url(:/gui/usb.png)");

	ui->graphicsView_3->setGeometry(261, 19, 42, 38);
	ui->graphicsView_3->setStyleSheet("background-image: url(:/gui/menu.png)");

	/* Menu bar icons */
	this->drawMenubarIcons();

	/* Camera visualsinitializations */
	ui->graphicsView_cameramenu->setStyleSheet("background-image: url(:/gui/testimage.jpg)");

	ui->label_pressOK3_2->setStyleSheet("color:white;");
	ui->label_pressOK3->setStyleSheet("color:white;");

	ui->graphicsView_camera->setGeometry(0, 0, APP_WIDTH, APP_HEIGHT);
	ui->graphicsView_camera->setVisible(false);



	/* Initialize camera interface */
#ifdef CAMERA_DISPLAY_METHOD_1
	csi.camera_scene = new QGraphicsScene();
	csi.camera_active_f = 0;
	csi.camera_image = cv::Mat::zeros(cv::Size(APP_WIDTH, APP_HEIGHT), CV_8UC3);
#endif
	/* Camera thread */
	int sem_rc = -1;
	sem_rc = sem_init(&csi.camera_sem, 0, 1);
	CHECK (sem_rc == 0);

	CameraThread *camera_thread = new CameraThread();
	camera_thread->start();
	//camera_thread->setPriority(QThread::HighPriority);



	QTimer *timer_camerashow = new QTimer(this);
	connect(timer_camerashow, SIGNAL(timeout()), this, SLOT(timerCameraShow()));
	timer_camerashow->start(30); /* ~30fps */


	/* Backend communication thread */
	BackendReceiveThread *backendrecv_thread = new BackendReceiveThread();
	backendrecv_thread->start();
	backendrecv_thread->setPriority(QThread::HighestPriority);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
	timer->start(200);

	/* Unblank screen periodically */
	this->screenUnblank();

	QTimer *timer2 = new QTimer(this);
	connect(timer2, SIGNAL(timeout()), this, SLOT(screenUnblank()));
	timer2->start(5000);

	/* Shut down and reboot hooks */
	connect(ui->pushButton_shutdown, SIGNAL(clicked()), this, SLOT(shutdownSystem()));
	connect(ui->pushButton_reboot, SIGNAL(clicked()), this, SLOT(rebootSystem()));

	/* Be sure to use layout and add the following. Important when using scrollArea */
	ui->scrollAreaWidgetContents_homepage->setLayout(ui->verticalLayout);

	//CustomLineEdit *cle_homepage = new CustomLineEdit(this);
	//ui->horizontalLayout_2->addWidget(cle_homepage);
	//cle_homepage->setText("asd");

	/* USB detect */
	QTimer *timer_usbdetect = new QTimer(this);
	connect(timer_usbdetect, SIGNAL(timeout()), this, SLOT(timerUsbDetect()));
	timer_usbdetect->start(1000);

	/* USB menu tree view page */
	this->usb_mounted_f = 0;
	ui->label_noUSBDev->setStyleSheet("color:gray;");
	ui->label_noUSBDev->setVisible(true);
	ui->treeView->setVisible(false);

	/* USB menu tree view initialize */
	QDirModel *dir_model = new QDirModel();
	dir_model->setSorting(QDir::DirsFirst |
						  QDir::IgnoreCase |
						  QDir::Name);

	ui->treeView->setModel(dir_model);
	QModelIndex treeView_index = dir_model->index("/home/root/projects");
	ui->treeView->setRootIndex(treeView_index);
	ui->treeView->header()->resizeSection(0,170);
	ui->treeView->header()->resizeSection(1,30);
	ui->treeView->header()->resizeSection(2,70);

}

static QImage Mat2QImage(cv::Mat const& src)
{
	 cv::Mat temp; // make the same cv::Mat
	 cvtColor(src, temp,CV_BGR2RGB); // cvtColor Makes a copt, that what i need
	 QImage dest((const uchar *) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
	 dest.bits(); // enforce deep copy, see documentation
	 // of QImage::QImage ( const uchar * data, int width, int height, Format format )
	 return dest;
}

static cv::Mat QImage2Mat(QImage const& src)
{
	 cv::Mat tmp(src.height(),src.width(),CV_8UC3,(uchar*)src.bits(),src.bytesPerLine());
	 cv::Mat result; // deep copy just in case (my lack of knowledge with open cv)
	 cvtColor(tmp, result,CV_BGR2RGB);
	 return result;
}

void MainWindow::timerUsbDetect (void)
{
	/* Automount should be enabled by installing udev-extraconf to the Linux image */
	/* sda */
	FILE *f_a = popen("mount | grep /dev/sda1", "r");
	if (NULL != f_a)
	{
		if (EOF == fgetc(f_a))
		{
			/* Not mounted */
			ui->label_noUSBDev->setVisible(true);
			ui->treeView->setVisible(false);
			this->usb_mounted_f = 0;
		}
		else
		{
			if (this->usb_mounted_f == 0)
			{
				/* Mounted */
				ui->label_noUSBDev->setVisible(false);
				ui->treeView->setVisible(true);
				/* USB menu tree view initialize */
				QDirModel *dir_model = new QDirModel();
				dir_model->setSorting(QDir::DirsFirst |
									  QDir::IgnoreCase |
									  QDir::Name);

				ui->treeView->setModel(dir_model);
				QModelIndex treeView_index = dir_model->index("/run/media/sda1");
				ui->treeView->setRootIndex(treeView_index);
				ui->treeView->header()->resizeSection(0,170);
				ui->treeView->header()->resizeSection(1,30);
				ui->treeView->header()->resizeSection(2,70);
				this->usb_mounted_f = 1;
			}
		}

		pclose(f_a);
	}
}

void MainWindow::timerCameraShow (void)
{
	if (csi.camera_active_f == 1)
	{
		//qDebug() << "TimerCameraShow";
#ifdef CAMERA_DISPLAY_METHOD_1
		sem_wait(&csi.camera_sem);
		QImage img = Mat2QImage(csi.camera_image);
		sem_post(&csi.camera_sem);

		QPixmap m_pixmap = QPixmap::fromImage(img);

		csi.camera_scene->addPixmap(m_pixmap);
		csi.camera_scene->setSceneRect(0,0,m_pixmap.width(), m_pixmap.height());

		ui->graphicsView_camera->setScene(csi.camera_scene);
		ui->graphicsView_camera->repaint();
#else
#ifdef CAMERA_DISPLAY_METHOD_2
		sem_wait(&csi.camera_sem);
		ui->graphicsView_camera->setStyleSheet("background-image: url(/home/root/projects/camera/embeddev_cv_image.jpg)");
		sem_post(&csi.camera_sem);
		ui->graphicsView_camera->repaint();
#endif
#endif
	}
}

void MainWindow::drawMenubarIcons (void)
{
	int positions_x[2] = {281, 251};
	int positions_y[2] = {0,   0};
	int widths[2] =      {30, 30};
	int heights[2] =     {20, 20};
	ui->graphicsView_battery->setGeometry(positions_x[0], positions_y[0], widths[0], heights[0]);
	ui->graphicsView_battery->setStyleSheet("background-image: url(:/gui/level3.png)");

	ui->graphicsView_connection->setGeometry(positions_x[1], positions_y[1], widths[1], heights[1]);
	ui->graphicsView_connection->setStyleSheet("background-image: url(:/gui/levelcharging.png)");
	ui->graphicsView_connection->setVisible(false);
}

void MainWindow::shutdownSystem (void)
{
	ui->graphicsView_connection->setVisible(false);
	ui->graphicsView_battery->setVisible(false);
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

	if (ui->tabWidget->currentIndex() == TABWIDGET_MENUPAGE_IDX)
	{
		// we added a spacer label at the bottom and scrolling 80px downward to make last button visible.
		//ui->scrollArea_homepage->ensureWidgetVisible(fw);

		if (globalPos.y() < APP_HEIGHT/2)
			ui->scrollArea_homepage->ensureVisible(0, (globalPos.y()<APP_HEIGHT)?(globalPos.y()-50):APP_HEIGHT, 0, 0);
		else if (globalPos.y() >= APP_HEIGHT/2)
			ui->scrollArea_homepage->ensureVisible(0, (globalPos.y()<=APP_HEIGHT)?(globalPos.y()+80):APP_HEIGHT, 0, 0);
	}
	else if (ui->tabWidget->currentIndex() == TABWIDGET_USBPAGE_IDX)
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

	if (ui->tabWidget->currentIndex() == TABWIDGET_MENUPAGE_IDX)
	{
		ui->scrollArea_homepage->ensureWidgetVisible(fw);
	}
	else if (ui->tabWidget->currentIndex() == TABWIDGET_USBPAGE_IDX)
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
	if (csi.camera_active_f != 1)
	{
		if (ui->tabWidget->currentIndex() == TABWIDGET_CAMERAPAGE_IDX)
			ui->tabWidget->setCurrentIndex(TABWIDGET_USBPAGE_IDX);
		else if (ui->tabWidget->currentIndex() == TABWIDGET_USBPAGE_IDX)
		{
			ui->tabWidget->setCurrentIndex(TABWIDGET_MENUPAGE_IDX);
		}
		else if (ui->tabWidget->currentIndex() == TABWIDGET_MENUPAGE_IDX)
			ui->tabWidget->setCurrentIndex(TABWIDGET_CAMERAPAGE_IDX);
	}
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
	if (ui->tabWidget->currentIndex() == TABWIDGET_MENUPAGE_IDX)
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
	else if (ui->tabWidget->currentIndex() == TABWIDGET_USBPAGE_IDX)
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
	else if (ui->tabWidget->currentIndex() == TABWIDGET_CAMERAPAGE_IDX)
	{
		if (csi.camera_active_f != 1)
		{
			csi.camera_active_f = 1;
			ui->label_pressOK3_2->setVisible(false);
			ui->label_pressOK3->setVisible(false);
			ui->graphicsView_camera->setVisible(true);
		}
	}
}

void MainWindow::cancelOperation (void)
{
	if (ui->tabWidget->currentIndex() == TABWIDGET_CAMERAPAGE_IDX)
	{
		if (csi.camera_active_f == 1)
		{
			csi.camera_active_f = 0;
			ui->label_pressOK3_2->setVisible(true);
			ui->label_pressOK3->setVisible(true);
			ui->graphicsView_camera->setVisible(false);
		}
	}
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
