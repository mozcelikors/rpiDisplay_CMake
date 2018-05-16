#include <CustomLineEdit.h>
#include <QWidget>
#include <QLabel>

CustomLineEdit::CustomLineEdit( QWidget *parent )
	: QLabel( parent )
{
	this->setFocusPolicy(Qt::StrongFocus);
	this->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );

}

CustomLineEdit::~CustomLineEdit(){}
