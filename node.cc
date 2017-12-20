#include <string.h>
#include <omnetpp.h>
#include "node.h"
#include "bs.h"
#include "cl_msg_m.h"
int Node::fnd;
int Node::hnd;
int Node::lnd;
int Node::nrDead;
void Node::handleMessage(cMessage *msg)
{
	int i;
    cModule *parent = parentModule();
    cModule *mod;
    cModule *myMod;             // this is my Module
    int numNodes;
    
    int trRange;
    int delx, dely;
    int ritems,mid;
    float tmp;
	if (!msg->isSelfMessage())
    {
    	if(((Start *)msg)->getProto()==CL_START)
    	{
    		this->curRound=((Start *)msg)->getRound();
    		cMessage *cmsg = new cMessage();
        	cmsg->setKind(SMSG_INIT);
        	scheduleAt(simTime() + INIT_WAIT, cmsg);
       	}
    	else if(((ClusterMessage *)msg)->getProto()==CL_AH)
    	{
    		this->nrChild++;
    	}
    	else if(((ClusterMessage *) msg)->getProto() == CL_BROD) //The nodes arrange themselves in clusters
    	{	cMessage *cmsg2=new cMessage();
    		cmsg2->setKind(SMSG_AH);
    		if(this->cHead==0)
    		{
    		
    		energyReceive(10*8);
    		mid=(((ClusterMessage *)msg)->getSrcAddress());
       		for (i = 1; i <= simulation.lastModuleIndex(); i++)
    		{
        				int x, y, id;
						mod = (cModule *) simulation.module(i);
				
        				if (strcmp(mod->name(), "node") == 0)
           				{	float j,j1;
               				id = ((Node *) mod)->myId;
   							x = ((Node *) mod)->xpos;
   							y = ((Node *) mod)->ypos;
   							nodePtr[id] = ((Node *) mod);
   							if(id!=mid)
   							continue;
   							if(x/10>this->xpos/10)
    						j=pow((x/10-this->xpos/10),2);
    						else
   				 			j=pow((this->xpos/10-x/10),2);
    						if(y/10>this->ypos/10)
   							j1=pow((y/10-this->ypos/10),2);
    						else
    						j1=pow((this->ypos/10-y/10),2);
    						tmp=j1+j;
                			if(tmp<this->headDist)
            				{
            					this->headDist=tmp;
            					this->headId=id;
							}
						}
			}
		}
			if(this->flag==0)
			{
				scheduleAt(simTime()+1,cmsg2);
				this->flag=1;
				ev<<"Distance to my head "<<this->headDist;
			}
		
					                	
		}

        
        //
        // cluster head announces its TDMA scheme with this message
        // received by nodes in cluster
        //
        else if (((ClusterMessage *) msg)->getProto() == CL_TDMA)
        {
            int i, s;
            float sTime;

            // reset data and counters
            this->myData = 0;   // reset myData
            this->curFrame = 0;

            // reduce energy for packet reception of header
            energyReceive(25 * 8);

            this->nrFrames = ((TDMAMessage *) msg)->getFrames();
            this->frameTime = ((TDMAMessage *) msg)->getFrameTime();
            this->headId = ((TDMAMessage *) msg)->getSrcAddress();
            //ev << this->myId << ": received TDMA  frameTime "<< this->frameTime << "\n";

            // find slot for this node
            for (i = 0; this->frameTime; i++)
            {
                if (((TDMAMessage *) msg)->getTdma(i) == this->myId &&
                    ((TDMAMessage *) msg)->getTdma(i) != 0)
                {
                    //ev << this->myId << ": found slot time " << i+1 << "\n";
                    sTime = (i + 1)/10;
                    this->mySlot = i + 1;
                    break;
                }
            }
            
            // sleep until slot time, then wake up and send data
            {
                cMessage *cmsg = new cMessage();
                cmsg->setKind(SMSG_SENDDATA);
                scheduleAt(simTime() + sTime, cmsg);
            }
            //ev << "tdma " << this->myId << "done\n";
        }
        //
        // cluster head receives data
        //
        else if (((ClusterMessage *) msg)->getProto() == CL_CHDATA)
        {
            int newHead = 0;    // XXX update in case someone

            // reduce energy for packet reception of data
            energyReceive(500 * 8);

            // cluster head received data
            this->nrDataPackets++;
            // check if all packets
            if (this->nrDataPackets == (this->nrChild-1)) // XXX
            {
                this->nrDataPackets = 0;
                this->curFrame++;
                if(this->curFrame>this->nrFrames)
                {
                	//Send status to BS
                	cMessage *cmsg=new cMessage();
                	cmsg->setKind(SMSG_STATUS2BS);
                	scheduleAt(simTime()+.1,cmsg);
                }
                else
                {
                	
                	// two "seconds" until processed data is sent to BS
                	cMessage *cmsg = new cMessage();
                	cmsg->setKind(SMSG_DATA2BS);
                	scheduleAt(simTime()+.4, cmsg);
				}
                
            }
        }
    }
    else
    {                           // self message
        if (msg->kind() == SMSG_INIT)
        {
            this->initNodes();
            double p=dblrand();
            int i=1/this->P;
            double t=(this->P)/(1-(this->P*((this->curRound)%i)));
            ev<<this->myId<<" my chosen no. is :"<<p<<" Threshhold is : "<<t;
            if((p<t)&&(this->wasHead==0)&&(this->myStatus==ALIVE))
            {
            	this->cHead=1;
            	this->wasHead=1;
            	this->setDisplayString("p=$xpos,$ypos;o=green");
            	ev<<"Will be Head "<<this->myId;
            	cMessage *cmsg=new cMessage();
            	cmsg->setKind(SMSG_CONN);
            	scheduleAt(simTime()+.1,cmsg);
        	}	
        	if(this->cHead==0)
        	{
        		cMessage *cm=new cMessage();
        		cm->setKind(SMSG_CHK);
        		scheduleAt(simTime()+2.5,cm);
        	}
        		
        }
        else if (msg->kind() == SMSG_SENDDATA)
        {
           
            // node has been woken up to transmit data to clusterhead
            
            sendDataToCHead(0);
            if (this->curFrame < this->nrFrames)
            {
                // sleep until slot time, then wake up and send data
                cMessage *cmsg = new cMessage();
                cmsg->setKind(SMSG_SENDDATA);
                scheduleAt(simTime() + this->frameTime, cmsg);
            }
            
        }
        else if (msg->kind() == SMSG_DATA2BS)
        {
            // cluster head sends data to BS
            sendData2BS(this->myId);
        }
        else if (msg->kind() == SMSG_MAKETDMA)
        {
        	ev<<this->myId<<" has "<<this->nrChild<<"\n";
            // reset some values
            this->curFrame = 0;

            // cluster head sends data to BS
            this->sendTDMA();
        }
        else if (msg->kind() == SMSG_STATUS2BS)
        {
            
            send2BS(this->myId, this->bsId, this->energy, this->myStatus, this->myCluster,
                    this->xpos, this->ypos);
            this->sentBS++;
        }
        else if (msg->kind() == SMSG_CHK)
        {
        	ev<<"Checking if i have a heads!!!!!!\n";
        	if((this->headId==1)&&(this->cHead==0))
        	{ev<<" i DONT!!!!!!!!!!!!!!";
        		sendData2BS(0);
        		//sendData2BS(0);
        		//sendData2BS(0);
        	}
        	if((this->cHead==1)&&(this->nrChild==0))
        	{ev<<" NO CHILDREN :(";
        		sendData2BS(0);
        		//sendData2BS(0);
        		//sendData2BS(0);
        	}
        }
        else if (msg->kind() == SMSG_CONN)
        {   
            cMessage *cmsg1=new cMessage();
            cmsg1->setKind(SMSG_BROD);
            
        	for (i = 1; i <= simulation.lastModuleIndex(); i++)
       		 {
        		int x, y, id;

        		//scan the simulation module vector
        		mod = (cModule *) simulation.module(i);

        		// check for nodes in transmission range
        		if ((strcmp(mod->name(), "node") == 0) || (strcmp(mod->name(), "bs") == 0))
		        {
        		    if (strcmp(mod->name(), "node") == 0)
            		{
                		id = ((Node *) mod)->myId;
                		x = ((Node *) mod)->xpos;
                		y = ((Node *) mod)->ypos;
                		nodePtr[id] = ((Node *) mod);
            		}
            		else
            	if (strcmp(mod->name(), "bs") == 0)
            		{
                	id = ((BS *) mod)->myId;
                	this->bsId = id;
               	    x = ((BS *) mod)->xpos;
                	y = ((BS *) mod)->ypos;
                	this->bsDist =
                    	(x / 10 - this->xpos / 10) * (x / 10 - this->xpos / 10) + (y / 10 -
                                                                               this->ypos / 10) *
                    	(y / 10 - this->ypos / 10);
            		}
            	if (id != this->myId /*&& id == this->bsId*/)
            	{
                	cGate *g;
                	char gName[32];
                	int items;

                	ev << this->myId << "(" << this->xpos << "," << this->ypos <<") found node with id " << id << " xpos: " <<  x << " ypos: " << y<< "\n";

               		if ((((this->ypos - ypos) * (this->ypos - ypos)) +
                    	 ((this->xpos - xpos) * (this->xpos - xpos))) < 2500 || id == this->bsId)
                		{

                    	items = this->gatev.items();
                    	ritems = mod->gatev.items();
						
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
                    	//draw the link
                    	g = this->gate(items);
                    	g->setDisplayString( /*g->displayString()*/"o='black,0");
                		}
            		}
        		}
    		}
    	scheduleAt(simTime()+.05,cmsg1);	
        }	
        else if(msg->kind()==SMSG_BROD)
        {
        	char str[32];
        	ClusterMessage *head=new ClusterMessage();
        	head->setSrcAddress(this->myId);
        	head->setDestAddress(BROADCAST);
        	head->setProto(CL_BROD);
        	float grt=0,j,i,j1,tmp;
        	int x, y, id;
        	int grtid=0;
        	for (i = 1; i <= simulation.lastModuleIndex(); i++)
    		{
        		
				mod = (cModule *) simulation.module(i);
				
        		if ((strcmp(mod->name(), "node") == 0)||(strcmp(mod->name(),"bs")==0))
           		{
           			if(strcmp(mod->name(), "node") == 0)
           			{
           			
               		id = ((Node *) mod)->myId;
   					x = ((Node *) mod)->xpos;
   					y = ((Node *) mod)->ypos;
   					nodePtr[id] = ((Node *) mod);
   					if(x>this->xpos)
    				j=pow((x/10-this->xpos/10),2);
    				else
   				 	j=pow((this->xpos/10-x/10),2);
    				if(y>this->ypos)
   					j1=pow((y/10-this->ypos/10),2);
    				else
    				j1=pow((this->ypos/10-y/10),2);
    				tmp=j1+j;
                	ev<<"\n"<<j<<" "<<j1<<" "<<id;
            		if(tmp>grt)
            		{
            			grt=tmp;
            			grtid=id;
					}
					}
				 if(strcmp(mod->name(),"bs")==0)
				{	
					id = ((BS *) mod)->myId;
   					x = ((BS *) mod)->xpos;
   					y = ((BS *) mod)->ypos;
				}
			}
			}
			
				energyTransmit(10*8,grt);
		  		for(int i=2;i<=nrNodes;i++)
        			{
        			if(i==this->myId)
        			continue;
        			sprintf(str, "O_%d", i);
        			ClusterMessage *copy1=(ClusterMessage *)head->dup();
        			send(copy1,str);
        			
        			}
        		if(this->myId!=nrNodes+1)
        		{
        		sprintf(str, "O_%d", nrNodes+1);
        		send(head,str);
			}
		}	
		else if (msg->kind() == SMSG_AH)
		{
			int id,x,y;
			ev<<"I am node "<<this->myId;
			if(this->cHead==0)
			ev<<"\n "<<this->headId<<" is my head";
			else
			ev<<"\n I am cluster head";
			//Connect to the Cluster Heads
			for (i = 1; i <= simulation.lastModuleIndex(); i++)
			{
			mod = (cModule *) simulation.module(i);

        		    if (strcmp(mod->name(), "node") == 0)
            		{
                		id = ((Node *) mod)->myId;
                		x = ((Node *) mod)->xpos;
                		y = ((Node *) mod)->ypos;
                		nodePtr[id] = ((Node *) mod);
            		}
            		if((this->cHead==1)&&(strcmp(mod->name(),"bs")==0))
            		{
            			id = ((BS *) mod)->myId;
                		this->bsId = id;
               	    	x = ((BS *) mod)->xpos;
                		y = ((BS *) mod)->ypos;
                	}
            	   	if (id == this->headId )
            		{
                	cGate *g;
                	char gName[32];
                	int items;

                	ev << this->myId << "(" << this->xpos << "," << this->ypos <<") found node with id " << id << " xpos: " <<  x << " ypos: " << y<< "\n";

               		if ((((this->ypos - ypos) * (this->ypos - ypos)) +
                    	 ((this->xpos - xpos) * (this->xpos - xpos))) < 2500 || id == this->bsId)
                		{

                    	items = this->gatev.items();
                    	ritems = mod->gatev.items();

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
                    	//draw the link
                    	g = this->gate(items);
                    	g->setDisplayString( /*g->displayString()*/"o=black,0");
                		}
            		}
			}
			
			ClusterMessage *head=new ClusterMessage();
			char str[32];
			if(this->cHead==0)
			{
        	head->setSrcAddress(this->myId);
        	head->setDestAddress(this->headId);
        	head->setProto(CL_AH);
        	energyTransmit(10*8,this->headDist);
		  	sprintf(str, "O_%d", this->headId);
	      	send(head,str);
        	}
        	else
        	{
        		head->setSrcAddress(this->myId);
        		head->setDestAddress(this->bsId);
        		head->setProto(CL_BSAH);
        		energyTransmit(10*8,this->bsDist);
        		sprintf(str,"O_%d",this->bsId);
        		send(head,str);
			}
        	cMessage *msg1=new cMessage();
        	msg1->setKind(SMSG_MAKETDMA);
        	if(this->cHead==1)
        	{
        		scheduleAt(simTime()+.3,msg1);
			}
		}	
        else
        {
            ev << "got strange self-message!!\n";
        }
    }
    delete(msg);
}
void Node::initialize()
{
	int i;
    cMessage *cm;
    cMessage *cm2;
    cModule *parent = parentModule();
    this->initStats();
    this->myId = par("id");
    this->xpos = par("xpos");
    this->ypos = par("ypos");
    this->nrGates = (int) parent->par("numNodes") + 3;
    this->energy = 100000;
    this->myCluster = 0;
    this->nrNodes = (int) parent->par("numNodes");
    this->nrFrames = (int) parent->par("frames");
    this->nrRounds = (int) parent->par("rounds");
    this->curRound=0;
    this->myData = 0;
    this->curFrame = 0;
    this->cHead=0;
    this->headId=1;
    this->sentBS = 0;
    this->wasHead = 0;
    this->nrChild=0;
    this->flag=0;
    Node::fnd=0;
    Node::hnd=0;
    Node::lnd=0;
    Node::nrDead=0;
    this->lastClusterRound = 0;
    this->headDist=99999999.99999999;
    this->myStatus=ALIVE;
    this->P=0.05;
    // default, set red
    this->setDisplayString("p=$xpos,$ypos;o=red");

    ev << this->myId << " energy: " << energy << " status " << this->myStatus << "\n";

    
}
void Node::finish()
{
;
}
void Node::initNodes()
{
	this->cHead=0;
    this->headId=1;
   	this->curFrame = 0;
   	if(((this->curRound)%10)==0)
   	this->wasHead = 0;
   	this->headDist=99999999.99999999;
   	this->flag=0;
   	this->nrChild=0;
   	this->bsId=1;
   	if(this->myStatus==ALIVE)
   	this->setDisplayString("p=$xpos,$ypos;o=red");
   	else
   	this->setDisplayString("p=$xpos,$ypos;o=black");
}
void Node::send2BS(int src, int rec, int energy, int status, int cluster, int xpos, int ypos)
{

    // send out a TOCLUSTER message
    Status2BSMessage *cmsg = new Status2BSMessage(); // create new status2BS message
    cmsg->setProto(CL_TOBS);
    cmsg->setSrcAddress(this->myId); // XXX node id
    cmsg->setDestAddress(rec);
    cmsg->setStatus(-2);
    cmsg->setEnergy(energy);
    cmsg->setCluster(cluster);
    cmsg->setYpos(ypos);
    cmsg->setXpos(xpos);

    ev << "in send2BS:" << this->myId << "\n";
    {
        char str[32];

        sprintf(str, "O_%d", rec);
        if (findGate(str) > 1)
        {
            cGate *g = gate(str);
            if (g->isConnected())
            {
                ev << this->myId << ": sToCluster to" << rec << "\n";
                send((Status2BSMessage *) cmsg, str);
                // reduce energy for header
                this->energyTransmit(25 * 8, this->bsDist);
            }
        }
    }
}
void Node::sendTDMA()
{
    int n;
    int nrPeers = 0;            // number of nodes in my cluster
    char str[32];
    int i, j;
    TDMAMessage *tmsg;
    int items, ritems;
    cGate *g;
    char gName[32];

    // create new Message including TDMA slots
    tmsg = new TDMAMessage();
    tmsg->setProto(CL_TDMA);
    tmsg->setSrcAddress(this->myId);
    tmsg->setDestAddress(BROADCAST);

    // compute number of nodes in cluster and update cluster head
    for (n = 2; n < this->nrNodes + 2; n++)
    {
        if (n != this->myId && nodePtr[n]->headId == this->myId)
        {
            // set TDMA slot
            tmsg->setTdma(nrPeers, n);
            ev << "tdma " << nrPeers << ": " << n << "\n";
            
            // compute distance as well
            nodePtr[n]->headDist =
                (((nodePtr[n])->xpos / 10 - this->xpos / 10) * ((nodePtr[n])->xpos / 10 -
                                                                this->xpos / 10)) +
                (((nodePtr[n])->ypos / 10 - this->ypos / 10) * ((nodePtr[n])->ypos / 10 -
                                                                this->ypos / 10));
            ev << "dist: " << nodePtr[n]->headDist << "\n";
            nrPeers++;
        }
    }
    ev << "clusterhead " << this->myId << ": have peers " << nrChild << "\n";

    tmsg->setFrames(this->nrFrames); // XXX nr Frames ???
    tmsg->setFrameTime(nrChild + 2); // XXX frame length?

    this->clusterNodes = nrChild;
    // give two slots to cluster head at the end of each round
    tmsg->setTdma(nrChild, this->myId);
    tmsg->setTdma(nrChild + 1, this->myId);

    for (i = 0; i < nrChild; i++)
    {
        int rec = tmsg->getTdma(i);
        sprintf(str, "O_%d", rec);
        if (findGate(str) > 1)
        {
            cGate *g = gate(str);
            if (g->isConnected())
            {
                //ev << this->myId << ": sTDMA to" << rec << "\n";
                send((TDMAMessage *) tmsg->dup(), str);
                // reduce energy for header
                this->energyTransmit(25 * 8, nodePtr[rec]->headDist);
            }
        }
        else
            ev << "no gate\n";
    }

    
}

void Node::sendDataToCHead(int newSun)
{
    int rec = this->headId;     // this data is sent to the cluster head
	if(rec==1)
	return;
    // send out a TOCLUSTER message
    DataToCHMessage *dmsg = new DataToCHMessage();
    dmsg->setProto(CL_CHDATA);
    dmsg->setSrcAddress(this->myId); // XXX node id
    dmsg->setDestAddress(rec);
    dmsg->setData(this->myData);
    this->myData++;
    {
        char str[32];

        sprintf(str, "O_%d", rec);
        if (findGate(str) > 1)
        {
            cGate *g = gate(str);
            if (g->isConnected())
            {
                //ev << this->myId << ": sDataCH to" << rec << "\n";
                send((DataToCHMessage *) dmsg, str);
                // reduce energy for data packet
                this->energyTransmit(500 * 8, this->headDist);
            }
        }
    }
}

void Node::sendData2BS(int data)
{
    if(this->myStatus!=ALIVE)
    return;
    int rec = this->bsId;       // this data is sent to the cluster head

    // send out a TOCLUSTER message
    Data2BSMessage *dmsg = new Data2BSMessage(); // create new toCenter message
    dmsg->setProto(CL_CHDATA);
    dmsg->setSrcAddress(this->myId); // XXX node id
    dmsg->setDestAddress(rec);
    dmsg->setData(this->myData);
    this->myData++;

    {
        char str[32];

        sprintf(str, "O_%d", rec);
        if (findGate(str) > 1)
        {
            cGate *g = gate(str);
            if (g->isConnected())
            {
                //ev << this->myId << ": sDataCH to" << rec << "\n";
                send((Data2BSMessage *) dmsg, str);
                // reduce energy for data packet to BS
                this->energyTransmit(250 * 8, this->bsDist);
            }
        }
    }

    // reduce energy for data aggregation
    energyDataAggr(this->clusterNodes);

}
void Node::energyReceive(int bits)
{
    double en;                  // lost energy

    if (this->myStatus == ALIVE)
    {
        en = (double) bits *50.0 * NANO;
        //ev << "REC-energy: " << en << "\n";
        this->energy = this->energy - en;
        //ev<< this->energy <<" is left in "<<this->myId;
    }
    if((this->energy<=0)&&(this->myStatus==ALIVE))
    {
    	this->myStatus=DEAD;
    	this->setDisplayString("p=$xpos,$ypos;o=black");
    	ev<<"DEAD";
    	Node::nrDead++;
    	if(Node::fnd==0)
    	fnd=this->curRound;
    	if(Node::nrDead==(this->nrNodes/2))
    	Node::hnd=curRound;
    	if(Node::nrDead==this->nrNodes)
    	Node::lnd=curRound;
	}
}

// energy consumption for transmission of bits over distance dist
// node that dist is already the square
void Node::energyTransmit(int bits, int dist)
{
    double en;

    if (this->myStatus == ALIVE)
    {
        en = bits * 10 * PICO * dist + (double) bits *50.0 * NANO;
        if (this->cHead == 1)
        //ev << "TR-energy HEAD: " << en << "\n";
        this->energy = this->energy - en;
        ev<< this->energy <<" is left in "<<this->myId;
    }
    if((this->energy<=0)&&(this->myStatus==ALIVE))
    {
    	this->myStatus=DEAD;
    	this->setDisplayString("p=$xpos,$ypos;o=black");
    	nrDead=nrDead+1;
    	if(fnd==0)
    	fnd=this->curRound;
    	if(Node::nrDead==(this->nrNodes/2))
    	Node::hnd=curRound;
    	if(Node::nrDead==this->nrNodes)
    	Node::lnd=curRound;
	}
    
}
void Node::energyDataAggr(int signals)
{
    double en;

    if (this->myStatus == ALIVE)
    {
        en = 5 * NANO * signals;
        //ev << "AGG-energy: " << en << "\n";
        this->energy = this->energy - en;
    }
}

