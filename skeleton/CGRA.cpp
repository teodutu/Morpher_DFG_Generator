#include "CGRA.h"

void CGRA::connectNeighbors() {

	int sizeAllPhyNodes = MII*YDim*XDim;

	for (int i = 0; i < sizeAllPhyNodes; ++i) {
		std::vector<int> temp;
		for (int j = 0; j < sizeAllPhyNodes; ++j) {
			temp.push_back(0);
		}
		phyConMat.push_back(temp);
	}


	for (int t = 0; t < MII; ++t) {
		for (int y = 0; y < YDim; ++y) {
			for (int x = 0; x < XDim; ++x) {

				//North
				if(y-1 >= 0){
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y-1][x],1,"north");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((t+1)%MII,y-1,x)] = 1;
				}

				//East
				if(x+1 < XDim){
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y][x+1],1,"east");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((t+1)%MII,y,x+1)] = 1;
				}

				//South
				if(y+1 < YDim){
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y+1][x],1,"south");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((t+1)%MII,y+1,x)] = 1;
				}

				//West
				if(x-1 >= 0){
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y][x-1],1,"west");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((t+1)%MII,y,x-1)] = 1;
				}

				//Time
//				if(t+1 < MII){
//					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y][x],0,"next_cycle");
//				}

				for (int i = t+1; i < t+MII; ++i) {
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(i)%MII][y][x],i-(t+1),"REGconnections");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((i)%MII,y,x)] = 1;
				}

				//Connecting it self
				phyConMat[getConMatIdx(t,y,x)][getConMatIdx(t,y,x)] = 1;


				CGRANodes[t][y][x].sortConnectedNodes();

			}
		}
	}

}

CGRA::CGRA(int MII, int Xdim, int Ydim, int regs, ArchType aType) {

	this->MII = MII;
	this->XDim = Xdim;
	this->YDim = Ydim;
	this->regsPerNode = regs;
	this->arch = aType;

	for (int t = 0; t < MII; ++t) {
		std::vector<std::vector<CGRANode> > tempL2;
		for (int y = 0; y < Ydim; ++y) {
			std::vector<CGRANode> tempL1;
			for (int x = 0; x < Xdim; ++x) {
				CGRANode tempNode(x,y,t,this);
				tempL1.push_back(tempNode);
			}
			tempL2.push_back(tempL1);
		}
		CGRANodes.push_back(tempL2);
	}


	InOutPortMap[NORTH] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};
	InOutPortMap[EAST] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};
	InOutPortMap[WEST] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};
	InOutPortMap[SOUTH] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};
	switch (arch) {
		case DoubleXBar:
			InOutPortMap[R0] = {R0,NORTH,EAST,WEST,SOUTH};
			InOutPortMap[R1] = {R1,NORTH,EAST,WEST,SOUTH};
			InOutPortMap[R2] = {R2,NORTH,EAST,WEST,SOUTH};
			InOutPortMap[R3] = {R3,NORTH,EAST,WEST,SOUTH};
			InOutPortMap[TILE] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};

			break;
		case RegXbar:
			InOutPortMap[R0] = {R0,NORTH};
			InOutPortMap[R1] = {R1,EAST};
			InOutPortMap[R2] = {R2,WEST};
			InOutPortMap[R3] = {R3,SOUTH};
			InOutPortMap[TILE] = {NORTH,EAST,WEST,SOUTH};
			break;

		case LatchXbar:
			InOutPortMap[R0] = {NORTH};
			InOutPortMap[R1] = {EAST};
			InOutPortMap[R2] = {WEST};
			InOutPortMap[R3] = {SOUTH};
			InOutPortMap[TILE] = {NORTH,EAST,WEST,SOUTH};
			break;
	}

//	connectNeighbors();
//	connectNeighborsMESH();
	connectNeighborsSMART();
//	connectNeighborsGRID();

}

int CGRA::getXdim() {
	return XDim;
}

int CGRA::getYdim() {
	return YDim;
}

CGRANode* CGRA::getCGRANode(int t, int y, int x) {

	if(t >= getMII()){
		errs() << "t = " << t << " MII = " << getMII() << "\n";
	}

	assert(t < getMII());
	assert(y < getYdim());
	assert(x < getXdim());

	return &CGRANodes[t][y][x];
}

CGRANode* CGRA::getCGRANode(int phyLoc) {
	assert(phyLoc < getMII()*getYdim()*getXdim());

	int t = phyLoc/(getYdim()*getXdim());
	int y = (phyLoc % getMII())/(getXdim());
	int x = ((phyLoc % getMII()) % getYdim());

	return &CGRANodes[t][y][x];
}

void CGRA::connectNeighborsMESH() {
	for (int t = 0; t < MII; ++t) {
		for (int y = 0; y < YDim; ++y) {
			for (int x = 0; x < XDim; ++x) {

				for (int yy = 0; yy < YDim; ++yy) {
					for (int xx = 0; xx < XDim; ++xx) {
						CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][yy][xx],abs(yy-y) + abs(xx-x) + 1,"mesh");
					}
				}

				for (int i = t+1; i < t+MII; ++i) {
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(i)%MII][y][x],i-(t+1),"REGconnections");
				}

				CGRANodes[t][y][x].sortConnectedNodes();

			}
		}
	}

}

void CGRA::connectNeighborsSMART() {
	for (int t = 0; t < MII; ++t) {
		for (int y = 0; y < YDim; ++y) {
			for (int x = 0; x < XDim; ++x) {

				//Connect All the nodes in the time axis
//				for (int i = t+1; i < t+MII; ++i) {
//					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(i)%MII][y][x],i-(t+1),"REGconnections");

//				for (int reg = 0; reg < regsPerNode; ++reg) {
//					CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y][x]);
//					CGRANodes[t][y][x].originalEdgesSize++;
//				}

				assert(regsPerNode == 4);
				CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],R0,&CGRANodes[(t+1)%MII][y][x],R0));
				CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],R1,&CGRANodes[(t+1)%MII][y][x],R1));
				CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],R2,&CGRANodes[(t+1)%MII][y][x],R2));
				CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],R3,&CGRANodes[(t+1)%MII][y][x],R3));
				CGRANodes[t][y][x].originalEdgesSize += 4;
//				}


						if(x > 0){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[t][y][x-1]);
							CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],EAST,&CGRANodes[t][y][x-1],WEST));
							CGRANodes[t][y][x].originalEdgesSize++;
						}

						if(x < XDim - 1){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[t][y][x+1]);
							CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],WEST,&CGRANodes[t][y][x+1],EAST));
							CGRANodes[t][y][x].originalEdgesSize++;
						}

						if(y > 0){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[t][y-1][x]);
							CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],NORTH,&CGRANodes[t][y-1][x],SOUTH));
							CGRANodes[t][y][x].originalEdgesSize++;
						}

						if(y < YDim - 1){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[t][y+1][x]);
							CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],SOUTH,&CGRANodes[t][y+1][x],NORTH));
							CGRANodes[t][y][x].originalEdgesSize++;
						}
			}
		}
	}

}

int CGRA::getMII() {
	return MII;
}



int CGRA::getConMatIdx(int t, int y, int x) {
	return t*YDim*XDim + y*XDim + x;
}

void CGRA::connectNeighborsGRID() {
	for (int t = 0; t < MII; ++t) {
		for (int y = 0; y < YDim; ++y) {
			for (int x = 0; x < XDim; ++x) {

//				for (int reg = 0; reg < regsPerNode; ++reg) {
//					CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y][x]);
//					CGRANodes[t][y][x].originalEdgesSize++;
//				}

				assert(regsPerNode == 4);
				CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],R0,&CGRANodes[(t+1)%MII][y][x],R0));
				CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],R1,&CGRANodes[(t+1)%MII][y][x],R1));
				CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],R2,&CGRANodes[(t+1)%MII][y][x],R2));
				CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],R3,&CGRANodes[(t+1)%MII][y][x],R3));
				CGRANodes[t][y][x].originalEdgesSize += 4;


//				for (int yy = 0; yy < YDim; ++yy) {
//					for (int xx = 0; xx < XDim; ++xx) {

						if(x > 0){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y][x-1]);
							CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],EAST,&CGRANodes[(t+1)%MII][y][x-1],WEST));
							CGRANodes[t][y][x].originalEdgesSize++;
						}

						if(x < XDim - 1){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y][x+1]);
							CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],WEST,&CGRANodes[(t+1)%MII][y][x+1],EAST));
							CGRANodes[t][y][x].originalEdgesSize++;
						}

						if(y > 0){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y-1][x]);
							CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],NORTH,&CGRANodes[(t+1)%MII][y-1][x],SOUTH));
							CGRANodes[t][y][x].originalEdgesSize++;
						}

						if(y < YDim - 1){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y+1][x]);
							CGRAEdges[&CGRANodes[t][y][x]].push_back(CGRAEdge(&CGRANodes[t][y][x],SOUTH,&CGRANodes[(t+1)%MII][y+1][x],NORTH));
							CGRANodes[t][y][x].originalEdgesSize++;
						}
//						CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][yy][xx],abs(yy-y) + abs(xx-x) + 1,"mesh");
//					}
//				}




			}
		}
	}
}

void CGRA::clearMapping() {

}

std::vector<CGRAEdge*> CGRA::findCGRAEdges(CGRANode* currCNode, Port inPort,std::map<CGRANode*,std::vector<CGRAEdge>>* cgraEdgesPtr) {
	std::vector<CGRAEdge*> candidateCGRAEdges;

	for (int i = 0; i < InOutPortMap[inPort].size(); ++i) {
		for (int j = 0; j < (*cgraEdgesPtr)[currCNode].size(); ++j) {
			if((*cgraEdgesPtr)[currCNode][j].mappedDFGEdge == NULL){
				if((*cgraEdgesPtr)[currCNode][j].SrcPort == InOutPortMap[inPort][i]){
					candidateCGRAEdges.push_back(&(*cgraEdgesPtr)[currCNode][j]);
				}
			}
		}
	}
	return candidateCGRAEdges;
}
