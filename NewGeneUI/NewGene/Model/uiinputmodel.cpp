#include "uiinputmodel.h"

UIInputModel::UIInputModel(Messager & messager, InputModel & model, boost::filesystem::path const path_to_model)
    : UIModel(messager, model, path_to_model)
{
}
