#ifndef LEACH_H
#define LEACH_H
#define NANO 0.001
#define PICO 0.000001

// NEW: CLUSTER_STUFF
#define NR_CLUSTERS      1      //update
#define BS_ID            1      // ID of base station
#define CH_MIN_ENERGY    30     // want to change this!!
#define CLUSTER_SIZE     100
#define FRAME_TIME       CLUSTER_SIZE+1 // how long is one frame (how many slots)

// messagetypes for clusters
#define CL_TOBS          1
#define CL_TOHEAD        2
#define CL_TDMA          3
#define CL_CHDATA        4
#define CL_BROD			 5
#define CL_START         6
#define CL_AH            7 	//by this message a node announces its association to a cluster head
#define CL_BSAH			 8  //Announce headship to BS

// self message types
#define SMSG_SENDDATA     11
#define SMSG_CHANGESTATUS 12
#define SMSG_DATA2BS      13    // cluster head sends data to BS
#define SMSG_STATUS2BS    14    // nodes send their new status to BS
#define SMSG_SINKSTART    15
#define SMSG_CHK    16    // nodes update seq number for expl messages
#define SMSG_INIT         17    // init updates
#define SMSG_MAKETDMA     18
#define SMSG_CONN		  19
#define SMSG_BROD		  20    //Broadcast "I am Cluster Head"
#define SMSG_AH			  21	//Announce my head
#define SMSG_RS			  22    //Round is starting
#define SMSG_END 		  23

#define BROADCAST     0
#define ALIVE         1
#define DEAD          2       // this node is dead

struct _node
{
    int id;
    int status;
    int outgate;
    int hops;
    int bcount;
    int scount;
    int seqNumber;              // per node
};

struct _adv
{
    int id;
    int energy;
    int status;
    int rating;
    int xpos;
    int ypos;
};

struct _peerInfo
{
    int id;
    int xpos;
    int ypos;
};

// define some events and times
#define INIT_WAIT               1 // wait to init nodes

#endif

