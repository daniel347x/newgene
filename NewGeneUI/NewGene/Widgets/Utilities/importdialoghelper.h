#ifndef IMPORTDIALOGHELPER_H
#define IMPORTDIALOGHELPER_H

#include "../../../../NewGeneBackEnd/Utilities/TimeRangeHelper.h"
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QList>
#include <vector>
#include <string>
#include <QListView>
#include "../../../../NewGeneBackEnd/Utilities/WidgetIdentifier.h"

namespace ImportDialogHelper
{

	void AddFileChooserBlock(QDialog & dialog, QFormLayout & form, QBoxLayout & formFileSelection, QWidget & FileChooserWidget, QList<QLineEdit *> & fieldsFileChooser, std::vector<std::string> const & fileChooserStrings);
	bool ValidateFileChooserBlock(QList<QLineEdit *> & fieldsFileChooser, std::vector<std::string> & dataFileChooser, std::string & errorMsg);

	void AddTimeRangeSelectorBlock(QDialog & dialog, QFormLayout & form, QList<QLineEdit *> & fieldsTimeRange, QList<QRadioButton *> & radioButtonsTimeRange);
	bool ValidateTimeRangeBlock(QList<QLineEdit *> & fieldsTimeRange, QList<QRadioButton *> & radioButtonsTimeRange, std::vector<std::string> & dataTimeRange, TimeRange::TimeRangeImportMode & timeRangeImportMode, std::string & errorMsg);

	void AddUoaCreationBlock(QDialog & dialog, QFormLayout & form, QWidget & UoaConstructionWidget, QVBoxLayout & formOverall, QWidget & UoaConstructionPanes, QHBoxLayout & formConstructionPanes, QVBoxLayout & formConstructionDivider, QListView *& lhs, QListView *& rhs, WidgetInstanceIdentifiers const & dmu_categories);

}

#endif // IMPORTDIALOGHELPER_H
