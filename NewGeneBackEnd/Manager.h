#ifndef MANAGER_H
#define MANAGER_H

#include <string>
#include <memory>
#include "globals.h"
#include "./Messager/Messager.h"

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
		, MANAGER_THREADS
		, MANAGER_TRIGGERS
		, MANAGER_UI_DATA
		, MANAGER_UI_ACTION
		, MANAGER_MODEL_ACTION
	};

	std::string get_text_name_from_enum(MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER const which_manager);

}

template<typename MANAGER_CLASS, MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER MANAGER_ENUM>
class Manager
{

	public:

		explicit Manager(Messager & messager_)
			: messager(messager_)
		{

		}

		virtual ~Manager()
		{

		}

		static Manager<MANAGER_CLASS, MANAGER_ENUM> & getManager()
		{

			if ( _manager == NULL )
			{
				boost::format msg( "Manager \"%1%\" not instantiated." );
				msg % MANAGER_DESCRIPTION_NAMESPACE::get_text_name_from_enum(MANAGER_ENUM);
				throw NewGeneException() << newgene_error_description( msg.str() );
			}

			return *_manager;

		}

		// **********************************************************************************************************
		// The following member variables
		// cannot be made protected because there is no way for this backend class to give friendship to the UI class
		// (non-type template parameter "MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER_UI" is not defined here)
		// **********************************************************************************************************
		static std::unique_ptr<MANAGER_CLASS> _manager;
		MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER which;
		std::string which_descriptor;

		Messager & messager;

};

template<typename MANAGER_CLASS, MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER MANAGER_ENUM>
std::unique_ptr<MANAGER_CLASS> Manager<MANAGER_CLASS, MANAGER_ENUM>::_manager;

#endif
