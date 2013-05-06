#ifndef UISTATUSMANAGER_H
#define UISTATUSMANAGER_H

#include "globals.h"
#include "uimanager.h"

class NewGeneMainWindow;

class UIStatusManager : public UIManager
{
    Q_OBJECT

public:

    enum IMPORTANCE
    {
          IMPORTANCE_DEBUG
        , IMPORTANCE_STANDARD
        , IMPORTANCE_HIGH
        , IMPORTANCE_CRITICAL
    };

    explicit UIStatusManager(NewGeneMainWindow *parent = 0);

    static UIStatusManager * getStatusManager(NewGeneMainWindow * parent = NULL);

    void LogStatus(QString const & status_, IMPORTANCE const importance_level = IMPORTANCE_STANDARD);
    void PostStatus(QString const & status_, IMPORTANCE const importance_level = IMPORTANCE_STANDARD);

signals:

public slots:

private:
    static UIStatusManager * status_;

};

#endif // UISTATUSMANAGER_H
