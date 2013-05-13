#ifndef UISETTINGS_H
#define UISETTINGS_H

#include <QObject>
#include "../../../NewGeneBackEnd/Settings/Settings.h"

class UISettings : public QObject
{
		Q_OBJECT
	public:
		explicit UISettings(QObject *parent = 0);

	signals:

	public slots:

};

#endif // UISETTINGS_H
