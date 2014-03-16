#include "importdialoghelper.h"

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#	include <boost/format.hpp>
#endif

#include <QPushButton>
#include <QFileDialog>
#include <QLabel>
#include <QRadioButton>
#include <QListView>
#include <QSpacerItem>
#include <QStandardItemModel>

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../../../../../NewGeneBackEnd/Utilities/Validation.h"
#include "../../../../NewGeneBackEnd/Utilities/TimeRangeHelper.h"
#include "../../../../NewGeneBackEnd/Model/InputModel.h"
#include "../../../../NewGeneBackEnd/Model/Tables/TableInstances/DMU.h"

#include <set>

void ImportDialogHelper::AddFileChooserBlock(QDialog & dialog, QFormLayout & form, QBoxLayout & formFileSelection, QWidget & FileChooserWidget, QList<QLineEdit *> & fieldsFileChooser, std::vector<std::string> const & fileChooserStrings)
{

	FileChooserWidget.setLayout(&formFileSelection);

	QString labelFilePathName = QString( fileChooserStrings[0].c_str() );

	QLineEdit *lineEditFilePathName = new QLineEdit(&FileChooserWidget);
	QPushButton *buttonFilePathName = new QPushButton("...", &FileChooserWidget);

	lineEditFilePathName->setMinimumWidth(300);

	formFileSelection.addWidget(lineEditFilePathName);
	formFileSelection.addWidget(buttonFilePathName);

	fieldsFileChooser << lineEditFilePathName;

	form.addRow(labelFilePathName, &FileChooserWidget);

	QObject::connect(buttonFilePathName, &QPushButton::clicked, [=, &dialog, &fileChooserStrings]()
	{
		QString the_file = QFileDialog::getOpenFileName(&dialog, fileChooserStrings[1].c_str(), fileChooserStrings[2].c_str(), fileChooserStrings[2].c_str());
		if (!the_file.isEmpty())
		{
			lineEditFilePathName->setText(the_file);
		}
	});

}

bool ImportDialogHelper::ValidateFileChooserBlock(QList<QLineEdit *> & fieldsFileChooser, std::vector<std::string> & dataFileChooser, std::string & errorMsg)
{

	bool valid = true;
	dataFileChooser.clear();

	QLineEdit * data_column_file_pathname_field = fieldsFileChooser[0];

	if (data_column_file_pathname_field)
	{
		std::string filePathName(data_column_file_pathname_field->text().toStdString());
		if (!boost::filesystem::exists(filePathName))
		{
			valid = false;
			errorMsg = "The indicated file does not exist.";
		}
		else if (boost::filesystem::is_directory(filePathName))
		{
			valid = false;
			errorMsg = "The indicated file is a directory.";
		}

		if (valid)
		{
			dataFileChooser.push_back(filePathName);
		}
	}
	else
	{
		valid = false;
		errorMsg = "You must select a file.";
	}

	return valid;

}

void ImportDialogHelper::AddTimeRangeGranularitySelectionBlock(QDialog & dialog, QFormLayout & form, QVBoxLayout & formTimeRangeGranularitySelection, QList<QRadioButton *> & radioButtonsTimeRangeGranularity)
{

	// Time range RADIO BUTTONS
	form.addRow(new QLabel("Time range granularity options:"));
	QRadioButton * NButton = new QRadioButton("None", &dialog);
	QRadioButton * YButton = new QRadioButton("Year", &dialog);
	QRadioButton * YMDButton = new QRadioButton("Year, Month, Day", &dialog);
	QRadioButton * MButton = new QRadioButton("Month", &dialog);
	QRadioButton * SButton = new QRadioButton("String", &dialog);
	formTimeRangeGranularitySelection.addWidget(NButton);
	formTimeRangeGranularitySelection.addWidget(YButton);
	formTimeRangeGranularitySelection.addWidget(YMDButton);
	formTimeRangeGranularitySelection.addWidget(MButton);
	formTimeRangeGranularitySelection.addWidget(SButton);
	form.addRow(&formTimeRangeGranularitySelection);
	NButton->setChecked(true);

	radioButtonsTimeRangeGranularity << NButton << YButton << YMDButton << MButton << SButton;

}

void ImportDialogHelper::AddTimeRangeSelectorBlock(

												   QDialog & dialog,
												   QFormLayout & form,
												   QList<QLineEdit *> & fieldsTimeRange,
												   QList<QRadioButton *> & radioButtonsTimeRange,
												   QBoxLayout & formTimeRangeSelection,

												   QWidget & YearWidget,
												   QFormLayout & formYearOptions,

												   QWidget & YearMonthDayWidget,
												   QFormLayout & formYearMonthDayOptions,
												   QWidget & YearMonthDayWidget_ints,
												   QFormLayout & formYearMonthDayOptions_ints,
												   QWidget & YearMonthDayWidget_strings,
												   QFormLayout & formYearMonthDayOptions_strings,
												   QBoxLayout & formYMDTimeRange_StringVsInt,
												   QList<QRadioButton *> & radioButtonsYMD_StringVsInt_TimeRange,

												   QWidget & YearMonthWidget,
												   QFormLayout & formYearMonthOptions,

												   TIME_GRANULARITY const & time_range_granularity

												   )
{

	// Time range RADIO BUTTONS

	// ************************************************************************************ //
	// Always hidden - But used by validation routine
	// to determine which time granularity is selected
	// ************************************************************************************ //
	form.addRow(new QLabel("Time range options:"));
	QRadioButton * YButton = new QRadioButton("Year", &dialog);
	QRadioButton * YMDButton = new QRadioButton("Year, Month, Day", &dialog);
	QRadioButton * YMButton = new QRadioButton("Year, Month", &dialog);
	formTimeRangeSelection.addWidget(YButton);
	formTimeRangeSelection.addWidget(YMDButton);
	formTimeRangeSelection.addWidget(YMButton);
	form.addRow(&formTimeRangeSelection);

	radioButtonsTimeRange << YButton << YMDButton << YMButton;
	YButton->hide();
	YMDButton->hide();
	YMButton->hide();




	// Time range OPTIONS - Year
	YearWidget.setLayout(&formYearOptions);

	// year
	QString labelyYearStart = QString("'Year Start' (integer or text) column name:");
	QLineEdit * lineEdityYearStart = new QLineEdit(&YearWidget);
	formYearOptions.addRow(labelyYearStart, lineEdityYearStart);
	fieldsTimeRange << lineEdityYearStart;                                  // 1
	QString labelyYearEnd = QString("'Year End' (integer or text) column name (leave blank if not present):");
	QLineEdit * lineEdityYearEnd = new QLineEdit(&YearWidget);
	formYearOptions.addRow(labelyYearEnd, lineEdityYearEnd);
	fieldsTimeRange << lineEdityYearEnd;                                    // 2

	YearWidget.hide();
	form.addRow(&YearWidget);




	// Time range OPTIONS - Year, Month, Day
	YearMonthDayWidget.setLayout(&formYearMonthDayOptions);

	QRadioButton * YMDIntButton = new QRadioButton("Multiple columns - one for year, one for month, and one for day", &YearMonthDayWidget);
	QRadioButton * YMDStringButton = new QRadioButton("Single columns containing text such as \"11/12/1992\"", &YearMonthDayWidget);
	formYMDTimeRange_StringVsInt.addWidget(YMDIntButton);
	formYMDTimeRange_StringVsInt.addWidget(YMDStringButton);
	formYearMonthDayOptions.addRow(&formYMDTimeRange_StringVsInt);
	radioButtonsYMD_StringVsInt_TimeRange << YMDStringButton << YMDIntButton;

	// year
	QString labelymdYearStart = QString("'Year Start' (integer) column name:");
	QLineEdit * lineEditymdYearStart = new QLineEdit(&YearMonthDayWidget_ints);
	formYearMonthDayOptions_ints.addRow(labelymdYearStart, lineEditymdYearStart);
	fieldsTimeRange << lineEditymdYearStart;                                // 3
	QString labelymdYearEnd = QString("'Year End' (integer) column name:");
	QLineEdit * lineEditymdYearEnd = new QLineEdit(&YearMonthDayWidget_ints);
	formYearMonthDayOptions_ints.addRow(labelymdYearEnd, lineEditymdYearEnd);
	fieldsTimeRange << lineEditymdYearEnd;                                  // 4

	// month
	QString labelymdMonthStart = QString("'Month Start' (integer) column name:");
	QLineEdit * lineEditymdMonthStart = new QLineEdit(&YearMonthDayWidget_ints);
	formYearMonthDayOptions_ints.addRow(labelymdMonthStart, lineEditymdMonthStart);
	fieldsTimeRange << lineEditymdMonthStart;                               // 5
	QString labelymdMonthEnd = QString("'Month End' (integer) column name:");
	QLineEdit * lineEditymdMonthEnd = new QLineEdit(&YearMonthDayWidget_ints);
	formYearMonthDayOptions_ints.addRow(labelymdMonthEnd, lineEditymdMonthEnd);
	fieldsTimeRange << lineEditymdMonthEnd;                                 // 6

	// day
	QString labelymdDayStart = QString("'Day Start' (integer) column name:");
	QLineEdit * lineEditymdDayStart = new QLineEdit(&YearMonthDayWidget_ints);
	formYearMonthDayOptions_ints.addRow(labelymdDayStart, lineEditymdDayStart);
	fieldsTimeRange << lineEditymdDayStart;                                 // 7
	QString labelymdDayEnd = QString("'Day End' (integer) column name:");
	QLineEdit * lineEditymdDayEnd = new QLineEdit(&YearMonthDayWidget_ints);
	formYearMonthDayOptions_ints.addRow(labelymdDayEnd, lineEditymdDayEnd);
	fieldsTimeRange << lineEditymdDayEnd;                                   // 8

	formYearMonthDayOptions.addRow(&YearMonthDayWidget_ints);
	YearMonthDayWidget_ints.hide();

	QString labelymdStart = QString("'Starting Day' (text) column name:");
	QLineEdit * lineEditymdStart = new QLineEdit(&YearMonthDayWidget_strings);
	formYearMonthDayOptions_strings.addRow(labelymdStart, lineEditymdStart);
	fieldsTimeRange << lineEditymdStart;                                    // 9
	QString labelymdEnd = QString("'Ending Day' (text) column name:");
	QLineEdit * lineEditymdEnd = new QLineEdit(&YearMonthDayWidget_strings);
	formYearMonthDayOptions_strings.addRow(labelymdEnd, lineEditymdEnd);
	fieldsTimeRange << lineEditymdEnd;                                      // 10

	formYearMonthDayOptions.addRow(&YearMonthDayWidget_strings);
	YearMonthDayWidget_strings.hide();

	YearMonthDayWidget.hide();
	form.addRow(&YearMonthDayWidget);


	YMDIntButton->setChecked(true);
	YearMonthDayWidget_ints.show();

	QObject::connect(YMDIntButton, &QRadioButton::toggled, [&]()
	{
		YearMonthDayWidget_ints.hide();
		YearMonthDayWidget_strings.hide();
		if (YMDIntButton->isChecked())
		{
			YearMonthDayWidget_ints.show();
		}
		if (YMDStringButton->isChecked())
		{
			YearMonthDayWidget_strings.show();
		}
	});

	QObject::connect(YMDStringButton, &QRadioButton::toggled, [&]()
	{
		YearMonthDayWidget_ints.hide();
		YearMonthDayWidget_strings.hide();
		if (YMDIntButton->isChecked())
		{
			YearMonthDayWidget_ints.show();
		}
		if (YMDStringButton->isChecked())
		{
			YearMonthDayWidget_strings.show();
		}
	});





	// Time range OPTIONS - Month
	YearMonthWidget.setLayout(&formYearMonthOptions);

	// year
	QString labelymYearStart = QString("'Year Start' column name:");
	QLineEdit * lineEditymYearStart = new QLineEdit(&YearMonthWidget);
	formYearMonthOptions.addRow(labelymYearStart, lineEditymYearStart);
	fieldsTimeRange << lineEditymYearStart;                                // 11
	QString labelymYearEnd = QString("'Year End' column name:");
	QLineEdit * lineEditymYearEnd = new QLineEdit(&YearMonthWidget);
	formYearMonthOptions.addRow(labelymYearEnd, lineEditymYearEnd);
	fieldsTimeRange << lineEditymYearEnd;                                  // 12

	// month
	QString labelymMonthStart = QString("'Month Start' column name:");
	QLineEdit * lineEditymMonthStart = new QLineEdit(&YearMonthWidget);
	formYearMonthOptions.addRow(labelymMonthStart, lineEditymMonthStart);
	fieldsTimeRange << lineEditymMonthStart;                               // 13
	QString labelymMonthEnd = QString("'Month End' column name:");
	QLineEdit * lineEditymMonthEnd = new QLineEdit(&YearMonthWidget);
	formYearMonthOptions.addRow(labelymMonthEnd, lineEditymMonthEnd);
	fieldsTimeRange << lineEditymMonthEnd;                                 // 14

	YearMonthWidget.hide();
	form.addRow(&YearMonthWidget);




	switch (time_range_granularity)
	{

		case TIME_GRANULARITY__DAY:
			{
				YMDButton->setChecked(true);
				YearMonthDayWidget.show();
			}
			break;

		case TIME_GRANULARITY__YEAR:
			{
				YButton->setChecked(true);
				YearWidget.show();
			}
			break;

		case TIME_GRANULARITY__MONTH:
			{
				YMButton->setChecked(true);
				YearMonthWidget.show();
			}
			break;

		default:
			{
				boost::format msg("Invalid time range setting.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

	QObject::connect(YButton, &QRadioButton::toggled, [&]()
	{
		YearMonthDayWidget.hide();
		YearWidget.hide();
		YearMonthWidget.hide();
		if (YButton->isChecked())
		{
			YearWidget.show();
		}
		if (YMDButton->isChecked())
		{
			YearMonthDayWidget.show();
		}
		if (YMButton->isChecked())
		{
			YearMonthWidget.show();
		}
	});

	QObject::connect(YMDButton, &QRadioButton::toggled, [&]()
	{
		YearMonthDayWidget.hide();
		YearWidget.hide();
		YearMonthWidget.hide();
		if (YButton->isChecked())
		{
			YearWidget.show();
		}
		if (YMDButton->isChecked())
		{
			YearMonthDayWidget.show();
		}
		if (YMButton->isChecked())
		{
			YearMonthWidget.show();
		}
	});

	QObject::connect(YMButton, &QRadioButton::toggled, [&]()
	{
		YearMonthDayWidget.hide();
		YearWidget.hide();
		YearMonthWidget.hide();
		if (YButton->isChecked())
		{
			YearWidget.show();
		}
		if (YMDButton->isChecked())
		{
			YearMonthDayWidget.show();
		}
		if (YMButton->isChecked())
		{
			YearMonthWidget.show();
		}
	});

}

bool ImportDialogHelper::ValidateTimeRangeBlock
(
		QDialog & dialog,
		QFormLayout & form,
		QList<QLineEdit *> & fieldsTimeRange,
		QList<QRadioButton *> & radioButtonsTimeRange,

		QWidget & YearWidget,
		QFormLayout & formYearOptions,

		QWidget & YearMonthDayWidget,
		QFormLayout & formYearMonthDayOptions,
		QWidget & YearMonthDayWidget_ints,
		QFormLayout & formYearMonthDayOptions_ints,
		QWidget & YearMonthDayWidget_strings,
		QFormLayout & formYearMonthDayOptions_strings,
		QList<QRadioButton *> & radioButtonsYMD_StringVsInt_TimeRange,

		QWidget & YearMonthWidget,
		QFormLayout & formYearMonthOptions,

		TIME_GRANULARITY const & time_range_granularity,
		std::vector<std::string> & dataTimeRange,
		std::string & errorMsg
)
{

	dataTimeRange.clear();

	QRadioButton * YButton = radioButtonsTimeRange[0];
	QRadioButton * YMDButton = radioButtonsTimeRange[1];
	QRadioButton * YMButton = radioButtonsTimeRange[2];

	if (!YButton || !YMDButton || !YMButton)
	{
		boost::format msg("Invalid time range selection radio buttons");
		errorMsg = msg.str();
		return false;
	}

	QRadioButton * YMDStringButton = radioButtonsYMD_StringVsInt_TimeRange[0];
	QRadioButton * YMDIntButton = radioButtonsYMD_StringVsInt_TimeRange[1];

	if (!YMDStringButton || !YMDIntButton)
	{
		boost::format msg("Invalid time range (text vs. int) radio buttons");
		errorMsg = msg.str();
		return false;
	}

	if (YButton->isChecked())
	{

		int currentIndex = 0;
		QLineEdit * timeRange_y_yearStart = fieldsTimeRange[currentIndex++];
		QLineEdit * timeRange_y_yearEnd = fieldsTimeRange[currentIndex++];

		if (!timeRange_y_yearStart || !timeRange_y_yearEnd)
		{
			boost::format msg("Invalid date fields");
			errorMsg = msg.str();
			return false;
		}

		std::string y_yearStart(timeRange_y_yearStart->text().toStdString());
		std::string y_yearEnd(timeRange_y_yearEnd->text().toStdString());

		bool valid = true;

		if (valid)
		{
			valid = Validation::ValidateColumnName(y_yearStart, "Start Year", true, errorMsg);
		}

		if (valid)
		{
			if (!boost::trim_copy(y_yearEnd).empty())
			{
				valid = Validation::ValidateColumnName(y_yearEnd, "End Year", true, errorMsg);
			}
		}

		if (valid)
		{
			dataTimeRange.push_back(y_yearStart);
			dataTimeRange.push_back(y_yearEnd);
		}

		return valid;

	}

	else if (YMDButton->isChecked())
	{

		int currentIndex = 2;
		QLineEdit * timeRange_ymd_yearStart = fieldsTimeRange[currentIndex++];
		QLineEdit * timeRange_ymd_yearEnd = fieldsTimeRange[currentIndex++];
		QLineEdit * timeRange_ymd_monthStart = fieldsTimeRange[currentIndex++];
		QLineEdit * timeRange_ymd_monthEnd = fieldsTimeRange[currentIndex++];
		QLineEdit * timeRange_ymd_dayStart = fieldsTimeRange[currentIndex++];
		QLineEdit * timeRange_ymd_dayEnd = fieldsTimeRange[currentIndex++];
		QLineEdit * timeRange_ymd_Start = fieldsTimeRange[currentIndex++];
		QLineEdit * timeRange_ymd_End = fieldsTimeRange[currentIndex++];

		if (!timeRange_ymd_yearStart || !timeRange_ymd_yearEnd || !timeRange_ymd_monthStart || !timeRange_ymd_monthEnd || !timeRange_ymd_dayStart || !timeRange_ymd_dayEnd || !timeRange_ymd_Start || !timeRange_ymd_End)
		{
			boost::format msg("Invalid date fields");
			errorMsg = msg.str();
			return false;
		}

		std::string ymd_yearStart(timeRange_ymd_yearStart->text().toStdString());
		std::string ymd_yearEnd(timeRange_ymd_yearEnd->text().toStdString());
		std::string ymd_monthStart(timeRange_ymd_monthStart->text().toStdString());
		std::string ymd_monthEnd(timeRange_ymd_monthEnd->text().toStdString());
		std::string ymd_dayStart(timeRange_ymd_dayStart->text().toStdString());
		std::string ymd_dayEnd(timeRange_ymd_dayEnd->text().toStdString());
		std::string ymd_Start(timeRange_ymd_Start->text().toStdString());
		std::string ymd_End(timeRange_ymd_End->text().toStdString());

		bool valid = true;

		bool using_string_fields = false;
		if (YMDStringButton->isChecked())
		{
			using_string_fields = true;
		}

		if (using_string_fields)
		{

			if (valid)
			{
				valid = Validation::ValidateColumnName(ymd_Start, "Starting Day", true, errorMsg);
			}

			if (valid)
			{
				valid = Validation::ValidateColumnName(ymd_End, "Ending Day", true, errorMsg);
			}

		}
		else
		{

			if (valid)
			{
				valid = Validation::ValidateColumnName(ymd_yearStart, "Start Year", true, errorMsg);
			}

			if (valid)
			{
				valid = Validation::ValidateColumnName(ymd_monthStart, "Start Month", true, errorMsg);
			}

			if (valid)
			{
				valid = Validation::ValidateColumnName(ymd_dayStart, "Start Day", true, errorMsg);
			}

			if (valid)
			{
				valid = Validation::ValidateColumnName(ymd_yearEnd, "End Year", true, errorMsg);
			}

			if (valid)
			{
				valid = Validation::ValidateColumnName(ymd_monthEnd, "End Month", true, errorMsg);
			}

			if (valid)
			{
				valid = Validation::ValidateColumnName(ymd_dayEnd, "End Day", true, errorMsg);
			}

		}

		if (valid && using_string_fields)
		{
			dataTimeRange.push_back(ymd_Start);
			dataTimeRange.push_back(ymd_End);
		}
		else if (valid)
		{
			dataTimeRange.push_back(ymd_yearStart);
			dataTimeRange.push_back(ymd_monthStart);
			dataTimeRange.push_back(ymd_dayStart);
			dataTimeRange.push_back(ymd_yearEnd);
			dataTimeRange.push_back(ymd_monthEnd);
			dataTimeRange.push_back(ymd_dayEnd);
		}

		if (valid)
		{
			std::for_each(dataTimeRange.begin(), dataTimeRange.end(), std::bind(boost::trim<std::string>, std::placeholders::_1, std::locale()));
			std::set<std::string> testtimerangecols(dataTimeRange.cbegin(), dataTimeRange.cend());
			if (testtimerangecols.size() != dataTimeRange.size())
			{
				boost::format msg("Duplicate time range column names");
				errorMsg = msg.str();
				return false;
			}
		}

		return valid;

	}

	else if (YMButton->isChecked())
	{

		int currentIndex = 8;
		QLineEdit * timeRange_ym_yearStart = fieldsTimeRange[currentIndex++];
		QLineEdit * timeRange_ym_yearEnd = fieldsTimeRange[currentIndex++];
		QLineEdit * timeRange_ym_monthStart = fieldsTimeRange[currentIndex++];
		QLineEdit * timeRange_ym_monthEnd = fieldsTimeRange[currentIndex++];

		if (!timeRange_ym_yearStart || !timeRange_ym_yearEnd || !timeRange_ym_monthStart || !timeRange_ym_monthEnd)
		{
			boost::format msg("Invalid date fields");
			errorMsg = msg.str();
			return false;
		}

		std::string ym_yearStart(timeRange_ym_yearStart->text().toStdString());
		std::string ym_yearEnd(timeRange_ym_yearEnd->text().toStdString());
		std::string ym_monthStart(timeRange_ym_monthStart->text().toStdString());
		std::string ym_monthEnd(timeRange_ym_monthEnd->text().toStdString());

		bool valid = true;

		if (valid)
		{
			valid = Validation::ValidateColumnName(ym_yearStart, "Start Year", true, errorMsg);
		}

		if (valid)
		{
			valid = Validation::ValidateColumnName(ym_monthStart, "Start Month", true, errorMsg);
		}

		if (valid)
		{
			valid = Validation::ValidateColumnName(ym_yearEnd, "End Year", true, errorMsg);
		}

		if (valid)
		{
			valid = Validation::ValidateColumnName(ym_monthEnd, "End Month", true, errorMsg);
		}

		if (valid)
		{
			dataTimeRange.push_back(ym_yearStart);
			dataTimeRange.push_back(ym_monthStart);
			dataTimeRange.push_back(ym_yearEnd);
			dataTimeRange.push_back(ym_monthEnd);
		}

		if (valid)
		{
			std::for_each(dataTimeRange.begin(), dataTimeRange.end(), std::bind(boost::trim<std::string>, std::placeholders::_1, std::locale()));
			std::set<std::string> testtimerangecols(dataTimeRange.cbegin(), dataTimeRange.cend());
			if (testtimerangecols.size() != dataTimeRange.size())
			{
				boost::format msg("Duplicate time range column names");
				errorMsg = msg.str();
				return false;
			}
		}

		return valid;

	}

	boost::format msg("Invalid time range specification.");
	errorMsg = msg.str();

	return false;

}

void ImportDialogHelper::AddUoaCreationBlock(QDialog & dialog, QFormLayout & form, QWidget & UoaConstructionWidget, QVBoxLayout & formOverall, QWidget & UoaConstructionPanes, QHBoxLayout & formConstructionPanes, QVBoxLayout & formConstructionDivider, QListView *& lhs, QListView *& rhs, WidgetInstanceIdentifiers const & dmu_categories)
{

	QString labelTitle = QString("Define the DMU categories for the new Unit of Analysis:");
	QLabel * title = new QLabel(labelTitle, &dialog);

	lhs = new QListView(&UoaConstructionPanes);
	QWidget * middle = new QWidget(&UoaConstructionPanes);
	middle->setLayout(&formConstructionDivider);
	rhs = new QListView(&UoaConstructionPanes);

	QSpacerItem * middlespacetop = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);
	QPushButton * add = new QPushButton(">>>", middle);
	QPushButton * remove = new QPushButton("<<<", middle);
	QSpacerItem * middlespacebottom = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);
	formConstructionDivider.addItem(middlespacetop);
	formConstructionDivider.addWidget(add);
	formConstructionDivider.addWidget(remove);
	formConstructionDivider.addItem(middlespacebottom);

	formConstructionPanes.addWidget(lhs);
	formConstructionPanes.addWidget(middle);
	formConstructionPanes.addWidget(rhs);

	UoaConstructionPanes.setLayout(&formConstructionPanes);
	formOverall.addWidget(title);
	formOverall.addWidget(&UoaConstructionPanes);
	UoaConstructionWidget.setLayout(&formOverall);
	form.addRow(&UoaConstructionWidget);

	{

		QStandardItemModel * model = new QStandardItemModel(lhs);

		int index = 0;
		std::for_each(dmu_categories.cbegin(), dmu_categories.cend(), [&](WidgetInstanceIdentifier const & dmu_category)
		{
			if (dmu_category.uuid && !dmu_category.uuid->empty() && dmu_category.code && !dmu_category.code->empty())
			{

				QStandardItem * item = new QStandardItem();
				std::string text = Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category);
				item->setText(text.c_str());
				item->setEditable(false);
				item->setCheckable(false);
				QVariant v;
				v.setValue(dmu_category);
				item->setData(v);
				model->setItem( index, item );

				++index;

			}
		});

		model->sort(0);

		lhs->setModel(model);


		QStandardItemModel * rhsModel = new QStandardItemModel(rhs);
		rhs->setModel(rhsModel);

	}

	{

		QObject::connect(add, &QPushButton::clicked, [&]()
		{

			QStandardItemModel * lhsModel = static_cast<QStandardItemModel*>(lhs->model());
			if (lhsModel == nullptr)
			{
				boost::format msg("Invalid list view items in Construct UOA popup.");
				QMessageBox msgBox;
				msgBox.setText( msg.str().c_str() );
				msgBox.exec();
				return false;
			}

			QStandardItemModel * rhsModel = static_cast<QStandardItemModel*>(rhs->model());
			if (rhsModel == nullptr)
			{
				boost::format msg("Invalid rhs list view items in Construct UOA popup.");
				QMessageBox msgBox;
				msgBox.setText( msg.str().c_str() );
				msgBox.exec();
				return false;
			}

			QItemSelectionModel * dmu_selectionModel = lhs->selectionModel();
			if (dmu_selectionModel == nullptr)
			{
				boost::format msg("Invalid selection in Create UOA widget.");
				QMessageBox msgBox;
				msgBox.setText( msg.str().c_str() );
				msgBox.exec();
				return false;
			}

			QModelIndex selectedIndex = dmu_selectionModel->currentIndex();
			if (!selectedIndex.isValid())
			{
				// No selection
				return false;
			}

			QVariant dmu_category_variant = lhsModel->item(selectedIndex.row())->data();
			WidgetInstanceIdentifier dmu_category = dmu_category_variant.value<WidgetInstanceIdentifier>();

			std::string text = Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category);
			QStandardItem * item = new QStandardItem();
			item->setText(text.c_str());
			item->setEditable(false);
			item->setCheckable(false);

			QVariant v;
			v.setValue(dmu_category);
			item->setData(v);
			rhsModel->appendRow( item );

			return true;

		});

		QObject::connect(remove, &QPushButton::clicked, [&]()
		{

			QStandardItemModel * rhsModel = static_cast<QStandardItemModel*>(rhs->model());
			if (rhsModel == nullptr)
			{
				boost::format msg("Invalid rhs list view items in Construct UOA popup.");
				QMessageBox msgBox;
				msgBox.setText( msg.str().c_str() );
				msgBox.exec();
				return false;
			}

			QItemSelectionModel * dmu_selectionModel = rhs->selectionModel();
			if (dmu_selectionModel == nullptr)
			{
				boost::format msg("Invalid rhs selection in Create UOA widget.");
				QMessageBox msgBox;
				msgBox.setText( msg.str().c_str() );
				msgBox.exec();
				return false;
			}

			QModelIndex selectedIndex = dmu_selectionModel->currentIndex();
			if (!selectedIndex.isValid())
			{
				return false;
			}

			//QVariant dmu_category_variant = rhsModel->item(selectedIndex.row())->data();
			//WidgetInstanceIdentifier dmu_category = dmu_category_variant.value<WidgetInstanceIdentifier>();

			//std::string text = Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category);

			rhsModel->takeRow(selectedIndex.row());
			dmu_selectionModel->clearSelection();

			return true;

		});

		QObject::connect(lhs, &QListView::doubleClicked, [=](const QModelIndex & index)
		{
			add->click();
		});

		QObject::connect(rhs, &QListView::doubleClicked, [=](const QModelIndex & index)
		{
			remove->click();
		});

	}

}

void ImportDialogHelper::AddVgCreationBlock(QDialog & dialog, QFormLayout & form, QWidget & VgConstructionWidget, QVBoxLayout & formOverall, QWidget & VgConstructionPanes, QHBoxLayout & formConstructionPane, QListView *& listpane, WidgetInstanceIdentifiers const & uoas)
{

	QString labelTitle = QString("Choose the unit of analysis:");
	QLabel * title = new QLabel(labelTitle, &dialog);

	listpane = new QListView(&VgConstructionPanes);

	formConstructionPane.addWidget(listpane);

	VgConstructionPanes.setLayout(&formConstructionPane);
	formOverall.addWidget(title);
	formOverall.addWidget(&VgConstructionPanes);
	VgConstructionWidget.setLayout(&formOverall);
	form.addRow(&VgConstructionWidget);

	{

		QStandardItemModel * model = new QStandardItemModel(listpane);

		int index = 0;
		std::for_each(uoas.cbegin(), uoas.cend(), [&](WidgetInstanceIdentifier const & uoa)
		{
			if (uoa.uuid && !uoa.uuid->empty() && uoa.code && !uoa.code->empty())
			{

				if (!uoa.foreign_key_identifiers)
				{
					boost::format msg("Missing foreign_key_identifiers in UOA object.");
					throw NewGeneException() << newgene_error_description(msg.str());
				}

				QStandardItem * item = new QStandardItem();
				std::string text = Table_UOA_Identifier::GetUoaCategoryDisplayText(uoa, *uoa.foreign_key_identifiers);
				item->setText(text.c_str());
				item->setEditable(false);
				item->setCheckable(false);
				QVariant v;
				v.setValue(uoa);
				item->setData(v);
				model->setItem( index, item );

				++index;

			}
		});

		model->sort(0);

		listpane->setModel(model);

	}

}
