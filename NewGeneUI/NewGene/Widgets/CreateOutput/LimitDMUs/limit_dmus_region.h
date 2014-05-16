#ifndef LIMIT_DMUS_REGION_H
#define LIMIT_DMUS_REGION_H

#include <QWidget>
#include "../../newgenewidget.h"

namespace Ui
{
    class limit_dmus_region;
}

class limit_dmus_region : public QWidget
{

        Q_OBJECT

    public:

        explicit limit_dmus_region(QWidget *parent = 0);
        ~limit_dmus_region();

    protected:

        void changeEvent( QEvent * e );

    private:

        Ui::limit_dmus_region * ui;

    signals:

    public slots:

};

#endif // LIMIT_DMUS_REGION_H
