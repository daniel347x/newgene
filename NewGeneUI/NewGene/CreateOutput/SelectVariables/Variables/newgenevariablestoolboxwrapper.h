#ifndef NEWGENEVARIABLESTOOLBOXWRAPPER_H
#define NEWGENEVARIABLESTOOLBOXWRAPPER_H

#include <QWidget>
#include <QGridLayout>
#include "newgenevariablestoolbox.h"

namespace Ui {
class NewGeneVariablesToolboxWrapper;
}

class NewGeneVariablesToolboxWrapper : public QWidget
{
    Q_OBJECT
    
public:
    explicit NewGeneVariablesToolboxWrapper(QWidget *parent = 0);
    ~NewGeneVariablesToolboxWrapper();
    
protected:
    
private:
    QGridLayout * gridLayout;
    NewGeneVariablesToolbox * newgeneToolBox;
};

#endif // NEWGENEVARIABLESTOOLBOXWRAPPER_H
