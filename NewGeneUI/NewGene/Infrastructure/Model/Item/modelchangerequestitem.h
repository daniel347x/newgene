#ifndef MODELCHANGEREQUESTITEM_H
#define MODELCHANGEREQUESTITEM_H

#include <QObject>
#include "modelchangeitem.h"

class ModelChangeRequestItem : public ModelChangeItem
{
		Q_OBJECT
	public:
		explicit ModelChangeRequestItem( QObject * parent = 0 );

	signals:

	public slots:

};

#endif // MODELCHANGEREQUESTITEM_H
