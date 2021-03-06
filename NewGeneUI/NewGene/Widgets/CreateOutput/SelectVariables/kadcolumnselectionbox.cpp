#include "kadcolumnselectionbox.h"
#include "ui_kadcolumnselectionbox.h"
#include "../../Utilities/myclickablelabel.h"
#include "../../newgenemainwindow.h"
#include "../../newgenetabwidget.h"

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
		QString vgWarningText = spinnerScrollArea->getFullWarningTextAllVGs();
		QMessageBox::information(this, "Variable Group Warning/s", vgWarningText);
	}
	else
	{
		QMessageBox::information(this, "Variable Group Warning/s", labelText);
	}
}


void KAdColumnSelectionBox::on_checkBoxSimpleMode_stateChanged(int state)
{
	NewGeneMainWindow * mainWindow = theMainWindow;
	if (mainWindow == nullptr)
	{
		return;
	}
	mainWindow->ShowHideTabs(state);
}
