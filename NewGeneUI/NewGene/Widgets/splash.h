#ifndef SPLASH_H
#define SPLASH_H

#include <QWidget>
#include <QUrl>
#include "Widgets/newgenemainwindow.h"

namespace Ui {
class Splash;
}

class Splash : public QWidget
{
    Q_OBJECT

public:
    explicit Splash(QWidget *parent, NewGeneMainWindow * mainWindow_, bool const opened_as_about_box_);
    ~Splash();

    bool closed_via_click;
    bool opened_as_about_box;

private slots:

    void closeAndRefreshSequence();
    void deleteMe();
    void on_pushButton_clicked();
    void receiveLinkClicked(const QUrl &);

private:
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent * event);
    void closeMyself();

    Ui::Splash *ui;
    NewGeneMainWindow * mainWindow;
};

#endif // SPLASH_H
