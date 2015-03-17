#include "kadcolumnselectionbox.h"
#include "ui_kadcolumnselectionbox.h"

KAdColumnSelectionBox::KAdColumnSelectionBox(QWidget * parent) :
	QFrame(parent),
	NewGeneWidget(WidgetCreationInfo(this,
									 WIDGET_NATURE_OUTPUT_WIDGET)),   // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui(new Ui::KAdColumnSelectionBox)
{
	ui->setupUi(this);
}

KAdColumnSelectionBox::~KAdColumnSelectionBox()
{
	delete ui;
}

void KAdColumnSelectionBox::changeEvent(QEvent * e)
{
	QFrame::changeEvent(e);

	switch (e->type())
	{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;

		default:
			break;
	}
}
