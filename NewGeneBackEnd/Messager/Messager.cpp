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
		if (static_cast<std::int32_t>(m_->get()->_message_category) & MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE)
		{
			hasStatus = true;
			break;
		}
	}
	return hasStatus;
}

bool Messager::RequiresLogging()
{
	bool requiresLogging = false;
	for (MessagesVector::const_iterator m_ = _messages.cbegin(); m_ != _messages.cend(); ++m_)
	{
		if (static_cast<std::int32_t>(m_->get()->_message_category) & MESSAGER_MESSAGE_CATEGORY__LOG_MESSAGE)
		{
			requiresLogging = true;
			break;
		}
	}
	return requiresLogging;
}

bool Messager::IsWarning()
{
	bool isWarning = false;
	for (MessagesVector::const_iterator m_ = _messages.cbegin(); m_ != _messages.cend(); ++m_)
	{
		if (static_cast<std::int32_t>(m_->get()->_message_category) & MESSAGER_MESSAGE_CATEGORY__WARNING)
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
	if (IsErrorCatastrophic())
	{
		isError = true;
	}
	else
	{
		for (MessagesVector::const_iterator m_ = _messages.cbegin(); m_ != _messages.cend(); ++m_)
		{
			if (static_cast<std::int32_t>(m_->get()->_message_category) & MESSAGER_MESSAGE_CATEGORY__ERROR)
			{
				isError = true;
				break;
			}
		}
	}
	return isError;
}

bool Messager::IsErrorCatastrophic()
{
	bool isErrorCatastrophic = false;
	for (MessagesVector::const_iterator m_ = _messages.cbegin(); m_ != _messages.cend(); ++m_)
	{
		if (static_cast<std::int32_t>(m_->get()->_message_category) & MESSAGER_MESSAGE_CATEGORY__ERROR_CATASTROPHIC)
		{
			isErrorCatastrophic = true;
			break;
		}
	}
	return isErrorCatastrophic;
}
