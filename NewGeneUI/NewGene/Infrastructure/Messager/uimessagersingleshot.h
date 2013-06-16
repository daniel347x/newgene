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

    private:
        std::unique_ptr<UIMessager> p_messager;

        // order of initialization important; do not reorder
    public:
        UIMessager & messager;

};

#endif // UIMESSAGERSINGLESHOT_H
