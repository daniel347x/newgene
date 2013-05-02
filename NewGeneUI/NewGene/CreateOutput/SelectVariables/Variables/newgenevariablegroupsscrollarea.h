#ifndef NEWGENEVARIABLEGROUPSSCROLLAREA_H
#define NEWGENEVARIABLEGROUPSSCROLLAREA_H

#include <QWidget>

namespace Ui {
class NewGeneVariableGroupsScrollArea;
}

class NewGeneVariableGroupsScrollArea : public QWidget
{
    Q_OBJECT
    
public:
    explicit NewGeneVariableGroupsScrollArea(QWidget *parent = 0);
    ~NewGeneVariableGroupsScrollArea();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::NewGeneVariableGroupsScrollArea *ui;
};

#endif // NEWGENEVARIABLEGROUPSSCROLLAREA_H
