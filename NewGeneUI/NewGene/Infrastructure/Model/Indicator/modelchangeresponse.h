#ifndef MODELCHANGERESPONSE_H
#define MODELCHANGERESPONSE_H

#include <QObject>
#include "modelchangeindicator.h"

class ModelChangeResponse : public ModelChangeIndicator
{
		Q_OBJECT
	public:
		explicit ModelChangeResponse(QObject * parent = 0);

	signals:

	public slots:

};

#endif // MODELCHANGERESPONSE_H
