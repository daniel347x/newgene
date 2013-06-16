#include "uimessagersingleshot.h"

UIMessagerSingleShot::UIMessagerSingleShot(UIMessager & messager_)
    : messager(messager_)
{
    messager.InitializeSingleShot();
}

UIMessagerSingleShot::~UIMessagerSingleShot()
{
    messager.FinalizeSingleShot();
}
