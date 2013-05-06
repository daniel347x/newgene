#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <QObject>

class NewGeneMainWindow;

class UIManager : public QObject
{
    Q_OBJECT
public:
    explicit UIManager(NewGeneMainWindow *parent = 0);

signals:

public slots:

};

#endif // UIMANAGER_H
