#ifndef IMPORTDIALOGHELPER_H
#define IMPORTDIALOGHELPER_H

#include "../../../../NewGeneBackEnd/Utilities/TimeRangeHelper.h"
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QList>
#include <vector>
#include <string>

namespace ImportDialogHelper
{

	void AddFileChooserBlock(QDialog & dialog, QFormLayout & form, QList<QLineEdit *> & fieldsFileChooser, std::vector<std::string> const & fileChooserStrings);
	bool ValidateFileChooserBlock(QList<QLineEdit *> & fieldsFileChooser, std::vector<std::string> & dataFileChooser, std::string & errorMsg);

	void AddTimeRangeSelectorBlock(QDialog & dialog, QFormLayout & form, QList<QLineEdit *> & fieldsTimeRange);
	bool ValidateTimeRangeBlock(QList<QLineEdit *> & fieldsTimeRange, std::vector<std::string> & dataTimeRange, TimeRange::TimeRangeImportMode & timeRangeImportMode, std::string & errorMsg);

}

#endif // IMPORTDIALOGHELPER_H
