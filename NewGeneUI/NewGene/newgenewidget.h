#ifndef NEWGENEWIDGET_H
#define NEWGENEWIDGET_H

#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include "newgenemodel.h"
class QWidget;
class NewGeneMainWindow;

class NewGeneWidget
{
public:
    explicit NewGeneWidget(QWidget * self_ = 0);

protected:
    NewGeneMainWindow & mainWindow();
    NewGeneModel & model();

private:
    QWidget * self;

};

#endif // NEWGENEWIDGET_H
