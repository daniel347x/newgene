#ifndef NEWGENEVARIABLEGROUP_H
#define NEWGENEVARIABLEGROUP_H

#include <QWidget>

namespace Ui {
class NewGeneVariableGroup;
}

class NewGeneVariableGroup : public QWidget
{
    Q_OBJECT
    
public:
    explicit NewGeneVariableGroup(QWidget *parent = 0);
    ~NewGeneVariableGroup();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::NewGeneVariableGroup *ui;
};

#endif // NEWGENEVARIABLEGROUP_H
