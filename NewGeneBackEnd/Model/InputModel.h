#ifndef INPUTMODEL_H
#define INPUTMODEL_H

#include "Model.h"
#include "..\Settings\InputModelSettings.h"
#include "..\Settings\Setting.h"

class InputModel : public Model<INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS>
{

	public:

		InputModel(Messager & messager, boost::filesystem::path const path_to_model_database)
			: Model(messager, path_to_model_database)
		{

		}

};

#endif
