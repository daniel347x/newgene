#ifndef UISTATUSMANAGER_H
#define UISTATUSMANAGER_H

#include <QObject>

class UIStatusManager : public QObject
{
    Q_OBJECT
public:
    explicit UIStatusManager(QObject *parent = 0);

    static UIStatusManager * getStatusManager();

signals:

public slots:

private:
    static UIStatusManager * status_;

};

#endif // UISTATUSMANAGER_H
