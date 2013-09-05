#ifndef UIMESSAGERSINGLESHOT_H
#define UIMESSAGERSINGLESHOT_H

#include "uimessager.h"

class UIMessagerSingleShot
{
	public:
		UIMessagerSingleShot(UIMessager & messager_);
		UIMessagerSingleShot();
		virtual ~UIMessagerSingleShot();

		UIMessager & get()
		{
			return messager;
		}

		void setMode(UIMessager::Mode const mode_)
		{
			messager.setMode(mode_);
		}

	private:

		std::unique_ptr<UIMessager> p_messager;

		// order of initialization important; do not reorder

	public:

		UIMessager & messager;

	protected:

		UIMessager::Mode saved_mode;

};

#endif // UIMESSAGERSINGLESHOT_H
