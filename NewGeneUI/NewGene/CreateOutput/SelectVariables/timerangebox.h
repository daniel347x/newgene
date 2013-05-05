#ifndef TIMERANGEBOX_H
#define TIMERANGEBOX_H

#include <QFrame>
#include "..\..\newgenewidget.h"

namespace Ui {
class TimeRangeBox;
}

class TimeRangeBox : public QFrame, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
    Q_OBJECT

public:
    explicit TimeRangeBox(QWidget *parent = 0);
    ~TimeRangeBox();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TimeRangeBox *ui;
};

#endif // TIMERANGEBOX_H
