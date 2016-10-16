#include "kadcolumnselectionbox.h"
#include "ui_kadcolumnselectionbox.h"
#include "../../Utilities/myclickablelabel.h"

KAdColumnSelectionBox::KAdColumnSelectionBox(QWidget * parent) :
	QFrame(parent),
	NewGeneWidget(WidgetCreationInfo(this,
									 WIDGET_NATURE_OUTPUT_WIDGET)),   // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui(new Ui::KAdColumnSelectionBox)
{
	ui->setupUi(this);

	MyClickableLabel * vgWarningLabel { findChild<MyClickableLabel *>("labelVariableGroupWarning") };
	if (vgWarningLabel)
	{
		QObject::connect(vgWarningLabel, SIGNAL(clicked(QString const &)), this, SLOT(popupWarning(QString const &)));
	}
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

void KAdColumnSelectionBox::popupWarning(QString const & labelText)
{
	KadWidgetsScrollArea * spinnerScrollArea { findChild<KadWidgetsScrollArea *>("scrollAreaWidgetContents") };
	if (spinnerScrollArea)
	{
		QString vgWarningText = spinnerScrollArea->getFullWarningText(true);
		QMessageBox::information(this, "Variable Group Warning", vgWarningText);
	}
	else
	{
		QMessageBox::information(this, "Variable Group Warning", labelText);
	}
}

