#ifndef UIPROJECTOUTPUTSETTINGS_H
#define UIPROJECTOUTPUTSETTINGS_H

#include "../../../NewGeneBackEnd/Settings/OutputProjectSettings.h"
#include "../../../NewGeneBackEnd/Settings/Setting.h"
#include "../../../NewGeneBackEnd/Project/OutputProject.h"
#include "uiallprojectsettings.h"
#include "Base/UISetting.h"
#include "outputprojectsettingsworkqueue.h"

namespace OUTPUT_PROJECT_SETTINGS_UI_NAMESPACE
{

	enum OUTPUT_PROJECT_SETTINGS_UI
	{
		  SETTING_FIRST = 0
		, SETTING_LAST
	};

}

class UIOutputProjectSettings : public QObject, public UIAllProjectSettings<OutputProjectSettings, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND, OUTPUT_PROJECT_SETTINGS_UI_NAMESPACE::OUTPUT_PROJECT_SETTINGS_UI, BackendProjectOutputSetting, UIProjectOutputSetting, UI_OUTPUT_PROJECT_SETTINGS>
{

		Q_OBJECT

	signals:

	public slots:


	public:
		UIOutputProjectSettings(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path(), QObject * parent = NULL)
			: QObject(parent)
			, UIAllProjectSettings(messager, path_to_settings, OutputProjectSettings::number_worker_threads)
		{

		}

		void SetMapEntry(Messager & messager_, SettingInfo & setting_info, boost::property_tree::ptree & pt);
		UIProjectOutputSetting * CloneSetting(Messager & messager_, UIProjectOutputSetting * current_setting, SettingInfo & setting_info) const;
		UIProjectOutputSetting * NewSetting(Messager & messager_, SettingInfo & setting_info, void const * setting_value_void = NULL);
		void SetPTreeEntry(Messager & messager, OUTPUT_PROJECT_SETTINGS_UI_NAMESPACE::OUTPUT_PROJECT_SETTINGS_UI which_setting, boost::property_tree::ptree & pt);

		virtual void WriteSettingsToFile(Messager & messager)
		{
			boost::property_tree::ptree pt;
			UIOnlySettings & uisettings = getUISettings();
			OutputProjectSettings & backendsettings = getBackendSettings();
			uisettings.WriteSettingsToPtree(messager, pt);
			backendsettings.WriteSettingsToPtree(messager, pt);
			uisettings.WritePtreeToFile(messager, pt);
		}

		void UpdateConnections();

	protected:

		WorkQueueManager<UI_OUTPUT_PROJECT_SETTINGS> * InstantiateWorkQueue(void * ui_object, bool isPool2_ = false)
		{
			OutputProjectSettingsWorkQueue * work_queue = new OutputProjectSettingsWorkQueue();
			work_queue->SetUIObject(reinterpret_cast<UIOutputProjectSettings*>(ui_object));
			work_queue->SetConnections();
			return work_queue;
		}

	public slots:
		void SignalMessageBox(STD_STRING);

};

#endif // UIPROJECTOUTPUTSETTINGS_H
