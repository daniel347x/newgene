#ifndef NEWGENEMAINWINDOW_H
#define NEWGENEMAINWINDOW_H

#include <QMainWindow>
#include "newgenewidget.h"

#include <memory>

namespace Ui {
class NewGeneMainWindow;
}

class NewGeneMainWindow : public QMainWindow, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
    Q_OBJECT

public:
    explicit NewGeneMainWindow(QWidget *parent = 0);
    ~NewGeneMainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::NewGeneMainWindow *ui;
    std::unique_ptr<NewGeneModel> model;

    friend class NewGeneWidget;
};

#endif // NEWGENEMAINWINDOW_H
