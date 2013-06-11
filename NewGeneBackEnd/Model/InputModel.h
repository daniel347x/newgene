#ifndef INPUTMODEL_H
#define INPUTMODEL_H

#include "Model.h"
#include "..\Settings\InputModelSettings.h"
#include "..\Settings\Setting.h"

class InputModel : public Model<INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS, InputModelSetting>
{

	public:

		InputModel(Messager & messager, InputModelSettings * model_settings)
			: Model(messager, model_settings)
		{

		}

};

#endif
