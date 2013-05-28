#include "uioutputmodel.h"

UIOutputModel::UIOutputModel(Messager & messager, OutputModel & model, boost::filesystem::path const path_to_model, QObject * parent)
    : QObject(parent)
    , UIModel(messager, model, path_to_model)
{
}
