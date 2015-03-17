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
            QTimer::singleShot( 100, parent(), SLOT( doEnable() ) );
            QTimer::singleShot( 10, this, SLOT( deleteMe() ) );

            if (opened_as_about_box)
            {
                //QTimer::singleShot( 1000, theMainWindow, SLOT( doEnable() ) );
            }
            else
            {
                //QTimer::singleShot( 500, theMainWindow, SLOT( show() ) );
                //QTimer::singleShot( 1000, theMainWindow, SLOT( doInitialize() ) );
            }
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
            if (event->type() == QEvent::MouseButtonRelease)
            {
                //QTimer::singleShot( 1000, this, SLOT( close() ) );
                //QTimer::singleShot( 2000, theMainWindow, SLOT( show() ) );
            }
            return ret;

        }

        void closeEvent(QCloseEvent * event)
        {
            if (!closed_via_click && !opened_as_about_box)
            {
                //QApplication::quit();
            }
            event->accept();
        }

    public:

        bool closed_via_click;
        bool opened_as_about_box;

    private slots:

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
