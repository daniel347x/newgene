#include "ModelManager.h"
#include "InputModel.h"
#include "OutputModel.h"

bool ModelManager::ImportRawData(Messager & messager, Model_basemost * model_, TABLE_TYPES const table_type, boost::filesystem::path const & path_to_raw_data)
{
	try
	{
		InputModel * input_model = nullptr;
		OutputModel * output_model = nullptr;
		switch (table_type)
		{
			case TABLE__DMU_IDENTIFIER:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__DMU_INSTANCE:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__CMU_IDENTIFIER:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__CMU_INSTANCE:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__UOA_IDENTIFIER:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__UOA_MEMBER:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__VG_CATEGORY:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__VG_SET_MEMBER:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__VG_SET_MEMBER_SELECTED:
				{
					OutputModel * output_model = dynamic_cast<OutputModel*>(model_);
					if (output_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__KAD_COUNT:
				{
					OutputModel * output_model = dynamic_cast<OutputModel*>(model_);
					if (output_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
		}
		switch (table_type)
		{
			case TABLE__DMU_IDENTIFIER:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__DMU_INSTANCE:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__CMU_IDENTIFIER:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__CMU_INSTANCE:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__UOA_IDENTIFIER:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__UOA_MEMBER:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__VG_CATEGORY:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__VG_SET_MEMBER:
				{
					InputModel * input_model = dynamic_cast<InputModel*>(model_);
					if (input_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__VG_SET_MEMBER_SELECTED:
				{
					OutputModel * output_model = dynamic_cast<OutputModel*>(model_);
					if (output_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__KAD_COUNT:
				{
					OutputModel * output_model = dynamic_cast<OutputModel*>(model_);
					if (output_model)
					{

					}
					else
					{
						// Todo: log warning
						return false;
					}
				}
				break;
		}
	}
	catch (std::bad_cast &)
	{
		// Todo: log warning
		return false;
	}
	return true;
}
