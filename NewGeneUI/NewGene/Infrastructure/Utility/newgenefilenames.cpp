#include "newgenefilenames.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif

#include <QMessageBox>

QString NewGeneFileNames::logFileName = "NewGene.log";
QString NewGeneFileNames::importLogFileName = "newgene.import.log";
QString NewGeneFileNames::settingsFileName = "NewGene.Settings.xml";

QString NewGeneFileNames::defaultInputProjectFileName = "DefaultInputProjectSettings.newgene.in.xml";
QString NewGeneFileNames::defaultOutputProjectFileName = "DefaultOutputProjectSettings.newgene.out.xml";

QString NewGeneFileNames::preLoadedInputProjectFileName = "NewGeneInputDatasets.newgene.in.xml";
QString NewGeneFileNames::preLoadedOutputProjectFileName = "DefaultOutputProjectSettings.newgene.out.xml";

bool checkValidProjectFilenameExtension(bool const isInputProject /* true = input project, false = output project */, std::string & filename, bool const appendIfMissing)
{
	std::string comp {".newgene.out.xml"};
	std::string warningInputFile {"Invalid .xml project file.  The file extension for NewGene input dataset project files must be '.newgene.in.xml'."};
	std::string warningOutputFile {"Invalid .xml project file.  The file extension for NewGene output dataset project files must be '.newgene.out.xml'."};
	if (isInputProject)
	{
		comp = ".newgene.in.xml";
	}
	if (boost::iends_with(filename, comp))
	{
		return true;
	}
	if (appendIfMissing)
	{
		if (isInputProject)
		{
			filename += warningInputFile;
		}
		else
		{
			filename += warningOutputFile;
		}
		return true;
	}
	else
	{
		if (isInputProject)
		{
			QMessageBox::information(nullptr, QString("Invalid project file"), warningInputFile);
		}
		else
		{
			QMessageBox::information(nullptr, QString("Invalid project file"), warningOutputFile);
		}
	}
	return false;
}
