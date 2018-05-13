#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private:
	Ui::MainWindow *ui;



private slots:
	void timerUpdate (void);
	void screenUnblank (void);
	void shutdownSystem (void);
	void okayOperation (void);
	void cancelOperation (void);
	void rebootSystem (void);
};

#endif // MAINWINDOW_H
