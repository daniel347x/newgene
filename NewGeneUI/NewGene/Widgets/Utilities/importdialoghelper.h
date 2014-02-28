#ifndef IMPORTDIALOGHELPER_H
#define IMPORTDIALOGHELPER_H

#include "../../../../NewGeneBackEnd/Utilities/TimeRangeHelper.h"

namespace ImportDialogHelper
{

	void AddFileChooserBlock(QFormLayout & form, QList<QLineEdit *> & fieldsFileChooser, std::vector<std::string> const & fileChooserStrings);
	bool ValidateFileChooserBlock(QList<QLineEdit *> & fieldsFileChooser, std::vector<std::string> & dataFileChooser, std::string & errorMsg);

	void AddTimeRangeSelectorBlock(QFormLayout & form, QList<QLineEdit *> & fieldsTimeRange);
	bool ValidateTimeRangeBlock(QList<QLineEdit *> & fieldsTimeRange, std::vector<std::string> & dataTimeRange, TimeRange::TimeRangeImportMode & timeRangeImportMode, std::string & errorMsg);

}

#endif // IMPORTDIALOGHELPER_H
