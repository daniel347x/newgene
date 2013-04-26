#ifndef NEWGENETABWIDGET_H
#define NEWGENETABWIDGET_H

#include <QTabWidget>

class NewGeneTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit NewGeneTabWidget(QWidget *parent = 0);
    
signals:
    
public slots:

public:
    void NewGeneInitialize();
    
};

#endif // NEWGENETABWIDGET_H
