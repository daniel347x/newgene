#ifndef UIMESSAGERSINGLESHOT_H
#define UIMESSAGERSINGLESHOT_H

#include "uimessager.h"

class UIMessagerSingleShot
{
    public:
        UIMessagerSingleShot(UIMessager & messager_);
        virtual ~UIMessagerSingleShot();

        UIMessager & get()
        {
            return messager;
        }

        UIMessager & messager;
};

#endif // UIMESSAGERSINGLESHOT_H
