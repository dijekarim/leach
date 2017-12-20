#include <stdio.h>
#include <string.h>

#include <omnetpp.h>
#include "nodebase.h"
#include "cl_msg_m.h"

void NodeBase::updateStats()
{
    if (this->myStatus ==1)
        this->nrMsg++;
    else
        ;
}

void NodeBase::initStats()
{
    this->nrMsg = 0;
}

void NodeBase::finish()
{
#if 0
    ev << "finished node id: ";
    ev << this->myId;
    ev << " nrMsg ";
    ev << this->nrMsg;
    ev << "\n";
#endif

#if 0
    recordScalar(" nrMsg", this->nrMsg);
#endif
}
