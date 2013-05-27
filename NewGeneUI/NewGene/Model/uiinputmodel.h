#ifndef UIINPUTMODEL_H
#define UIINPUTMODEL_H

#include "../../../NewGeneBackEnd/Model/InputModel.h"
#include "uimodel.h"

class UIInputModel : public UIModel<InputModel>
{
	public:
		UIInputModel(Messager & messager, InputModel & model, boost::filesystem::path const path_to_model = boost::filesystem::path());
};

#endif // UIINPUTMODEL_H
