// Example subclassed widget for future use and tailoring!, Mustafa

#include "mainwidget.h"


// To use this example subclassed widget:
/*MainWidget *main_widget = new MainWidget;
QWidget *tabHome = new QWidget(main_widget);
main_widget->addTab(tabHome, QObject::tr("name"));

main_widget->show();*/

/* !!! Very good and more generic example:All you need is to pass the parent to the widget's constructor: e.g.
(i.e. where to add this = Inside ui->frame)

All you need is to pass the parent to the widget's constructor:

Tile *tile = new Tile(ui->frame); // <-- here
tile->setImg("://Images/placeholderTile.png");
tile->setCol(true);
tile->setGeometry(retX(line),retY(line),50,50);

Since Tile is your own class, you should definitely have a Qt-style, parent-taking explicit constructor for it:

class Tile : public QWidget {
  ...
public:
  explicit Tile(QWidget * parent = 0) : QWidget(parent) { ... }
};
*/

MainWidget::MainWidget(QWidget *parent)
{

}

MainWidget::~MainWidget()
{

}
