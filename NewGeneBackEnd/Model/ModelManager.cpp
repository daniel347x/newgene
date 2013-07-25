#include "ModelManager.h"
#include "InputModel.h"
#include "OutputModel.h"

bool ModelManager::ImportRawData(Messager & messager, Model_basemost * model_, TABLE_TYPES const table_type, boost::filesystem::path const & path_to_raw_data)
{

	InputModel * input_model = nullptr;
	OutputModel * output_model = nullptr;

	try
	{
		switch (table_type)
		{
			case TABLE__DMU_IDENTIFIER:
			case TABLE__DMU_INSTANCE:
			case TABLE__CMU_IDENTIFIER:
			case TABLE__CMU_INSTANCE:
			case TABLE__UOA_IDENTIFIER:
			case TABLE__UOA_MEMBER:
			case TABLE__VG_CATEGORY:
			case TABLE__VG_SET_MEMBER:
				{
					input_model = dynamic_cast<InputModel*>(model_);
					if (!input_model)
					{
						// Todo: log warning
						return false;
					}
				}
				break;
			case TABLE__VG_SET_MEMBER_SELECTED:
			case TABLE__KAD_COUNT:
				{
					output_model = dynamic_cast<OutputModel*>(model_);
					if (!output_model)
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

	switch (table_type)
	{
		case TABLE__DMU_IDENTIFIER:
			{
			}
			break;
		case TABLE__DMU_INSTANCE:
			{
			}
			break;
		case TABLE__CMU_IDENTIFIER:
			{
			}
			break;
		case TABLE__CMU_INSTANCE:
			{
			}
			break;
		case TABLE__UOA_IDENTIFIER:
			{
			}
			break;
		case TABLE__UOA_MEMBER:
			{
			}
			break;
		case TABLE__VG_CATEGORY:
			{
			}
			break;
		case TABLE__VG_SET_MEMBER:
			{
			}
			break;
		case TABLE__VG_SET_MEMBER_SELECTED:
			{
			}
			break;
		case TABLE__KAD_COUNT:
			{
			}
			break;
	}

	return true;
}
