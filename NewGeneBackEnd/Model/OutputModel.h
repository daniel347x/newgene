#ifndef OUTPUTMODEL_H
#define OUTPUTMODEL_H

#include "Model.h"
#include "..\Settings\OutputModelSettings.h"
#include "..\Settings\InputModelSettings.h"
#include "InputModel.h"
#include "..\Settings\Setting.h"
#include "Tables/TableManager.h"
#include <memory>

class OutputModel : public Model<OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS>
{

	public:

		OutputModel(Messager & messager, boost::filesystem::path const path_to_model_database, std::shared_ptr<InputModelSettings> const & input_model_settings_, std::shared_ptr<InputModel> const & input_model_)
			: Model(messager, path_to_model_database)
			, input_model_settings(input_model_settings_)
			, input_model(input_model_)
		{

		}

		InputModelSettings & getInputModelSettings()
		{
			return *input_model_settings;
		}

		InputModel & getInputModel()
		{
			return *input_model;
		}

		void LoadTables();

		Table_VARIABLES_SELECTED t_variables_selected_identifiers;
		Table_KAD_COUNT t_kad_count;

protected:

		std::shared_ptr<InputModelSettings> input_model_settings;
		std::shared_ptr<InputModel> input_model;

};

#endif
