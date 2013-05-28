#ifndef UIOUTPUTMODEL_H
#define UIOUTPUTMODEL_H

#include "../../../NewGeneBackEnd/Model/OutputModel.h"
#include "uimodel.h"

class UIOutputModel : public UIModel<OutputModel>
{
	public:
		UIOutputModel(Messager & messager, OutputModel & model, boost::filesystem::path const path_to_model = boost::filesystem::path());
};

#endif // UIOUTPUTMODEL_H
