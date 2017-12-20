message ClusterMessage
{
        int proto;
        int srcAddress;
        int destAddress;
}

message Start extends ClusterMessage
{
		int round;
}
// Status sent from Node to BS

message Status2BSMessage extends ClusterMessage
{
        int energy;
        int status;
        int cluster;
        int xpos;
        int ypos;
}


// BS sends this message to anounce cluster heads to nodes

message ClusterHeadMessage extends ClusterMessage
{
        int cHead[103];
}


// TDMA message

message TDMAMessage extends ClusterMessage
{
        int frames;
        int frameTime;
        int tdma[103];      // XXX constant

}


// Data sent by nodes to cluster heads

message DataToCHMessage extends ClusterMessage
{
        int data;
}

// Data sent by nodes to BS

message Data2BSMessage extends ClusterMessage
{
        int data;
}


