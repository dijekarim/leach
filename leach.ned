
module leach
    parameters:
        numNodes,
        trRange,
        rounds,
        frames,
        sRand,
        xMax,
        yMax;
    submodules:
        bs: BS;
            parameters:
                id = 1,
                randInit = sRand,
                xpos = xMax/2,
                numNodes = numNodes,
                ypos = 1750;
        node: Node[numNodes];
            parameters:
                energy = 10000,
                id = index+2,
                xpos = intuniform(0, xMax),
                ypos = intuniform(0, yMax);
    connections nocheck:
endmodule

//
// Instantiates the network
//
network leach1: leach
endnetwork
