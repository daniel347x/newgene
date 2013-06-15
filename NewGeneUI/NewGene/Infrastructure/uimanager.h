#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <QObject>
#include <QString>
#include <QApplication>
#include <memory>
#include "..\..\NewGeneBackEnd\Manager.h"

class NewGeneMainWindow;
class NewGeneWidget;

namespace MANAGER_DESCRIPTION_NAMESPACE
{

	enum WHICH_MANAGER_UI
	{
		  MANAGER_DOCUMENTS_UI
		, MANAGER_SETTINGS_UI
		, MANAGER_STATUS_UI
		, MANAGER_MODEL_UI
		, MANAGER_LOGGING_UI
		, MANAGER_PROJECT_UI
		, MANAGER_THREADS_UI
		, MANAGER_TRIGGERS_UI
		, MANAGER_UI_DATA_UI
		, MANAGER_UI_ACTION_UI
		, MANAGER_MODEL_ACTION_UI
	};

	std::string get_text_name_from_enum_ui(MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER_UI const which_manager);

}

template<typename MANAGER_CLASS_UI, typename MANAGER_CLASS_BACKEND, MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER_UI MANAGER_ENUM_UI, MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER MANAGER_ENUM_BACKEND>
class UIManager
{

	public:

		explicit UIManager()
		{
			MANAGER_CLASS_BACKEND::_manager.reset(new MANAGER_CLASS_BACKEND);
			MANAGER_CLASS_BACKEND::getManager().which = MANAGER_ENUM_BACKEND;
			MANAGER_CLASS_BACKEND::getManager().which_descriptor = MANAGER_DESCRIPTION_NAMESPACE::get_text_name_from_enum(MANAGER_ENUM_BACKEND).c_str();
		}

		virtual ~UIManager()
		{
			MANAGER_CLASS_BACKEND::_manager.reset();
		}

		NewGeneMainWindow & getMainWindow()
		{
			if ( theMainWindow == NULL )
			{
				boost::format msg( "Main window does not exist in %1%" );
				msg % which_descriptor.toStdString();
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
			return *theMainWindow;
		}

		static UIManager<MANAGER_CLASS_UI, MANAGER_CLASS_BACKEND, MANAGER_ENUM_UI, MANAGER_ENUM_BACKEND> & getManager()
		{

			if ( _ui_manager == NULL )
			{
				_ui_manager.reset( new MANAGER_CLASS_UI() );
				if ( _ui_manager )
				{
					_ui_manager->which = MANAGER_ENUM_UI;
					_ui_manager->which_descriptor = MANAGER_DESCRIPTION_NAMESPACE::get_text_name_from_enum_ui(MANAGER_ENUM_UI).c_str();
				}
			}

			if ( _ui_manager == NULL )
			{
				boost::format msg( "Manager \"%1%\" not instantiated." );
				msg % MANAGER_DESCRIPTION_NAMESPACE::get_text_name_from_enum_ui(MANAGER_ENUM_UI);
				throw NewGeneException() << newgene_error_description( msg.str() );
			}

			return *_ui_manager;

		}

		static MANAGER_CLASS_BACKEND & getBackendManager()
		{
			MANAGER_CLASS_UI & ui_manager = getManager();
			return MANAGER_CLASS_BACKEND::getManager();
		}

	protected:
		MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER_UI which;
		QString which_descriptor;

	protected:

		friend class NewGeneWidget;

	protected:
		static std::unique_ptr<MANAGER_CLASS_UI> _ui_manager;

};

template<typename MANAGER_CLASS_UI, typename MANAGER_CLASS_BACKEND, MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER_UI MANAGER_ENUM_UI, MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER MANAGER_ENUM_BACKEND>
std::unique_ptr<MANAGER_CLASS_UI> UIManager<MANAGER_CLASS_UI, MANAGER_CLASS_BACKEND, MANAGER_ENUM_UI, MANAGER_ENUM_BACKEND>::_ui_manager;

#endif // UIMANAGER_H
