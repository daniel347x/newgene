#ifndef OUTPUTPROJECTWORKQUEUE_H
#define OUTPUTPROJECTWORKQUEUE_H

#include "Base/outputprojectworkqueue_base.h"

class OutputProjectWorkQueue : public WorkQueueManager<UI_OUTPUT_PROJECT>
{

    public:

        explicit OutputProjectWorkQueue(QObject * parent = NULL);

};

#endif // OUTPUTPROJECTWORKQUEUE_H
