#ifndef NEWGENEVARIABLESUMMARY_H
#define NEWGENEVARIABLESUMMARY_H

#include <QWidget>

namespace Ui {
class NewGeneVariableSummary;
}

class NewGeneVariableSummary : public QWidget
{
    Q_OBJECT
    
public:
    explicit NewGeneVariableSummary(QWidget *parent = 0);
    ~NewGeneVariableSummary();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::NewGeneVariableSummary *ui;
};

#endif // NEWGENEVARIABLESUMMARY_H
