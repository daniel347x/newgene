#ifndef NEWGENESELECTVARIABLES_H
#define NEWGENESELECTVARIABLES_H

#include <QWidget>

namespace Ui {
class NewGeneSelectVariables;
}

class NewGeneSelectVariables : public QWidget
{
    Q_OBJECT
    
public:
    explicit NewGeneSelectVariables(QWidget *parent = 0);
    ~NewGeneSelectVariables();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::NewGeneSelectVariables *ui;
};

#endif // NEWGENESELECTVARIABLES_H
