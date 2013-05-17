#ifndef MODELCHANGEREQUEST_H
#define MODELCHANGEREQUEST_H

#include "modelchangeindicator.h"

class ModelChangeRequest : public ModelChangeIndicator
{
		Q_OBJECT
	public:
		explicit ModelChangeRequest( QObject * parent = 0 );

	signals:

	public slots:

};

#endif // MODELCHANGEREQUEST_H
