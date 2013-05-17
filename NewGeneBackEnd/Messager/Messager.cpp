#include "Messager.h"

void Messager::AppendMessage(MessagerMessage * message)
{
	_messages.push_back(std::unique_ptr<MessagerMessage>(message));
}

bool Messager::HasStatus()
{
	bool hasStatus = false;
	for (MessagesVector::const_iterator m_ = _messages.cbegin(); m_ != _messages.cend(); ++m_)
	{
		if (static_cast<std::int32_t>(m_->get()->_message_level) & MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE)
		{
			hasStatus = true;
			break;
		}
	}
	return hasStatus;
}

bool Messager::IsWarning()
{
	bool isWarning = false;
	for (MessagesVector::const_iterator m_ = _messages.cbegin(); m_ != _messages.cend(); ++m_)
	{
		if (static_cast<std::int32_t>(m_->get()->_message_level) & MESSAGER_MESSAGE_CATEGORY__WARNING)
		{
			isWarning = true;
			break;
		}
	}
	return isWarning;
}

bool Messager::IsError()
{
	bool isError = false;
	for (MessagesVector::const_iterator m_ = _messages.cbegin(); m_ != _messages.cend(); ++m_)
	{
		if (static_cast<std::int32_t>(m_->get()->_message_level) & MESSAGER_MESSAGE_CATEGORY__ERROR)
		{
			isError = true;
			break;
		}
	}
	return isError;
}
