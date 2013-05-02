#ifndef NEWGENEVARIABLESTOOLBOX_H
#define NEWGENEVARIABLESTOOLBOX_H

#include <QToolBox>
#include "newgenevariablegroup.h"

class NewGeneVariablesToolbox : public QToolBox
{
    Q_OBJECT
public:
    explicit NewGeneVariablesToolbox(QWidget *parent = 0);
    
signals:
    
public slots:

private:
    NewGeneVariableGroup * groups;
    
};

#endif // NEWGENEVARIABLESTOOLBOX_H
