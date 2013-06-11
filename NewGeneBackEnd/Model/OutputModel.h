#ifndef OUTPUTMODEL_H
#define OUTPUTMODEL_H

#include "Model.h"
#include "..\Settings\OutputModelSettings.h"
#include "..\Settings\Setting.h"

class OutputModel : public Model<OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS, OutputModelSetting>
{

	public:

		OutputModel(Messager & messager, OutputModelSettings * model_settings)
			: Model(messager, model_settings)
		{

		}

};

#endif
