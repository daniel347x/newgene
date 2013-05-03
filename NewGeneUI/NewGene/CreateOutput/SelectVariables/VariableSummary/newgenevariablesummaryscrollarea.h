#ifndef NEWGENEVARIABLESUMMARYSCROLLAREA_H
#define NEWGENEVARIABLESUMMARYSCROLLAREA_H

#include <QWidget>
#include "NewGeneVariableSummaryGroup.h"

namespace Ui {
class NewGeneVariableSummaryScrollArea;
}

class NewGeneVariableSummaryScrollArea : public QWidget
{
    Q_OBJECT
    
public:
    explicit NewGeneVariableSummaryScrollArea(QWidget *parent = 0);
    ~NewGeneVariableSummaryScrollArea();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::NewGeneVariableSummaryScrollArea *ui;

private:
    NewGeneVariableSummaryGroup * groups;

};

#endif // NEWGENEVARIABLESUMMARYSCROLLAREA_H
