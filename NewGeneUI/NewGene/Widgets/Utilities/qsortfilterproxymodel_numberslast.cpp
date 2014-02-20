#include "qsortfilterproxymodel_numberslast.h"
#include "../../../../NewGeneBackEnd/Utilities/WidgetIdentifier.h"
#include "../../Infrastructure/Project/uiprojectmanager.h"
#include "../../Infrastructure/Project/uiinputproject.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif

QSortFilterProxyModel_NumbersLast::QSortFilterProxyModel_NumbersLast(QObject * parent)
	: QSortFilterProxyModel(parent)
{
}

bool QSortFilterProxyModel_NumbersLast::lessThan(const QModelIndex &left, const QModelIndex &right) const
{

	QVariant leftData = sourceModel()->data(left);
	QVariant rightData = sourceModel()->data(right);

	WidgetInstanceIdentifier identifierLeft = leftData.value<WidgetInstanceIdentifier>();
	WidgetInstanceIdentifier identifierRight = rightData.value<WidgetInstanceIdentifier>();

	bool has_code_left = false;
	if (identifierLeft.code && !identifierLeft.code->empty())
	{
		has_code_left = true;
	}

	bool has_description_left = false;
	if (identifierLeft.longhand && !identifierLeft.longhand->empty())
	{
		has_description_left = true;
	}

	bool has_text_left = false;
	if (has_code_left || has_description_left)
	{
		has_text_left = true;
	}

	bool has_code_right = false;
	if (identifierRight.code && !identifierRight.code->empty())
	{
		has_code_right = true;
	}

	bool has_description_right = false;
	if (identifierRight.longhand && !identifierRight.longhand->empty())
	{
		has_description_right = true;
	}

	bool has_text_right = false;
	if (has_code_right || has_description_right)
	{
		has_text_right = true;
	}

	if (has_text_left && has_text_right)
	{
		// compare by text, for both
		if (has_code_left && has_code_right)
		{
			return boost::lexicographical_compare(*identifierLeft.code, *identifierRight.code);
		}
		else if (has_code_left && !has_code_right)
		{
			return true;
		}
		else if (!has_code_left && has_code_right)
		{
			return false;
		}
		else
		{
			if (has_description_left && has_description_right)
			{
				return boost::lexicographical_compare(*identifierLeft.longhand, *identifierRight.longhand);
			}
			else if (has_description_left && !has_description_right)
			{
				return true;
			}
			else if (!has_description_left && has_description_right)
			{
				return false;
			}
			else
			{
				return false;
			}
		}
	}

	else if (has_text_left && !has_text_right)
	{
		// the left wins because it has text
		return true;
	}

	else if (!has_text_left && has_text_right)
	{
		// the right wins because it has text
		return false;
	}

	// compare by uuid, for both
	return boost::lexicographical_compare(*identifierLeft.uuid, *identifierRight.uuid);

}
