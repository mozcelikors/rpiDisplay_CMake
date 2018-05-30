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

private slots:
	void timerUpdate (void);
	void screenUnblank (void);
	void shutdownSystem (void);
	void okayOperation (void);
	void cancelOperation (void);
	void rebootSystem (void);
	void timerCameraShow (void);
};

#endif // MAINWINDOW_H
