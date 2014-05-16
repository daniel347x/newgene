#include "limit_dmus_region.h"

limit_dmus_region::limit_dmus_region(QWidget *parent) :
    QWidget(parent),
    ui( new Ui::NewGeneSelectVariables )
{
    ui->setupUi( this );
}

limit_dmus_region::~limit_dmus_region()
{
    delete ui;
}

void limit_dmus_region::changeEvent( QEvent * e )
{
    QWidget::changeEvent( e );

    switch ( e->type() )
    {
        case QEvent::LanguageChange:
            ui->retranslateUi( this );
            break;

        default:
            break;
    }
}
