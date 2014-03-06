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

	void AddTimeRangeGranularitySelectionBlock(QDialog & dialog, QFormLayout & form, QVBoxLayout & formTimeRangeGranularitySelection, QList<QRadioButton *> & radioButtonsTimeRangeGranularity);

	void AddTimeRangeSelectorBlock(
								   QDialog & dialog,
								   QFormLayout & form,
								   QList<QLineEdit *> & fieldsTimeRange,
								   QList<QRadioButton *> & radioButtonsTimeRange,
								   QBoxLayout & formTimeRangeSelection,
								   QWidget & YearWidget,
								   QFormLayout & formYearOptions,
								   QWidget & YearMonthDayWidget,
								   QFormLayout & formYearMonthDayOptions,
								   TIME_GRANULARITY const & time_range_granularity
			);
	bool ValidateTimeRangeBlock(
								   QDialog & dialog,
								   QFormLayout & form,
								   QList<QLineEdit *> & fieldsTimeRange,
								   QList<QRadioButton *> & radioButtonsTimeRange,
								   QWidget & YearWidget,
								   QFormLayout & formYearOptions,
								   QWidget & YearMonthDayWidget,
								   QFormLayout & formYearMonthDayOptions,
								   TIME_GRANULARITY const & time_range_granularity,
								   std::vector<std::string> & dataTimeRange,
								   std::string errorMsg
			);

	void AddUoaCreationBlock(QDialog & dialog, QFormLayout & form, QWidget & UoaConstructionWidget, QVBoxLayout & formOverall, QWidget & UoaConstructionPanes, QHBoxLayout & formConstructionPanes, QVBoxLayout & formConstructionDivider, QListView *& lhs, QListView *& rhs, WidgetInstanceIdentifiers const & dmu_categories);
	void AddVgCreationBlock(QDialog & dialog, QFormLayout & form, QWidget & VgConstructionWidget, QVBoxLayout & formOverall, QWidget & VgConstructionPanes, QHBoxLayout & formConstructionPane, QListView *& listpane, WidgetInstanceIdentifiers const & uoas);

}

#endif // IMPORTDIALOGHELPER_H
