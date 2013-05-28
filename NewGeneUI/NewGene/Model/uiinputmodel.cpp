#include "uiinputmodel.h"

UIInputModel::UIInputModel(UIMessager & messager, InputModel & model, boost::filesystem::path const path_to_model, QObject * parent)
    : QObject(parent)
    , UIModel(messager, model, path_to_model)
{
}
