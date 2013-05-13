#ifndef UISETTINGS_H
#define UISETTINGS_H

#include <QObject>
#include "globals.h"
#include "../../../NewGeneBackEnd/Settings/Settings.h"

class UISettings : public QObject
{
		Q_OBJECT
	public:
		explicit UISettings(QObject *parent = 0);

	signals:

	public slots:

	protected:

		std::unique_ptr<Settings> backend_settings;

};

#endif // UISETTINGS_H
