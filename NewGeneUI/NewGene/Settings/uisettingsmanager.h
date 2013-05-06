#ifndef UISETTINGS_H
#define UISETTINGS_H

#include "globals.h"
#ifndef Q_MOC_RUN
#   include <boost\filesystem.hpp>
#endif
#include <QObject>

class UISettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit UISettingsManager(QObject *parent = 0);

    static UISettingsManager * getSettingsManager();

signals:

public slots:

private:
    static UISettingsManager * settings_;

    bool dirty;
    boost::filesystem::path settingsPath;
};

#endif // UISETTINGS_H
