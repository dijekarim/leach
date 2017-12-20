#ifndef NODE_H
#define NODE_H

#include <omnetpp.h>
#include "leach.h"
#include "nodebase.h"
#include "bs.h"

/**
 * Represents a node in the network
 */
class Node : public NodeBase
{
    public:
    Node() : NodeBase() {};

    protected:
    virtual void initialize();
    virtual void handleMessage(omnetpp::cMessage * msg);
    virtual void finish();

    void sendExploratory(int src, int rec, char attr, int value, int data, int scount, int bcount,
                         int hopcount, int seqNumber);
    void sendData(int src, int rec, char attr, int value, int data, int scount, int bcount,
                  int mustShortest);
    void sendReinforcement(int src, int rec, char attr, int value, int bcount, int scount, int hops,
                           int seqNumber, int period, int strategy);

    void send2BS(int src, int rec, int energy, int status, int cluster, int xpos, int ypos);
    void initNodes();
    void sendTDMA();
    void sendDataToCHead(int newSun);
    void sendData2BS(int data);
    void energyReceive(int bits);
    void energyTransmit(int bits, int dist);
    void energyDataAggr(int signals);
    float computedis(cModule *mod);

  public:  //XXX
    static int fnd;
    static int hnd;
    static int lnd;
    static int nrDead;
    int xpos;                   // just for simulation
    int ypos;
    int nrRounds;				// number of rounds (each round starts with setups)
    int curRound;               //Current round no.
    int nrFrames;               // number of "rounds in a round" (called frames)
    int curFrame;
    int frameTime;              // how many slots has one frame
    int myData;
    int mySlot;                 // slot in current frame
    int bsId;                   // ID of base station
	int myStatus;
    double energy;              // XXX energy in microJoule  XXX nnode.h
    float headDist;               // distance to head
    int bsDist;
    int myCluster;              // to which cluster this node belongs
	int flag;
    int lastClusterRound;       // round when node was cluster head last time
    
    Node *nodePtr[103];
    int clusterNodes;
	int nrChild;
    int cHead;                  // flag denoting if I this node is cluster head
    int headId;                 // id of cluster head (my cluster)

    int myHead;

    int nrDataPackets;          // nr of data packets head has received in this frame
    int sentBS;                 // how many status updates sent to BS by this node
    int wasHead;                // ctr denoting if cluster was cluster head in last 1/P rounds
    int headBatt;
	double P;
};

Define_Module(Node); 
   
#endif

