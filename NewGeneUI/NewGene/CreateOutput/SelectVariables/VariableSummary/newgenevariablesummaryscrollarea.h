#ifndef NEWGENEVARIABLESUMMARYSCROLLAREA_H
#define NEWGENEVARIABLESUMMARYSCROLLAREA_H

#include <QScrollArea>

namespace Ui {
class NewGeneVariableSummaryScrollArea;
}

class NewGeneVariableSummaryScrollArea : public QScrollArea
{
    Q_OBJECT
    
public:
    explicit NewGeneVariableSummaryScrollArea(QWidget *parent = 0);
    ~NewGeneVariableSummaryScrollArea();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::NewGeneVariableSummaryScrollArea *ui;
};

#endif // NEWGENEVARIABLESUMMARYSCROLLAREA_H
