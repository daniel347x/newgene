#ifndef UIMODEL_H
#define UIMODEL_H

#include "globals.h"
#include <QObject>

class UIModel : public QObject
{
		Q_OBJECT
	public:
		explicit UIModel( QObject * parent = 0 );

	signals:

	public slots:

};

#endif // UIMODEL_H
