#ifndef GLOBALS_H
#define GLOBALS_H

#include <QMessageBox>
#include "newgenefilenames.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include <memory>
#include <vector>
#include <map>
#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif

class NewGeneMainWindow;

extern NewGeneMainWindow * theMainWindow;

#endif // GLOBALS_H
