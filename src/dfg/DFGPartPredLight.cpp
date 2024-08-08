#include <morpherdfggen/dfg/DFGPartPredLight.h>

#include "llvm/Analysis/CFG.h"
#include <algorithm>
#include <queue>
#include <map>
#include <set>
#include <vector>

// #include "RemoveAGI.h"
// #include "llvm/Analysis/IVUsers.h"

using namespace std;
#define LV_NAME "dfg_gen" //"sfp"
#define DEBUG_TYPE LV_NAME






// this function is needed.
void DFGPartPredLight::generateTrigDFGDOT(Function &F) {


	std::set<exitNode> exitNodes;

	getLoopExitConditionNodes(exitNodes);
	//	connectBB();
	removeAlloc();
	connectBBTrig();
	createCtrlBROrTree();
	printDOT(this->name + "bef_handlePHINodes_PartPredDFG.dot");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][handlePHINodes begin]\n");
	handlePHINodes(this->loopBB);

	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][handlePHINodes end]\n");
	printDOT(this->name + "after_handlePHINodes_PartPredDFG.dot");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][handleSELECTNodes begin]\n");
	handleSELECTNodes();

	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][handleSELECTNodes end]\n");
	printDOT(this->name + "after_handleSELECTNodes_PartPredDFG.dot");

	//exit(true);

	// insertshiftGEPs();
	addMaskLowBitInstructions();
	insertshiftGEPsCorrect();
	//removeDisconnetedNodes();
	//	scheduleCleanBackedges();
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][fillCMergeMutexNodes begin]\n");
	fillCMergeMutexNodes();
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][fillCMergeMutexNodes end]\n");


	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][constructCMERGETree begin]\n");
	constructCMERGETree();
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][constructCMERGETree end]\n");

	//	printDOT(this->name + "_PartPredDFG.dot"); return;
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][addLoopExitStoreHyCUBE begin]\n");
	addLoopExitStoreHyCUBE(exitNodes);
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][addLoopExitStoreHyCUBE end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][handlestartstop begin]\n");
	handlestartstop();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][handlestartstop end] Nodelist size:" <<NodeList.size()<<"\n\n");

	//	printDOT(this->name + "afterhandlestartstop_PartPredDFG.dot");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][scheduleASAP begin]\n");
	scheduleASAP();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][scheduleASAP end]\n\n");
	//	printDOT(this->name + "afterscheduleASAP_PartPredDFG.dot");
	//	return;
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][scheduleALAP begin]\n");
	scheduleALAP();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][scheduleALAP end]\n\n");
	// assignALAPasASAP();
	//	balanceSched();

    


	cerr <<"Remove AGI instructions for morpher light. "<< endl;
	int baseline_node_count = NodeList.size();
	removeAGI();
	int post_removal_node_count = NodeList.size();
	LLVM_DEBUG(dbgs() << "-------------------------------------BASELINE NODE COUNT = " << baseline_node_count << "\n");
	LLVM_DEBUG(dbgs() << "-------------------------------------POST REMOVAL NODE COUNT = " << post_removal_node_count << "\n");

	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][scheduleASAP begin]\n");
	scheduleASAP();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][scheduleASAP end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][scheduleALAP begin]\n");
	scheduleALAP();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][scheduleALAP end]\n\n");

	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][nameNodes begin]\n");
	nameNodes();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][nameNodes end]\n\n");

	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][classifyParents begin]\n");
	classifyParents();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][classifyParents end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][removeDisconnectedNodes begin]\n");
	removeDisconnetedNodes();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][removeDisconnectedNodes end]\n\n");
//
//	changeTypeofSingleSourceCompNodes();
//	removeDisconnetedNodes();
	printDOT(this->name + "_AGIremovedDFG.dot");
	printDOT(this->name + "PartPredDFG.dot");
	printNewDFGXML();


}




//this function is needed.
void DFGPartPredLight::scheduleASAP() {

	for(dfgNode* n : NodeList){
		n->setASAPnumber(-1);
	}

	std::queue<std::vector<dfgNode*>> q;
	std::vector<dfgNode*> qv;
	for(std::pair<BasicBlock*,dfgNode*> p : startNodes){
		qv.push_back(p.second);
	}

	for(dfgNode* n : NodeList){
		//		if(n->getNameType() == "OutLoopLOAD"){
		if(n->getAncestors().size() == 0){
			qv.push_back(n);
		}
		//		}
	}

	q.push(qv);

	int level = 0;

	std::set<dfgNode*> visitedNodes;

	LLVM_DEBUG(dbgs() << "Schedule ASAP ..... \n");


	while(!q.empty()){
		std::vector<dfgNode*> q_element = q.front(); q.pop();
		std::vector<dfgNode*> nqv; nqv.clear();

		std::set<dfgNode*> nqv_set; nqv_set.clear();
		std::pair<std::set<dfgNode*>::iterator,bool> ret;

		if(level > maxASAPLevel) maxASAPLevel = level;


		for(dfgNode* node : q_element){
			visitedNodes.insert(node);

			if(node->getASAPnumber() < level){
				node->setASAPnumber(level);
			}



			for(dfgNode* child : node->getChildren()){
				bool isBackEdge = node->childBackEdgeMap[child];
				if(!isBackEdge){
					//					nqv.push_back(child);
					ret = nqv_set.insert(child);
					if(ret.second == true){
						nqv.push_back(child);
					}
				}
			}

		}
		if(!nqv.empty()){
			q.push(nqv);
		}
		level++;
	}
// #ifdef REMOVE_AGI
	LLVM_DEBUG(dbgs() << "Visited nodes size : " << visitedNodes.size() << ", Nodes list size:" << NodeList.size() << "\n");
// #else
// 	assert(visitedNodes.size() == NodeList.size());
// #endif
}




//this function is needed
void DFGPartPredLight::printNewDFGXML() {


	std::string fileName = kernelname + "_PartPredDFG.xml";
// #ifdef REMOVE_AGI
	fileName = kernelname + "_PartPred_AGI_REMOVED_DFG.xml";
// #endif
	std::ofstream xmlFile;
	xmlFile.open(fileName.c_str());



	//    insertMOVC();
	//	scheduleASAP();
	//	scheduleALAP();
	//	balanceASAPALAP();
	//				  LoopDFG.addBreakLongerPaths();
	CreateSchList();

	std::map<BasicBlock*,std::set<BasicBlock*>> mBBs = checkMutexBBs();
	std::map<std::string,std::set<std::string>> mBBs_str;

	std::map<int,std::vector<dfgNode*>> asaplevelNodeList;
	for (dfgNode* node : NodeList){
		asaplevelNodeList[node->getASAPnumber()].push_back(node);
	}

	std::map<dfgNode*,std::string> nodeBBModified;
	for(dfgNode* node : NodeList){
		nodeBBModified[node]=node->BB->getName().str();
	}


	for(dfgNode* node : NodeList){
		int cmergeParentCount=0;
		std::set<std::string> mutexBBs;
		for(dfgNode* parent: node->getAncestors()){
			if(HyCUBEInsStrings[parent->getFinalIns()] == "CMERGE" || parent->getNameType() == "SELECTPHI"){
				nodeBBModified[parent]=nodeBBModified[parent]+ "_" + std::to_string(node->getIdx()) + "_" + std::to_string(cmergeParentCount);
				mutexBBs.insert(nodeBBModified[parent]);
				cmergeParentCount++;
			}
		}
		for(std::string bb_str1 : mutexBBs){
			for(std::string bb_str2 : mutexBBs){
				if(bb_str2==bb_str1) continue;
				mBBs_str[bb_str1].insert(bb_str2);
			}
		}
	}


	xmlFile << "<MutexBB>\n";
	for(std::pair<BasicBlock*,std::set<BasicBlock*>> pair : mBBs){
		BasicBlock* first = pair.first;
		xmlFile << "<BB1 name=\"" << first->getName().str() << "\">\n";
		for(BasicBlock* second : pair.second){
			xmlFile << "\t<BB2 name=\"" << second->getName().str() << "\"/>\n";
		}
		xmlFile << "</BB1>\n";
	}
	for(std::pair<std::string,std::set<std::string>> pair : mBBs_str){
		std::string first = pair.first;
		xmlFile << "<BB1 name=\"" << first << "\">\n";
		for(std::string second : pair.second){
			xmlFile << "\t<BB2 name=\"" << second << "\"/>\n";
		}
		xmlFile << "</BB1>\n";
	}
	xmlFile << "</MutexBB>\n";

	xmlFile << "<DFG count=\"" << NodeList.size() << "\">\n";
	std::cout << "DFG node count: " << NodeList.size() << "\n";

	for(dfgNode* node : NodeList){
		//	for (int i = 0; i < maxASAPLevel; ++i) {
		//		for(dfgNode* node : asaplevelNodeList[i]){
		xmlFile << "<Node idx=\"" << node->getIdx() << "\"";
		xmlFile << " ASAP=\"" << node->getASAPnumber() << "\"";
		xmlFile << " ALAP=\"" << node->getALAPnumber() << "\"";

		//		    if(node->getNameType() == "OutLoopLOAD") {
		//		    	xmlFile << "OutLoopLOAD=\"1\"";
		//		    }
		//		    else{
		//		    	xmlFile << "OutLoopLOAD=\"0\"";
		//		    }
		//
		//		    if(node->getNameType() == "OutLoopSTORE") {
		//		    	xmlFile << "OutLoopSTORE=\"1\"";
		//		    }
		//		    else{
		//		    	xmlFile << "OutLoopSTORE=\"0\"";
		//		    }

		//		    xmlFile << "BB=\"" << node->BB->getName().str() << "\"";
		xmlFile << "BB=\"" << nodeBBModified[node] << "\"";
		if(node->hasConstantVal()){
			xmlFile << "CONST=\"" << node->getConstantVal() << "\"";
		}
		xmlFile << ">\n";

		xmlFile << "<OP>";
		if((node->getNameType() == "OutLoopLOAD") || (node->getNameType() == "OutLoopSTORE") ){
			xmlFile << "O";
		}
		xmlFile << HyCUBEInsStrings[node->getFinalIns()] << "</OP>\n";

		if(node->getArrBasePtr() != "NOT_A_MEM_OP"){
			xmlFile << "<BasePointerName size=\"" << array_pointer_sizes[node->getArrBasePtr()] << "\">";
			xmlFile << node->getArrBasePtr();
			xmlFile << "</BasePointerName>\n";
		}

//#ifdef ARCHI_16BIT
		if(node->getNameType() == "LOOPSTART") 
			xmlFile << "<BasePointerName size=\"1\">loopstart</BasePointerName>\n";
		if(node->getNameType() == "LOOPEXIT") 
			xmlFile << "<BasePointerName size=\"1\">loopend</BasePointerName>\n";
		if(node->getNameType() == "STORESTART") 
			xmlFile << "<BasePointerName size=\"1\">loopstart</BasePointerName>\n";
//#endif	
		if(node->getGEPbaseAddr() != -1){
			GetElementPtrInst* GEP = cast<GetElementPtrInst>(node->getNode());
			int gep_offset = GEPOffsetMap[GEP];
			xmlFile << "<GEPOffset>";
			xmlFile << gep_offset;
			xmlFile << "</GEPOffset>\n";
		}

		//			xmlFile << "<OP>" << HyCUBEInsStrings[node->getFinalIns()] << "</OP>\n";

		xmlFile << "<Inputs>\n";
		for(dfgNode* parent : node->getAncestors()){
			//			xmlFile << "\t<Input idx=\"" << parent->getIdx() << "\" type=\"DATA\"/>\n";
			xmlFile << "\t<Input idx=\"" << parent->getIdx() << "\"/>\n";
		}
		//		for(dfgNode* parentPHI : node->getPHIancestors()){
		//			xmlFile << "\t<Input idx=\"" << parentPHI->getIdx() << "\" type=\"PHI\"/>\n";
		//		}
		xmlFile << "</Inputs>\n";

		xmlFile << "<Outputs>\n";
		for(dfgNode* child : node->getChildren()){
			//			xmlFile << "\t<Output idx=\"" << child->getIdx() <<"\" type=\"DATA\"/>\n";
			xmlFile << "\t<Output idx=\"" << child->getIdx() << "\" ";

			if(node->childBackEdgeMap[child]){
				xmlFile << "nextiter=\"1\" ";
			}
			else{
				xmlFile << "nextiter=\"0\" ";
			}


			bool written=false;
			if(findEdge(node,child)->getType() == EDGE_TYPE_PS){
				xmlFile << "type=\"PS\"/>\n";
				written=true;
			}
			else if(node->getNameType()=="CMERGE"){
				if(child->getNode()){
					if(dyn_cast<PHINode>(child->getNode())){

					}
					else if(dyn_cast<SelectInst>(child->getNode())){

					}
					else{ // if not phi
						written = true;
						int operand_no = findOperandNumber(child,child->getNode(),cmergePHINodes[node]->getNode());
						if( operand_no == 0){
							child->parentClassification[0]=node;
							if(child->getNPB()){
								xmlFile << "NPB=\"1\" ";
							}
							else{
								xmlFile << "NPB=\"0\" ";
							}
							xmlFile << "type=\"P\"/>\n";
						}
						else if ( operand_no == 1){
							child->parentClassification[1]=node;
							xmlFile << "type=\"I1\"/>\n";
						}
						else if(( operand_no == 2)){
							child->parentClassification[2]=node;
							xmlFile << "type=\"I2\"/>\n";
						}
						else{
							LLVM_DEBUG(dbgs() << "node=" << node->getIdx() << ",child=" << child->getIdx() << "\n");
							assert(false);
						}
					}
				}
			}
			else if(node->getNameType()=="SELECTPHI" && (child != selectPHIAncestorMap[node])){
				if(child->getNode()){
					written = true;
					LLVM_DEBUG(dbgs() << "SELECTPHI :: " << node->getIdx());
					LLVM_DEBUG(dbgs() << ",child = " << child->getIdx() << " | "); LLVM_DEBUG(child->getNode()->dump());
					LLVM_DEBUG(dbgs() << ",phiancestor = " << selectPHIAncestorMap[node]->getIdx() << " | ");
					LLVM_DEBUG(selectPHIAncestorMap[node]->getNode()->dump());

					int operand_no = findOperandNumber(child,child->getNode(),selectPHIAncestorMap[node]->getNode());
					if( operand_no == 0){
						child->parentClassification[0]=node;
						if(child->getNPB()){
							xmlFile << "NPB=\"1\" ";
						}
						else{
							xmlFile << "NPB=\"0\" ";
						}
						xmlFile << "type=\"P\"/>\n";
					}
					else if ( operand_no == 1){
						child->parentClassification[1]=node;
						xmlFile << "type=\"I1\"/>\n";
					}
					else if(( operand_no == 2)){
						child->parentClassification[2]=node;
						xmlFile << "type=\"I2\"/>\n";
					}
					else{
						LLVM_DEBUG(dbgs() << "node = " << node->getIdx() << ", child = " << child->getIdx() << "\n");
						assert(false);
					}
				}
			}

			if(Edge2OperandIdxMap.find(node) != Edge2OperandIdxMap.end()
					&& Edge2OperandIdxMap[node].find(child) != Edge2OperandIdxMap[node].end()){
				int operand_no = Edge2OperandIdxMap[node][child];
				if( operand_no == 0){
					child->parentClassification[0]=node;
					if(child->getNPB()){
						xmlFile << "NPB=\"1\" ";
					}
					else{
						xmlFile << "NPB=\"0\" ";
					}
					xmlFile << "type=\"P\"/>\n";
				}
				else if ( operand_no == 1){
					child->parentClassification[1]=node;
					xmlFile << "type=\"I1\"/>\n";
				}
				else if(( operand_no == 2)){
					child->parentClassification[2]=node;
					xmlFile << "type=\"I2\"/>\n";
				}
				written = true;
			}

			if(written){
			}
			else if(child->parentClassification[0]==node){
				if(child->getNPB()){
					xmlFile << "NPB=\"1\" ";
				}
				else{
					xmlFile << "NPB=\"0\" ";
				}
				xmlFile << "type=\"P\"/>\n";
			}
			else if(child->parentClassification[1]==node){
				xmlFile << "type=\"I1\"/>\n";
			}
			else if(child->parentClassification[2]==node){
				xmlFile << "type=\"I2\"/>\n";
			}
			else if(child->parentClassification[3]==node){
				xmlFile << "type=\"I3\"/>\n";// type to handle single source nodes in mapper
			}
			else{
				bool found=false;
				for(std::pair<int,dfgNode*> pair : child->parentClassification){
					if(pair.second == node){
						xmlFile << "type=\"I2\"/>\n";
						found=true;
						break;
					}
				}

				if(!found){
					LLVM_DEBUG(dbgs() << "node = " << node->getIdx() <<"child getNameType= " << child->getNameType() << ", child = " << child->getIdx() << "\n");
				}

				assert(found);
			}
		}
		//		for(dfgNode* phiChild : node->getPHIchildren()){
		//			xmlFile << "\t<Output idx=\"" << phiChild->getIdx() << "\" type=\"PHI\"/>\n";
		//		}
		xmlFile << "</Outputs>\n";

		xmlFile << "<RecParents>\n";
		for(dfgNode* recParent : node->getRecAncestors()){
			xmlFile << "\t<RecParent idx=\"" << recParent->getIdx() << "\"/>\n";
		}
		xmlFile << "</RecParents>\n";

		xmlFile << "</Node>\n\n";

	}
	//		}
	//	}




	xmlFile << "</DFG>\n";





	xmlFile.close();




}








// #ifdef REMOVE_AGI
//DMD
/* Remove all address generation nodes in DFG -
First, identify getelementptr(GEP)) nodes. Go through the parent nodes of GEP nodes till the PHI node. Then remove all the nodes between
GEP and PHI node.
Afterwards, identify Branchin instruction(BRI) nodes. Go through the parent nodes of BRI nodes till the PHI node. Then remove all the nodes between
BRI and PHI node.
Finally, remove the GEP nodes from its childrens parent list.

Use tempNodeSet to store parents of GEP/BRI instructions. Add tempNodeSet to removalNodesSet when PHI is found in parents of GEP/BRI
 */


//Following functions are needed.
void DFGPartPredLight::removeAGI(){
	std::set<dfgNode*> removalNodes;
	std::set<dfgNode*> allNodes;
	std::set<dfgNode*> tempNodesSet;
	std::set<dfgNode*> non_agi_NodesSet;

    std::cout << "hello, I am removing \n";
	// Create set of non AGI nodes
	for(dfgNode* node : NodeList){
		LLVM_DEBUG(dbgs() << "Node Name Type/ ID:" << node->getNameType() << "/" << node->getIdx() << "\n");
		allNodes.insert(node);
		if(node->getNode()){

			if(GetElementPtrInst* GEP = dyn_cast<GetElementPtrInst>(node->getNode())){
				LLVM_DEBUG(dbgs() << "removeAGI: found GEP instruction and goes through its child instructions ---------\n");

				for(dfgNode* child : node->getChildren()){
					if(non_agi_NodesSet.find(child) == non_agi_NodesSet.end()){//to avoid looping
						non_agi_NodesSet.insert(child);
						if(child->getNameType() == "OutLoopLOAD" || child->getNameType() == "CMERGE"|| child->getNameType() == "LOOPSTART"|| child->getNameType() != ""){
							LLVM_DEBUG(dbgs() << "removeAGI: has Name Type---"<< child->getNameType() << child->getIdx()<<" ---------\n");
							addChildsToNonAGINodes(child,non_agi_NodesSet,tempNodesSet);
						}
						if(StoreInst* strinst = dyn_cast<StoreInst>(child->getNode())){

						}else{
							addChildsToNonAGINodes(child,non_agi_NodesSet,tempNodesSet);
						}
					}
				}

			}

		}
	}

	for(dfgNode* nagi : non_agi_NodesSet){
		LLVM_DEBUG(dbgs() << "removeAGI: Non AGI Nodes = " << nagi->getIdx() << "\n");
	}

	//remove AGI

	for(dfgNode* node : NodeList){
		LLVM_DEBUG(dbgs() << "Node Name Type/ ID:" << node->getNameType() << "/" << node->getIdx() << "\n");
		if(node->getNode()){

			if(GetElementPtrInst* GEP = dyn_cast<GetElementPtrInst>(node->getNode())){
				LLVM_DEBUG(dbgs() << "removeAGI: found GEP instruction and goes through its parent instructions ---------\n");
				for(dfgNode* child : node->getChildren()){
					node->removeChild(child);
					child->removeAncestor(node);
				}
				removalNodes.insert(node);
				addParentsToRemovalNodes(node,removalNodes,non_agi_NodesSet);
			}


		}
	}

	//------------ Some AGI nodes will be missed by the above traversal
	//------------ Following code segment safely remove the connection to those nodes and add the to removal nodes set
	std::set<dfgNode*> agi_NodesSet;
	std::set_difference(allNodes.begin(), allNodes.end(), non_agi_NodesSet.begin(), non_agi_NodesSet.end(),
			std::inserter(agi_NodesSet, agi_NodesSet.end()));


	std::set<dfgNode*> agi_rem_diff_NodesSet;
	std::set_difference(agi_NodesSet.begin(), agi_NodesSet.end(), removalNodes.begin(), removalNodes.end(),
			std::inserter(agi_rem_diff_NodesSet, agi_rem_diff_NodesSet.end()));

	removalNodes.insert(agi_rem_diff_NodesSet.begin(),agi_rem_diff_NodesSet.end());

	for(dfgNode* ard : agi_rem_diff_NodesSet){
		LLVM_DEBUG(dbgs() << "DFGAGIRemove: Difference between removal Nodes and AGI nodes = " << ard->getIdx() << "\n");
	}

	for(dfgNode* node : NodeList){
		for(dfgNode* ard : agi_rem_diff_NodesSet){
			if(node->getIdx()==ard->getIdx()){
				for(dfgNode* parent : node->getAncestors()){
					parent->removeChild(node);
				}
				for(dfgNode* child : node->getChildren()){
					child->removeAncestor(node);
				}
			}
		}

	}

	//------------------------------------

	for(dfgNode* rn : removalNodes){
		LLVM_DEBUG(dbgs() << "removeAGI: Removing Node = " << rn->getIdx() << "\n");
		NodeList.erase(std::remove(NodeList.begin(), NodeList.end(), rn), NodeList.end());
	}
	LLVM_DEBUG(dbgs() << "Nodelist size--"<< NodeList.size());

}

void DFGPartPredLight::addChildsToNonAGINodes(dfgNode* node, std::set<dfgNode*> &non_agi_NodesSet, std::set<dfgNode*> &tempNodesSet){

	for(dfgNode* child : node->getChildren()){
		if(non_agi_NodesSet.find(child) == non_agi_NodesSet.end()){//to avoid loops
			non_agi_NodesSet.insert(child);
			if(child->getNameType() == "OutLoopLOAD" || child->getNameType() == "CMERGE"|| child->getNameType() == "LOOPSTART" || child->getNameType() != ""){
				LLVM_DEBUG(dbgs() << "DFGAGIRemove: removeAGI: has Name Type---"<< child->getNameType()<< child->getIdx() <<" ---------\n");
				addChildsToNonAGINodes(child,non_agi_NodesSet,tempNodesSet);
			}
			else if(StoreInst* strinst = dyn_cast<StoreInst>(child->getNode())){

			}else{
				addChildsToNonAGINodes(child,non_agi_NodesSet,tempNodesSet);
			}
		}
	}

	if(!node->getAncestors().empty()){
		for(dfgNode* parent : node->getAncestors()){
			if(non_agi_NodesSet.find(parent) == non_agi_NodesSet.end()){//parent is not in the non AGI set
				if(parent->getNameType() !=""){
					non_agi_NodesSet.insert(parent);
					addParentsToNonAGINodes(parent,non_agi_NodesSet,tempNodesSet);
				}
				else if(LoadInst* loadinst = dyn_cast<LoadInst>(parent->getNode())){

				}
				else if(GetElementPtrInst* GEP = dyn_cast<GetElementPtrInst>(parent->getNode())){

				}
				else{
					non_agi_NodesSet.insert(parent);
					addParentsToNonAGINodes(parent,non_agi_NodesSet,tempNodesSet);
				}
			}
		}
	}

}

void DFGPartPredLight::addParentsToNonAGINodes(dfgNode* node, std::set<dfgNode*> &non_agi_NodesSet, std::set<dfgNode*> &tempNodesSet){

	if(!node->getAncestors().empty()){
		for(dfgNode* parent : node->getAncestors()){
			if(non_agi_NodesSet.find(parent) == non_agi_NodesSet.end()){//parent is not in the non AGI set
				if(parent->getNameType() !=""){
					non_agi_NodesSet.insert(parent);
					addParentsToNonAGINodes(parent,non_agi_NodesSet,tempNodesSet);
				}
				else if(LoadInst* loadinst = dyn_cast<LoadInst>(parent->getNode())){

				}
				else if(GetElementPtrInst* GEP = dyn_cast<GetElementPtrInst>(parent->getNode())){

				}
				else{
					non_agi_NodesSet.insert(parent);
					addParentsToNonAGINodes(parent,non_agi_NodesSet,tempNodesSet);
				}
			}
		}
	}

}

void DFGPartPredLight::addParentsToRemovalNodes(dfgNode* node, std::set<dfgNode*> &removalNodes, std::set<dfgNode*> &non_agi_NodesSet){
	/*if(node->getNameType() == "OutLoopLOAD" || node->getNameType() == "CMERGE"|| node->getNameType() == "LOOPSTART"  ){

        }
        else if(PHINode* PHI = dyn_cast<PHINode>(node->getNode())){

			LLVM_DEBUG(dbgs() << "DFGAGIRemove: PHI node found in parent instructions. Adding all nodes between PHI and GEP/BRI to removalNodes------ \n";
            removalNodes.insert(tempNodesSet.begin(),tempNodesSet.end());
            LLVM_DEBUG(dbgs() << "DFGAGIRemove: Done adding\n";
            tempNodesSet.clear();
		}*/




	if(!node->getAncestors().empty()){
		for(dfgNode* parent : node->getAncestors()){
			if(removalNodes.find(parent) == removalNodes.end()){//if the node is not in the removal node set - to break the loops with backedges
				if(non_agi_NodesSet.find(parent) == non_agi_NodesSet.end()){//if parent is not in the non AGI set=parent is AGI
					removalNodes.insert(parent);
					addParentsToRemovalNodes(parent,removalNodes,non_agi_NodesSet);
				}
				else{
					parent->removeChild(node);
				}
			}
		}

	}else{
		LLVM_DEBUG(dbgs() << "DFGAGIRemove: removeAGI: No parents------"<< node->getIdx()<< "\n");
	}


}
// #endif
