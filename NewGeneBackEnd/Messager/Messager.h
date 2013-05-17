#ifndef MESSAGER_H
#define MESSAGER_H

#include <vector>
#include <memory>
#include <cstdint>

#ifndef Q_MOC_RUN
#endif

enum MESSAGER_MESSAGE_ENUM
{
	  MESSAGER_MESSAGE__FILE_DOES_NOT_EXIST
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
	
		MessagerMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::int32_t const TheMessageLevel)
			: _message(TheMessage)
			, _message_level(TheMessageLevel)
		{

		}

	protected:

		MESSAGER_MESSAGE_ENUM _message;
		std::int32_t _message_level;

		friend class Messager;

};

class MessagerStatusMessage : public MessagerMessage
{

	public:

		MessagerStatusMessage(MESSAGER_MESSAGE_ENUM const TheMessage)
			: MessagerMessage(TheMessage, MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE)
		{

		}

};

class MessagerWarningMessage : public MessagerMessage
{

public:

	MessagerWarningMessage(MESSAGER_MESSAGE_ENUM const TheMessage)
		: MessagerMessage(TheMessage, MESSAGER_MESSAGE_CATEGORY__WARNING | MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE)
	{

	}

};

class MessagerErrorMessage : public MessagerMessage
{

public:

	MessagerErrorMessage(MESSAGER_MESSAGE_ENUM const TheMessage)
		: MessagerMessage(TheMessage, MESSAGER_MESSAGE_CATEGORY__ERROR | MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE)
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
