#ifndef SPLASHWINDOW_H
#define SPLASHWINDOW_H

#include "globals.h"
#include <QTimer>
#include <QQuickWidget>
#include <QQmlEngine>
#include <QQmlContext>
#include <Qurl>
#include <QApplication>
#include "Widgets/newgenemainwindow.h"

class SplashWindow : public QQuickWidget
{
    Q_OBJECT

    public:

        SplashWindow(QWidget * parent_, bool const opened_as_about_box_) : QQuickWidget{parent_}, closed_via_click{false}, opened_as_about_box{opened_as_about_box_}
        {
            QTimer::singleShot( 10, parent(), SLOT( doDisable() ) );
            connect(this, SIGNAL(statusChanged(QQuickWidget::Status)), this, SLOT(receiveStatusChanged(QQuickWidget::Status)));
        }

        Q_INVOKABLE void close_window()
        {
            closed_via_click = true;
            QTimer::singleShot( 10, this, SLOT( close() ) );
            QTimer::singleShot( 1000, this, SLOT( closeAndRefreshSequence() ) );
        }

        Q_INVOKABLE void setCursorNormal()
        {
            setCursor(Qt::ArrowCursor);
        }

        Q_INVOKABLE void setCursorLink()
        {
            setCursor(Qt::PointingHandCursor);
        }

    protected:
        bool eventFilter(QObject *obj, QEvent *event)
        {
            bool ret = QQuickWidget::eventFilter(obj, event);
            return ret;

        }

        void closeEvent(QCloseEvent * event)
        {
            if (!closed_via_click)
            {
                closeAndRefreshSequence();
            }
            event->accept();
        }

    public:

        bool closed_via_click;
        bool opened_as_about_box;

    private slots:

        void closeAndRefreshSequence()
        {
            // Deal with visual artifacts - I'm not sure why the window needs to be repainted,
            // but repainting to remove artifacts after the splash screen has been removed
            // only works after the splash screen has been deleted, and there is a safety delay
            // prior to deleting it.
            QTimer::singleShot( 0, this, SLOT( deleteMe() ) );
            QTimer::singleShot( 100, parent(), SLOT( doEnable() ) );
            QTimer::singleShot( 1000, parent(), SLOT( update() ) );
        }

        void receiveStatusChanged(QQuickWidget::Status status)
        {
            if (status == QQuickWidget::Status::Error)
            {
            }
        }

        void deleteMe()
        {
            deleteLater();
        }

};

#endif // SPLASHWINDOW_H
