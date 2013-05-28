#ifndef UIMODEL_H
#define UIMODEL_H

//#include "globals.h"
#include "uimessager.h"
#include <QObject>
#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif

template<typename BACKEND_MODEL_CLASS>
class UIModel
{
	public:
		UIModel( UIMessager & messager, BACKEND_MODEL_CLASS & model, boost::filesystem::path const path_to_model )
		{

		}

	signals:

	public slots:

	public:

		BACKEND_MODEL_CLASS & getBackendModel()
		{
			return *_backend_model;
		}

		std::shared_ptr<BACKEND_MODEL_CLASS> & getBackendModelSharedPtr()
		{
			return _backend_model;
		}

	protected:

	std::shared_ptr<BACKEND_MODEL_CLASS> _backend_model;

};

#endif // UIMODEL_H
