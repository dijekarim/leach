#include <stdio.h>
#include <string>
#include <omnetpp.h>
#include "node.h"
#include "bs.h"
#include "cl_msg_m.h"
Define_Module(BS);

void BS::handleMessage(omnetpp::cMessage *msg)
{

    if (msg->isSelfMessage())
    {
        EV << "BS: got self message type " << msg->getKind() << "\n";
        if (msg->getKind() == SMSG_INIT)
        {
            EV << "BS: got start message\n";
            for(int i=1;i<=nrRounds;i++)
            {
            omnetpp::cMessage *cmsg=new omnetpp::cMessage();
            cmsg->setKind(SMSG_RS);
            scheduleAt((omnetpp::simTime()+10*(i-1)),cmsg);
			}
            omnetpp::cMessage *cmsg=new omnetpp::cMessage();
            cmsg->setKind(SMSG_END);
            scheduleAt((omnetpp::simTime()+10*nrRounds),cmsg);
        }
        else if(msg->getKind() == SMSG_RS)
        {
        	this->roundsDone++;
        	resetStatus();
        	EV << "Starting Round";
        	this->initNodes();
		}
		else if(msg->getKind() == SMSG_END)
        {
        	endSimulation();
		}
    }
    else
    {                           // no self message
    	if (((ClusterMessage *) msg)->getProto() == CL_BSAH)
    	{
    		this->nrHead++;
		}
        if (((ClusterMessage *) msg)->getProto() == CL_TOBS)
        {
            int energy;
            int cluster;
            int status;
            int sender;
            int clHead;
            int curHEnergy;     // energy of current head
            int curHStatus;     // status of current head
            int xpos, ypos;

            Status2BSMessage *statusmsg = omnetpp::check_and_cast<Status2BSMessage *>(msg);
            sender = statusmsg->getSrcAddress();
            energy = statusmsg->getEnergy();
            cluster = statusmsg->getCluster();
            status = statusmsg->getStatus();
            xpos = statusmsg->getXpos();
            ypos = statusmsg->getYpos();

            if (this->roundsDone < 300)
                this->roundTimes[roundsDone] = SIMTIME_DBL(omnetpp::simTime());

            
            //ev << "BS received from " << sender << "status " << status << " rating: " << rating << "\n";
            this->nrStatusRec++;
            //ev << "BS rec " << this->nrStatusRec << " nrNodes: "<< this->nrNodes << "\n";

            // check if done
            if (this->roundsDone > this->nrRounds)
                {
                    this->calledEnd = 1;
                    endSimulation();
                }
			
                
            
            else
                EV << "BS: not done\n";
        }
        
    }
 delete(msg);
}
void BS::initialize()
{	
    int xpos, ypos, i;
    cModule *parent = getParentModule();
    this->myId = par("id");
    this->nrGates = (int) parent->par("numNodes") + 2;
    this->xpos = par("xpos");
    this->ypos = par("ypos");
    this->nrNodes = (int) parent->par("numNodes");
    this->xMax = (int) parent->par("xMax");
    this->yMax = (int) parent->par("yMax");
    this->numNodes = (int) parent->par("numNodes");
    this->resetStatus();
    this->roundEnergyLoss = 1;
    this->roundsDone = 0;
    this->nrRounds = (int) parent->par("rounds");
    this->firstDead = 0;
    this->calledEnd = 0;
    this->halfDead = 0;
    this->halfDeadCtr = 0;
    this->deadNodes = 0;
    this->oldDeadNodes = 0;
    this->P = 0.05;
    
    this->cHeadsRound = 0;
    for (i = 0; i < 104; i++)
    {
        advInfo[i].id = 0;
        advInfo[i].energy = 0;
        advInfo[i].status = 0;
    }

    for (i = 0; i < this->nrRounds; i++)
    {
        this->roundTimes[i] = 0;
    }
    
    // schedule first sending
    this->setDisplayString("p=$xpos,$ypos;b=,,rect;o=blue");

    EV << "BS: id " << this->myId << " nrGates: " << this->nrGates << " x,y " << this->
        xpos << this->ypos << "\n";

    {
        omnetpp::cMessage *cmsg = new omnetpp::cMessage();
        cmsg->setKind(SMSG_INIT);
        scheduleAt(omnetpp::simTime(), cmsg);
    }
}
void BS::resetStatus()
{
    int i;

    this->nrStatusRec = 0;      // number of received status
    //this->deadNodes = 0;
    this->nrHead=0;
}

void BS::initNodes()
{
    int i;
    cModule *parent = getParentModule();
    cModule *mod;
    cModule *myMod;             // this is my Module
    int numNodes;
    int trRange;
    int delx, dely;
    int ritems;
	numNodes = (int) parent->par("numNodes");
    EV << "BS numNodes is: " << numNodes << "\n";
    trRange = (int) parent->par("trRange");
    EV << "BS trRange is: " << trRange << "\n";

    for (i = 1; i <= omnetpp::cSimulation().lastModuleIndex(); i++)
    {
        int x, y, id;

        //scan the simulation module vector
        mod = (cModule *) SIMULATION.module(i);

        if (strcmp(mod->name(), "node") == 0)
        {
            id = ((Node *) mod)->myId;
            x = ((Node *) mod)->xpos;
            y = ((Node *) mod)->ypos;
            nodePtr[id] = ((Node *) mod);

            if (id != this->myId)
            {
                cGate *g;
                char gName[32];
                int items;
				
				
                ev << "BS: " << this->myId << "(" << this->xpos << "," << this->
                    ypos << ") found node with id " << id << " xpos: " << x << " ypos: " << y <<
                    "\n";

                items = this->gatev.items();
                ritems = mod->gatev.items();

                ev << "items " << items << "\n";

                // make new gate here
                sprintf(gName, "O_%d", id);
                g = new cGate(gName, 'O');
                this->gatev.addAt(items, g); // position, element
                g->setOwnerModule((cModule *) this, items);

                // make new gate at other side
                sprintf(gName, "I_%d", this->myId);
                g = new cGate(gName, 'I');
                mod->gatev.addAt(ritems, g); // position, element
                g->setOwnerModule((cModule *) mod, ritems);

                //CHANNEL
                cLinkType *etere = findLink("etere");
                connect((cModule *) this, items, (cLinkType *) etere, (cModule *) mod, ritems);
                g = this->gate(items);
				g->setDisplayString( /*g->displayString()*/"o='black,0");
            }
        }
    }
    Start *head=new Start();
    head->setSrcAddress(this->myId);
    head->setDestAddress(BROADCAST);
    head->setProto(CL_START);
    head->setRound(roundsDone);
    char str[32];
    for(int i=2;i<=nrNodes;i++)
        			{
        			if(i==this->myId)
        			continue;
        			sprintf(str, "O_%d", i);
        			Start *copy=(Start *)head->dup();
        			send(copy,str);
        			}
        		sprintf(str, "O_%d", nrNodes+1);
        		send(head,str);
}
void BS::finish()
{
	ev<<Node::fnd<<"\n"<<Node::hnd<<"\n"<<Node::lnd;
}
