#ifndef MODELSETTINGS_H
#define MODELSETTINGS_H

#include "Settings.h"
#include "Setting.h"
#include <map>

template<typename MODEL_SETTINGS_ENUM, typename MODEL_SETTING_CLASS>
class ModelSettings : public Settings<MODEL_SETTINGS_ENUM, MODEL_SETTING_CLASS>
{

public:

	ModelSettings(Messager & messager, boost::filesystem::path const model_settings_path)
		: Settings(messager, model_settings_path)
	{

	}

	virtual ~ModelSettings() {}


protected:

};

#endif
