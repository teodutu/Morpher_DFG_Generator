#ifndef DFG3DRA_H
#define DFG3DRA_H


#include <unordered_set>
#include <morpherdfggen/common/dfg.h>

#include "DFGPartPred.h"


//comment this in normal compilation
//#define REMOVE_AGI
//Uncomment this if compiling fo the pace0.5 architecture
//#define ARCHI_16BIT



class DFG3DRA : public DFGPartPred
{
public :
	DFG3DRA(std::string name,std::map<Loop*,std::string>* lnPtr, Loop* l): DFGPartPred(name,lnPtr, l) {}


};

#endif
