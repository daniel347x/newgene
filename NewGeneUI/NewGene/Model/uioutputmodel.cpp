#include "uioutputmodel.h"

UIOutputModel::UIOutputModel(Messager & messager, OutputModel & model, boost::filesystem::path const path_to_model)
    : UIModel(messager, model, path_to_model)
{
}
