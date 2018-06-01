#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	void switchTab (void);
	void downFocus (void);
	void upFocus (void);
	void splashScreen (void);
	void drawMenubarIcons (void);
private:
	Ui::MainWindow *ui;
	int checkIfFolder_treeview (void);
	int usb_mounted_f ;

private slots:
	void timerUpdate (void);
	void screenUnblank (void);
	void shutdownSystem (void);
	void okayOperation (void);
	void cancelOperation (void);
	void rebootSystem (void);
	void timerCameraShow (void);
	void timerUsbDetect (void);
};

#endif // MAINWINDOW_H
