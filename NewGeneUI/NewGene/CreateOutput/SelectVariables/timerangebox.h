#ifndef TIMERANGEBOX_H
#define TIMERANGEBOX_H

#include <QFrame>

namespace Ui {
class TimeRangeBox;
}

class TimeRangeBox : public QFrame
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
