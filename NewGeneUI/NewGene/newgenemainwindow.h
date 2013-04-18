#ifndef NEWGENEMAINWINDOW_H
#define NEWGENEMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class NewGeneMainWindow;
}

class NewGeneMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit NewGeneMainWindow(QWidget *parent = 0);
    ~NewGeneMainWindow();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::NewGeneMainWindow *ui;
};

#endif // NEWGENEMAINWINDOW_H
