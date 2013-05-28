#include "uioutputmodel.h"

UIOutputModel::UIOutputModel(UIMessager & messager, OutputModel & model, boost::filesystem::path const path_to_model, QObject * parent)
    : QObject(parent)
    , UIModel(messager, model, path_to_model)
{
}
