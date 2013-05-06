#ifndef UISETTINGS_H
#define UISETTINGS_H

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

};

#endif // UISETTINGS_H
