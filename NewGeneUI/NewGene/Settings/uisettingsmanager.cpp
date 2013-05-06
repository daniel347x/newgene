#include "uisettingsmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include <QStandardPaths>
#include <fstream>
//#include <QDebug>

QString UISettingsManager::settingsFileName = "NewGene.Settings.xml";
UISettingsManager * UISettingsManager::settings_ = NULL;

UISettingsManager::UISettingsManager(QObject *parent) :
    QObject(parent),
    dirty(false)
{
    QStringList settingsPathStringList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    bool found = false;
    for (int n=0; n<settingsPathStringList.size(); ++n)
    {
        QString settingsPathString = settingsPathStringList.at(n);
        boost::filesystem::path settingsPathTest(settingsPathString.toStdString());
        settingsPathTest /= settingsFileName.toStdString();
        if (boost::filesystem::exists(settingsPathTest) && boost::filesystem::is_regular_file(settingsPathTest))
        {
            settingsPath = settingsPathTest;
            found = true;
            break;
        }
    }
    if (!found)
    {
        for (int n=0; n<settingsPathStringList.size(); ++n)
        {
            QString settingsPathString = settingsPathStringList.at(n);
            boost::filesystem::path settingsPathTest(settingsPathString.toStdString());
            settingsPathTest /= settingsFileName.toStdString();
            std::ofstream settingsPathTestFile;
            settingsPathTestFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            try
            {
                settingsPathTestFile.open(settingsPathTest.c_str());
                if (settingsPathTestFile.is_open())
                {
                    settingsPathTestFile.write("\n", 1); // write 1 character
                    settingsPathTestFile.close();
                    settingsPath = settingsPathTest;
                    found = true;
                }
                else
                {
                    continue;
                }
            }
            catch (std::ofstream::failure e)
            {
                continue;
            }
        }
    }

    if (found)
    {
        //ReadSettings();
    }
}

UISettingsManager * UISettingsManager::getSettingsManager()
{
    if (settings_ == NULL)
    {
        settings_ = new UISettingsManager;
    }
    if (settings_ == NULL)
    {
        boost::format msg("Settings manager not instantiated.");
        throw NewGeneException() << newgene_error_description(msg.str());
    }
    return settings_;
}
