#ifndef BS_H
#define BS_H

#include <omnetpp.h>
#include "leach.h"
#include "node.h"
#include "nodebase.h"


class BS : public NodeBase
{
    public:
        BS() : NodeBase() {};

    void initNodes();
    virtual void initialize(void);
    virtual void handleMessage(omnetpp::cMessage *msg);
    void finish();
    void resetStatus();
    int xpos;                   // just for simulation
    int ypos;

    int nrStatusRec;            // number of received status
    //struct _adv advInfo[NR_CLUSTERS];
    struct _adv advInfo[104];   // 

    cModule *nodePtr[104];

    int roundEnergyLoss;
    int xMax;
    int yMax;
    int numNodes;
    int roundsDone;
    int nrRounds;
    int firstDead;

    int roundTimes[400];        // times when round starts
    int deadRound[400];
    int nrHeads[400];
    int calledEnd;
    int halfDead;
    int halfDeadCtr;
    int oldDeadNodes;
    int deadNodes;
    int nrHead;
    
    double P;
    
    int cHeadsRound;
};

#endif

