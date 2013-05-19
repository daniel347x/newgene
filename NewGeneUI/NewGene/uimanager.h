#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <QObject>
#include <QString>
//#include "newgenemainwindow.h"
#include <QApplication>
#include <memory>

class NewGeneMainWindow;
class NewGeneWidget;

namespace MANAGER_DESCRIPTION_NAMESPACE
{
	enum WHICH_MANAGER
	{
		  MANAGER_DOCUMENTS
		, MANAGER_SETTINGS
		, MANAGER_STATUS
		, MANAGER_MODEL
		, MANAGER_LOGGING
		, MANAGER_PROJECT
	};
}

template<typename MANAGER_CLASS_UI, typename MANAGER_CLASS_BACKEND, MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER MANAGER_ENUM>
class UIManager
{

	public:

		explicit UIManager()
		{
			_backend_manager.reset(new MANAGER_CLASS_BACKEND);
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

		static UIManager<MANAGER_CLASS_UI, MANAGER_CLASS_BACKEND, MANAGER_ENUM> & getManager()
		{

			if ( _ui_manager == NULL )
			{
				_ui_manager.reset( new MANAGER_CLASS_UI() );
				if ( _ui_manager )
				{
					_ui_manager->which = MANAGER_ENUM;
					_ui_manager->which_descriptor = get_text_name_from_enum(MANAGER_ENUM).c_str();
				}
			}

			if ( _ui_manager == NULL )
			{
				boost::format msg( "Manager not instantiated." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}

			return *_ui_manager;

		}

	protected:
		MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER which;
		QString which_descriptor;

	protected:

		friend class NewGeneWidget;

	protected:
		static std::unique_ptr<MANAGER_CLASS_UI> _ui_manager;
		std::unique_ptr<MANAGER_CLASS_BACKEND> _backend_manager;

	private:
		static std::string get_text_name_from_enum(MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER const which_manager)
		{
			if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_DOCUMENTS)
			{
				return "UIDocumentManager";
			}
			else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_LOGGING)
			{
				return "UILoggingManager";
			}
			else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_MODEL)
			{
				return "UIModelManager";
			}
			else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT)
			{
				return "UIProjectManager";
			}
			else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_SETTINGS)
			{
				return "UISettingsManager";
			}
			else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_STATUS)
			{
				return "UIStatusManager";
			}
			return std::string();
		}

};

template<typename MANAGER_CLASS_UI, typename MANAGER_CLASS_BACKEND, MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER MANAGER_ENUM>
std::unique_ptr<MANAGER_CLASS_UI> UIManager<MANAGER_CLASS_UI, MANAGER_CLASS_BACKEND, MANAGER_ENUM>::_ui_manager;

#endif // UIMANAGER_H
