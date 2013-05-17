#ifndef MODELCHANGEINDICATOR_H
#define MODELCHANGEINDICATOR_H

#include <QObject>

class ModelChangeIndicator : public QObject
{
		Q_OBJECT
	public:
		explicit ModelChangeIndicator( QObject * parent = 0 );

	signals:

	public slots:

};

#endif // MODELCHANGEINDICATOR_H
