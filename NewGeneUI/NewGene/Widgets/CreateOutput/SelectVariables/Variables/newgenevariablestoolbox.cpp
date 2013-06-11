#include "newgenevariablestoolbox.h"

#include <QLayout>

NewGeneVariablesToolbox::NewGeneVariablesToolbox( QWidget * parent ) :
	QToolBox( parent ),
	NewGeneWidget( this ) // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
{
	layout()->setSpacing( 1 );

	groups = new NewGeneVariableGroup( this );
	addItem( groups, "Country Variables" );

	NewGeneVariableGroup * tmpGrp = new NewGeneVariableGroup( this );
	addItem( tmpGrp, "MID Detail Variables" );
}
