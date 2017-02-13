#ifndef NEWGENEFILENAMES_H
#define NEWGENEFILENAMES_H

#include <QString>

struct NewGeneFileNames
{
	static QString logFileName;
	static QString importLogFileName;
	static QString settingsFileName;
	static QString defaultInputProjectFileName;
	static QString defaultOutputProjectFileName;
	static QString preLoadedInputProjectFileName;
	static QString preLoadedOutputProjectFileName;
};

bool checkValidProjectFilenameExtension(bool const isInputProject /* true = input project, false = output project */, std::string & filename, bool const appendIfMissing = false);

#endif // NEWGENEFILENAMES_H
