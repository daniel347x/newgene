#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>

#if 0
//#include <boost/predef.h> // not until Boost 1.55
#if __APPLE__ // ... but this is defined for Clang anyways
#   include <stdint.h>
	namespace std
	{
		typedef ::int8_t int8_t;
		typedef ::int16_t int16_t;
		typedef ::int32_t int32_t;
		typedef ::int64_t int64_t;
	}
#else
#   include <cstdint>
#endif
#endif

#include <cstdint>

#include <QtGlobal>

#include "../../NewGeneBackEnd/UIAction/ActionChanges.h"

#if 0
#if __APPLE__
#   include <boost/preprocessor/stringize.hpp>
#   pragma message "start message"
#   ifdef __MAC_OS_X_VERSION_MAX_ALLOWED
#       if (__MAC_OS_X_VERSION_MAX_ALLOWED == 1080)
#           pragma message "__MAC_OS_X_VERSION_MAX_ALLOWED == 1080"
#       else
#           pragma message "__MAC_OS_X_VERSION_MAX_ALLOWED != 1080"
#           pragma message BOOST_PP_STRINGIZE(__MAC_OS_X_VERSION_MAX_ALLOWED)
#       endif
#   else
#       pragma message "__MAC_OS_X_VERSION_MAX_ALLOWED is not defined"
#   endif
#   ifdef __MAC_10_8
#       if (__MAC_10_8 == 1080)
#           pragma message "__MAC_10_8 == 1080"
#       else
#           pragma message "__MAC_10_8 != 1080"
#           pragma message BOOST_PP_STRINGIZE(__MAC_10_8)
#       endif
#   else
#       pragma message "__MAC_10_8 is not defined"
#   endif
#   pragma message "done message"
#endif
#endif

typedef std::string STD_STRING;
typedef std::int64_t STD_INT64;

#include "../../NewGeneBackEnd/globals.h"
#include <QMessageBox>
#include "newgenefilenames.h"
#include "../../NewGeneBackEnd/Utilities/NewGeneException.h"
#include <memory>
#include <vector>
#include <map>
#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif
#include "Infrastructure/Messager/uimessager.h"

class NewGeneMainWindow;

class UIProjectManager;
class UISettingsManager;
class UILoggingManager;
class UIModelManager;
class UIDocumentManager;
class UIStatusManager;
class UITriggerManager;
class UIThreadManager;
class UIUIDataManager;
class UIUIActionManager;
class UIModelActionManager;

extern NewGeneMainWindow * theMainWindow;

UIProjectManager & projectManagerUI();
UISettingsManager & settingsManagerUI();
UILoggingManager & loggingManagerUI();
UIModelManager & modelManagerUI();
UIDocumentManager & documentManagerUI();
UIStatusManager & statusManagerUI();
UITriggerManager & triggerManagerUI();
UIThreadManager & threadManagerUI();
UIUIDataManager & uidataManagerUI();
UIUIActionManager & uiactionManagerUI();
UIModelActionManager & modelactionManagerUI();

#endif // GLOBALS_H
