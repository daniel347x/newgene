#ifndef NEWGENEVARIABLESTOOLBOXWRAPPER_H
#define NEWGENEVARIABLESTOOLBOXWRAPPER_H

#include <QWidget>
#include "..\..\..\newgenewidget.h"
#include <QGridLayout>
#include "newgenevariablestoolbox.h"

namespace Ui
{
	class NewGeneVariablesToolboxWrapper;
}

class NewGeneVariablesToolboxWrapper : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:
		explicit NewGeneVariablesToolboxWrapper( QWidget * parent = 0 );
		~NewGeneVariablesToolboxWrapper();

	protected:

	private:
		QGridLayout * gridLayout;
		NewGeneVariablesToolbox * newgeneToolBox;
};

#endif // NEWGENEVARIABLESTOOLBOXWRAPPER_H
