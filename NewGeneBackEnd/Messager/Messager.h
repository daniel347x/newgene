#ifndef MESSAGER_H
#define MESSAGER_H

#include <vector>
#include <set>
#include <memory>
#include <cstdint>

#ifndef Q_MOC_RUN
#endif

enum MESSAGER_MESSAGE_ENUM
{
	  MESSAGER_MESSAGE__FILE_DOES_NOT_EXIST
	, MESSAGER_MESSAGE__FILE_INVALID_FORMAT
	, MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT
	, MESSAGER_MESSAGE__INVALID_SETTING_ENUM_VALUE
	, MESSAGER_MESSAGE__NO_SETTING_FOUND
	, MESSAGER_MESSAGE__NO_PROJECT_AVAILABLE
	, MESSAGER_MESSAGE__GENERAL_WARNING
	, MESSAGER_MESSAGE__GENERAL_ERROR
	, MESSAGER_MESSAGE__UNKNOWN_ERROR
	, MESSAGER_MESSAGE__LAST
};

enum MESSAGER_MESSAGE_CATEGORY_ENUM
{
	  MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE = 0x00000001
	, MESSAGER_MESSAGE_CATEGORY__WARNING = 0x00000002
	, MESSAGER_MESSAGE_CATEGORY__ERROR = 0x00000004
};

class MessagerMessage
{

	public:

		virtual ~MessagerMessage() {}

		MessagerMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::int32_t const TheMessageLevel, std::string const & TheMessageText)
			: _message(TheMessage)
			, _message_level(TheMessageLevel)
			, _message_text(TheMessageText)
		{

		}

	public:

		MESSAGER_MESSAGE_ENUM _message;
		std::int32_t _message_level;
		std::string _message_text;

		friend class Messager;

};

class MessagerStatusMessage : public MessagerMessage
{

	public:

		MessagerStatusMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::string const & TheMessageText)
			: MessagerMessage(TheMessage, MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE, TheMessageText)
		{

		}

};

class MessagerWarningMessage : public MessagerMessage
{

public:

	MessagerWarningMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::string const & TheMessageText)
		: MessagerMessage(TheMessage, MESSAGER_MESSAGE_CATEGORY__WARNING | MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE, TheMessageText)
	{

	}

};

class MessagerErrorMessage : public MessagerMessage
{

public:

	MessagerErrorMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::string const & TheMessageText)
		: MessagerMessage(TheMessage, MESSAGER_MESSAGE_CATEGORY__ERROR | MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE, TheMessageText)
	{

	}

};

class Messager
{

	public:

		typedef std::vector<std::unique_ptr<MessagerMessage> > MessagesVector;

		Messager() {}

		void AppendMessage(MessagerMessage * message);

		bool HasStatus();
		bool IsWarning();
		bool IsError();

	protected:

		MessagesVector _messages;

};

#endif
