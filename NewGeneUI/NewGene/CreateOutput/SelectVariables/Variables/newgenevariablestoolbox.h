#ifndef NEWGENEVARIABLESTOOLBOX_H
#define NEWGENEVARIABLESTOOLBOX_H

#include <QToolBox>

namespace Ui {
class NewGeneVariablesToolbox;
}

class NewGeneVariablesToolbox : public QToolBox
{
    Q_OBJECT
    
public:
    explicit NewGeneVariablesToolbox(QWidget *parent = 0);
    ~NewGeneVariablesToolbox();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::NewGeneVariablesToolbox *ui;
};

#endif // NEWGENEVARIABLESTOOLBOX_H
