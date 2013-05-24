#ifndef UIMODEL_H
#define UIMODEL_H

#include "globals.h"
#include <QObject>

template<typename MODEL_CLASS>
class UIModel : public QObject
{
		Q_OBJECT
	public:
		explicit UIModel( Messager & messager, MODEL_CLASS & model, boost::filesystem::path const path_to_model, QObject * parent = 0 );

	signals:

	public slots:

	protected:

	std::shared_ptr<BACKEND_PROJECT_CLASS > _backend_project;

};

#endif // UIMODEL_H
