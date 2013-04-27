#ifndef NEWGENEVARIABLES_H
#define NEWGENEVARIABLES_H

#include <QWidget>
#include <QStringListModel>
#include <QItemSelectionModel>

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

protected:
    QStringListModel model_;

};

#endif // NEWGENEVARIABLES_H
