#ifndef MODELCHANGERESPONSEITEM_H
#define MODELCHANGERESPONSEITEM_H

#include <QObject>
#include "modelchangeitem.h"

class ModelChangeResponseItem : public ModelChangeItem
{
		Q_OBJECT
	public:
		explicit ModelChangeResponseItem( QObject * parent = 0 );

	signals:

	public slots:

};

#endif // MODELCHANGERESPONSEITEM_H
