#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>


#include "../../include/morpherdfggen/common/ArchPrecision.h"


#ifdef __cplusplus
extern "C" {
#endif



#ifdef ARCHI_16BIT
	const int bytes_per_variable = 2;
#else
	const int bytes_per_variable = 4;
#endif
  


void printArr(const char* name, uint8_t* arr, int size, uint8_t io, uint32_t addr);
void reportDynArrSize(const char* name, uint8_t* arr, uint32_t idx_i, int size);
void printDynArrSize();
void clearPrintedArrs();
void loopEnd(const char* loopName);
void loopStart(const char* loopName);
void outloopValueReport(uint32_t nodeIdx, uint32_t value, uint32_t addr, uint8_t isLoad, uint8_t isHostTrans, uint8_t size);

void loopTraceOpen(const char* fnName);
void loopTraceClose();
void loopInvoke(const char* loopName);
void loopInvokeEnd(const char* loopName);
void loopInsUpdate(const char* name, int insCount);

void loopBBInsUpdate(const char* loopName, const char* BBName, int insCount);
void loopBBMappingUnitUpdate(const char* BBName, const char* munitName);

void loopInsClear(const char* name);
void loopBBInsClear();

void updateLoopPreHeader(const char* loopName, const char* preheaderBB);

void reportExecInsCount(int count);
void recordUncondMunitTransition(const char* srcBB, const char* destBB);
void recordCondMunitTransition(const char* srcBB, const char* destBB1, const char* destBB2, int condition);

//2018 work of profiling NNVM compiled caffe code
void reportBBTrace(const char* FName, const char* BBName, int insCount);
void sortandPrintStats();


//2018 work of triggered CGRA execution
void reportNewBBinPath(const char* bbName, const char* loopName);
void reportLoopEnd(const char* loopName);

void LiveInReport (const char* varname, uint8_t* value, uint32_t size);
void LiveInReport2(const char* varname, uint32_t* value, uint32_t size);
void LiveOutReport(const char* varname, uint8_t* value, uint32_t size);
void LiveInReportIntermediateVar(const char* varname, uint32_t value);
void LiveOutReportIntermediateVar(const char* varname, uint32_t value);
void LiveInReportPtrTypeUsage(const char* varname,const char* varbaseaddr, uint32_t value, uint32_t size);
void please_map_me();
uint8_t* trunkTo16Bit(uint8_t* value, uint32_t size);


#ifdef __cplusplus
}
#endif
