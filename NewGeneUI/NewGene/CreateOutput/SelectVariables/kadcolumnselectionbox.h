#ifndef KADCOLUMNSELECTIONBOX_H
#define KADCOLUMNSELECTIONBOX_H

#include <QFrame>

namespace Ui {
class KAdColumnSelectionBox;
}

class KAdColumnSelectionBox : public QFrame
{
    Q_OBJECT
    
public:
    explicit KAdColumnSelectionBox(QWidget *parent = 0);
    ~KAdColumnSelectionBox();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::KAdColumnSelectionBox *ui;
};

#endif // KADCOLUMNSELECTIONBOX_H
