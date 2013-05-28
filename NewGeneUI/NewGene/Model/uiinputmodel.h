#ifndef UIINPUTMODEL_H
#define UIINPUTMODEL_H

#include "../../../NewGeneBackEnd/Model/InputModel.h"
#include "uimodel.h"

class UIInputModel : public QObject, public UIModel<InputModel>
{

		Q_OBJECT

	signals:

	public slots:


	public:
		UIInputModel(Messager & messager, InputModel & model, boost::filesystem::path const path_to_model = boost::filesystem::path(), QObject * parent = NULL);
};

#endif // UIINPUTMODEL_H
