#include "uimessagersingleshot.h"

UIMessagerSingleShot::UIMessagerSingleShot(UIMessager & messager_)
	: messager(messager_)
{
	messager.InitializeSingleShot();
}

UIMessagerSingleShot::UIMessagerSingleShot()
	: p_messager(new UIMessager())
	, messager(*p_messager)
{
	saved_mode = messager.getMode();
	messager.InitializeSingleShot();
}

UIMessagerSingleShot::~UIMessagerSingleShot()
{
	messager.FinalizeSingleShot();
	messager.setMode(saved_mode);
}
