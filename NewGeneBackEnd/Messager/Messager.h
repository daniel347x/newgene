#ifndef MESSAGER_H
#define MESSAGER_H

#include <vector>

#ifndef Q_MOC_RUN
#endif

enum MESSAGER_MESSAGE_ENUM
{
	MESSAGER_MESSAGE__LAST
};

class MessagerMessage
{

	public:
	
		MessagerMessage(MESSAGER_MESSAGE_ENUM const TheMessage)
			: _message(TheMessage)
		{

		}

	protected:

		MESSAGER_MESSAGE_ENUM _message;

	private:

		MessagerMessage() {}

};

class Messager
{

	public:
	
		Messager() {}

	protected:

		std::vector<MessagerMessage> _messages;

};

#endif
