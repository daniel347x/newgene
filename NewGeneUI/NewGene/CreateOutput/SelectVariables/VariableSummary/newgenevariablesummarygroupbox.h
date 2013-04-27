#ifndef NEWGENEVARIABLESUMMARYGROUPBOX_H
#define NEWGENEVARIABLESUMMARYGROUPBOX_H

#include <QGroupBox>

namespace Ui {
class NewGeneVariableSummaryGroupBox;
}

class NewGeneVariableSummaryGroupBox : public QGroupBox
{
    Q_OBJECT
    
public:
    explicit NewGeneVariableSummaryGroupBox(QWidget *parent = 0);
    ~NewGeneVariableSummaryGroupBox();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::NewGeneVariableSummaryGroupBox *ui;

public:
    static int index_;
    int myIndex_;
};

#endif // NEWGENEVARIABLESUMMARYGROUPBOX_H
