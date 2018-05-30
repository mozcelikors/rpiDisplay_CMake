#include <QObject>
#include <QThread>

class CameraThread : public QThread
{
	Q_OBJECT

protected:
	void run();
};

