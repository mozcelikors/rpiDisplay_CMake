#include <QObject>
#include <QThread>

class BackendReceiveThread : public QThread
{
	Q_OBJECT

protected:
	void run();
};

