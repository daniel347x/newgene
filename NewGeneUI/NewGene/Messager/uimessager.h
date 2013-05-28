#ifndef UIMESSAGER_H
#define UIMESSAGER_H

#include <QObject>
#include <QTimer>
#include <QCoreApplication>
//#include "newgeneapplication.h"
#include "..\..\NewGeneBackEnd\Messager\Messager.h"

class UISettingsManager;
class UIProjectManager;
class UILoggingManager;
class UIModelManager;
class UIDocumentManager;
class UIStatusManager;

class UIMessagerStatusMessage : public MessagerStatusMessage
{

	public:

		UIMessagerStatusMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::string const & TheMessageText)
			: MessagerStatusMessage(TheMessage, TheMessageText)
		{

		}

};

class UIMessagerWarningMessage : public MessagerWarningMessage
{

	public:

		UIMessagerWarningMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::string const & TheMessageText)
			: MessagerWarningMessage(TheMessage, TheMessageText)
		{

		}

};

class UIMessagerErrorMessage : public QObject, public MessagerErrorMessage
{

		Q_OBJECT

	public:

		UIMessagerErrorMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::string const & TheMessageText)
			: MessagerErrorMessage(TheMessage, TheMessageText)
		{
			QObject::connect(this, SIGNAL(sendErrorMessageToBeDisplayed(std::string const)),
								  QCoreApplication::instance(), SLOT(showErrorBox(std::string const)));

			emit sendErrorMessageToBeDisplayed(TheMessageText);

			//QTimer::singleShot( 0, static_cast<NewGeneApplication*>(QCoreApplication::instance()), SLOT(showErrorBox(TheMessageText) ) );
		}

	signals:

		void sendErrorMessageToBeDisplayed(std::string const msg);

};

class UIMessager : public QObject, public Messager
{
		Q_OBJECT
	public:
		explicit UIMessager(QObject *parent = 0);
		~UIMessager();

		void displayStatusMessages();

	signals:

	public slots:

	public:

		bool do_not_handle_messages_on_desctruction;

};

#endif // UIMESSAGER_H
