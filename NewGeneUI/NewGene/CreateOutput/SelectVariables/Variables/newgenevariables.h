#ifndef NEWGENEVARIABLES_H
#define NEWGENEVARIABLES_H

#include <QWidget>

namespace Ui {
class NewGeneVariables;
}

class NewGeneVariables : public QWidget
{
    Q_OBJECT
    
public:
    explicit NewGeneVariables(QWidget *parent = 0);
    ~NewGeneVariables();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::NewGeneVariables *ui;
};

#endif // NEWGENEVARIABLES_H
