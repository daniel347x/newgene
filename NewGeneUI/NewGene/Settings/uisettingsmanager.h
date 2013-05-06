#ifndef UISETTINGS_H
#define UISETTINGS_H

#include "globals.h"
#include "newgenemainwindow.h"
#ifndef Q_MOC_RUN
#   include <boost\filesystem.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <boost/property_tree/xml_parser.hpp>
#endif
#include <QObject>
#include <QString>

class NewGeneMainWindow;

class UISettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit UISettingsManager(QObject *parent = 0);

    static UISettingsManager * getSettingsManager(NewGeneMainWindow * parent = NULL);

    static QString settingsFileName;

signals:

public slots:

protected:
    bool ObtainSettingsPath();

private:
    static UISettingsManager * settings_;

    bool dirty;
    boost::filesystem::path settingsPath;
    boost::property_tree::ptree settings;

};

#endif // UISETTINGS_H
