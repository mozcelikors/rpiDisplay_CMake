#include <QWidget>
#include <QLabel>

class CustomLineEdit : public QLabel
{
        Q_OBJECT

        public:
				CustomLineEdit( QWidget *parent = 0 );
				~CustomLineEdit();

        signals:

};
