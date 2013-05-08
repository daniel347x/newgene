#ifndef UIGLOBALSETTINGS_H
#define UIGLOBALSETTINGS_H

#include <QObject>
#include "uisettings.h"

class UIGlobalSettings : public UISettings
{
		Q_OBJECT
	public:
		explicit UIGlobalSettings(QObject *parent = 0);

	signals:

	public slots:

};

#endif // UIGLOBALSETTINGS_H
