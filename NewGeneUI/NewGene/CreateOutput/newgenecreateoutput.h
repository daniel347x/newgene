#ifndef NEWGENECREATEOUTPUT_H
#define NEWGENECREATEOUTPUT_H

#include <QWidget>

namespace Ui {
class NewGeneCreateOutput;
}

class NewGeneCreateOutput : public QWidget
{
    Q_OBJECT
    
public:
    explicit NewGeneCreateOutput(QWidget *parent = 0);
    ~NewGeneCreateOutput();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::NewGeneCreateOutput *ui;
};

#endif // NEWGENECREATEOUTPUT_H
