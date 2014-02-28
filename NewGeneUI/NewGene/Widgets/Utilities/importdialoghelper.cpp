#include "importdialoghelper.h"

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif

void ImportDialogHelper::AddFileChooserBlock(QFormLayout & form, QList<QLineEdit *> & fieldsFileChooser, std::vector<std::string> const & fileChooserStrings)
{

	QString labelFilePathName = QString( fileChooserStrings[0].c_str() );
	QLineEdit *lineEditFilePathName = new QLineEdit(&dialog);
	QPushButton *buttonFilePathName = new QPushButton("...", &dialog);
	QBoxLayout formFileSelection(QBoxLayout::LeftToRight);
	formFileSelection.addWidget(lineEditFilePathName);
	formFileSelection.addWidget(buttonFilePathName);
	fieldsFileChooser << lineEditFilePathName;
	//buttons << buttonFilePathName;
	form.addRow(labelFilePathName, &formFileSelection);

	QObject::connect(buttonFilePathName, &QPushButton::clicked, [&lineEditFilePathName, this]()
	{
		QString the_file = QFileDialog::getOpenFileName(this, fileChooserStrings[1].c_str(), fileChooserStrings[2].c_str(), fileChooserStrings[2].c_str());
		if (!the_file.isEmpty())
		{
			lineEditFilePathName->setText(the_file);
		}
	});

}

bool ImportDialogHelper::ValidateFileChooserBlock(QList<QLineEdit *> & fieldsFileChooser, std::vector<std::string> & dataFileChooser, std::string & errorMsg)
{

	bool valid = false;

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

void ImportDialogHelper::AddTimeRangeSelectorBlock(QFormLayout & form, QList<QLineEdit *> & fieldsTimeRange)
{

	// Time range RADIO BUTTONS
	form.addRow(new QLabel("Time range options:"));
	QBoxLayout formTimeRangeSelection(QBoxLayout::LeftToRight);
	QRadioButton * YButton = new QRadioButton("Year", &dialog);
	QRadioButton * YMDButton = new QRadioButton("Year, Month, Day", &dialog);
	formTimeRangeSelection.addWidget(YButton);
	formTimeRangeSelection.addWidget(YMDButton);
	form.addRow(&formTimeRangeSelection);
	YMDButton->setChecked(true);


	// Time range OPTIONS - Year
	QWidget YearWidget;
	QFormLayout formYearOptions(&YearWidget);
	YearWidget.setLayout(&formYearOptions);

	// year
	QString labelyYearStart = QString("'Year Start' column name:");
	QLineEdit * lineEdityYearStart = new QLineEdit(&YearWidget);
	formYearOptions.addRow(labelyYearStart, lineEdityYearStart);
	fieldsTimeRange << lineEdityYearStart;
	QString labelyYearEnd = QString("'Year End' column name:");
	QLineEdit * lineEdityYearEnd = new QLineEdit(&YearWidget);
	formYearOptions.addRow(labelyYearEnd, lineEdityYearEnd);
	fieldsTimeRange << lineEdityYearEnd;

	YearWidget.hide();
	form.addRow(&YearWidget);


	// Time range OPTIONS - Year, Month, Day
	QWidget YearMonthDayWidget;
	QFormLayout formYearMonthDayOptions(&YearMonthDayWidget);
	YearMonthDayWidget.setLayout(&formYearMonthDayOptions);

	// year
	QString labelymdYearStart = QString("'Year Start' column name:");
	QLineEdit * lineEditymdYearStart = new QLineEdit(&YearMonthDayWidget);
	formYearMonthDayOptions.addRow(labelymdYearStart, lineEditymdYearStart);
	fieldsTimeRange << lineEditymdYearStart;
	QString labelymdYearEnd = QString("'Year End' column name:");
	QLineEdit * lineEditymdYearEnd = new QLineEdit(&YearMonthDayWidget);
	formYearMonthDayOptions.addRow(labelymdYearEnd, lineEditymdYearEnd);
	fieldsTimeRange << lineEditymdYearEnd;

	// month
	QString labelymdMonthStart = QString("'Month Start' column name:");
	QLineEdit * lineEditymdMonthStart = new QLineEdit(&YearMonthDayWidget);
	formYearMonthDayOptions.addRow(labelymdMonthStart, lineEditymdMonthStart);
	fieldsTimeRange << lineEditymdMonthStart;
	QString labelymdMonthEnd = QString("'Month End' column name:");
	QLineEdit * lineEditymdMonthEnd = new QLineEdit(&YearMonthDayWidget);
	formYearMonthDayOptions.addRow(labelymdMonthEnd, lineEditymdMonthEnd);
	fieldsTimeRange << lineEditymdMonthEnd;

	// day
	QString labelymdDayStart = QString("'Day Start' column name:");
	QLineEdit * lineEditymdDayStart = new QLineEdit(&YearMonthDayWidget);
	formYearMonthDayOptions.addRow(labelymdDayStart, lineEditymdDayStart);
	fieldsTimeRange << lineEditymdDayStart;
	QString labelymdDayEnd = QString("'Day End' column name:");
	QLineEdit * lineEditymdDayEnd = new QLineEdit(&YearMonthDayWidget);
	formYearMonthDayOptions.addRow(labelymdDayEnd, lineEditymdDayEnd);
	fieldsTimeRange << lineEditymdDayEnd;

	YearMonthDayWidget.show();
	form.addRow(&YearMonthDayWidget);


	QObject::connect(YButton, &QRadioButton::toggled, [&YButton, &YMDButton, &YearWidget, &YearMonthDayWidget, this]()
	{
		YearMonthDayWidget.hide();
		YearWidget.hide();
		if (YButton->isChecked())
		{
			YearWidget.show();
		}
		if (YMDButton->isChecked())
		{
			YearMonthDayWidget.show();
		}
	});

	QObject::connect(YMDButton, &QRadioButton::toggled, [&YButton, &YMDButton, &YearWidget, &YearMonthDayWidget, this]()
	{
		YearMonthDayWidget.hide();
		YearWidget.hide();
		if (YButton->isChecked())
		{
			YearWidget.show();
		}
		if (YMDButton->isChecked())
		{
			YearMonthDayWidget.show();
		}
	});

}

bool ImportDialogHelper::ValidateTimeRangeBlock(QList<QLineEdit *> & fieldsTimeRange, std::vector<std::string> & dataTimeRange)
{

}
