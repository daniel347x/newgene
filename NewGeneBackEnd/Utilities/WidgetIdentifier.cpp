#include "WidgetIdentifier.h"

WidgetInstanceIdentifier::Notes MakeNotes(char const * const notes1, char const * const notes2, char const * const notes3)
{
	std::shared_ptr<std::string> notes1_;
	std::shared_ptr<std::string> notes2_;
	std::shared_ptr<std::string> notes3_;
	if (notes1)
	{
		notes1_ = std::make_shared<std::string>(notes1);
	}
	if (notes2)
	{
		notes2_ = std::make_shared<std::string>(notes2);
	}
	if (notes3)
	{
		notes3_ = std::make_shared<std::string>(notes3);
	}
	return WidgetInstanceIdentifier::Notes(notes1_, notes2_, notes3_);
}

WidgetInstanceIdentifier::Notes MakeNotes(std::string notes1, std::string notes2, std::string notes3)
{
	return WidgetInstanceIdentifier::Notes(notes1, notes2, notes3);
}
