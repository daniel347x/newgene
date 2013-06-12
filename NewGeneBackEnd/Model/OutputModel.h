#ifndef OUTPUTMODEL_H
#define OUTPUTMODEL_H

#include "Model.h"
#include "..\Settings\OutputModelSettings.h"
#include "..\Settings\Setting.h"

class OutputModel : public Model<OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS>
{

	public:

		OutputModel(Messager & messager, boost::filesystem::path const path_to_model_database)
			: Model(messager, path_to_model_database)
		{

		}

};

#endif
