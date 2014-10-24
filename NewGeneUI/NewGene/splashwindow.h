#ifndef SPLASHWINDOW_H
#define SPLASHWINDOW_H

#include "globals.h"
#include <QTimer>
#include <QQuickWidget>
#include <QQmlEngine>
#include <QQmlContext>
#include <Qurl>
#include "Widgets/newgenemainwindow.h"

class SplashWindow : public QQuickWidget
{
    Q_OBJECT

    public:
        Q_INVOKABLE void Close()
        {
            QTimer::singleShot( 1000, this, SLOT( close() ) );
            QTimer::singleShot( 2000, theMainWindow, SLOT( show() ) );
        }

    protected:
        bool eventFilter(QObject *obj, QEvent *event)
        {
            bool ret = QQuickWidget::eventFilter(obj, event);
            if (event->type() == QEvent::MouseButtonRelease)
            {
                //QTimer::singleShot( 1000, this, SLOT( close() ) );
                //QTimer::singleShot( 2000, theMainWindow, SLOT( show() ) );
            }
            return ret;
        }
};

#endif // SPLASHWINDOW_H
