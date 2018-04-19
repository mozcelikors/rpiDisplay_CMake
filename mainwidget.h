#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QTabWidget>

// Example subclassed widget for future use and tailoring!, Mustafa
class MainWidget : public QTabWidget
{
	Q_OBJECT
public:
	explicit MainWidget(QWidget *parent = 0);
	~MainWidget();

//private slots:
};

#endif // MAINWIDGET_H
