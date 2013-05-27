#ifndef UIMODEL_H
#define UIMODEL_H

#include "globals.h"
#include <QObject>

template<typename BACKEND_MODEL_CLASS>
class UIModel
{
	public:
		UIModel( Messager & messager, BACKEND_MODEL_CLASS & model, boost::filesystem::path const path_to_model )
		{

		}

	signals:

	public slots:

	protected:

	std::shared_ptr<BACKEND_MODEL_CLASS > _backend_model;

};

#endif // UIMODEL_H
