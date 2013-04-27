#ifndef NEWGENEVARIABLESUMMARYHOLDER_H
#define NEWGENEVARIABLESUMMARYHOLDER_H

#include <QWidget>

namespace Ui {
class NewGeneVariableSummaryHolder;
}

class NewGeneVariableSummaryHolder : public QWidget
{
    Q_OBJECT
    
public:
    explicit NewGeneVariableSummaryHolder(QWidget *parent = 0);
    ~NewGeneVariableSummaryHolder();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::NewGeneVariableSummaryHolder *ui;
};

#endif // NEWGENEVARIABLESUMMARYHOLDER_H
