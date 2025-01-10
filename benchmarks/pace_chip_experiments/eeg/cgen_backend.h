#include <stdlib.h>
#include <stdio.h>

void* TVMBackendAllocWorkspace(int device_type, int device_id, uint64_t nbytes,
                                       int dtype_code_hint, int dtype_bits_hint) {
  /* if (device_type != 1) { */
  /*   printf("Error: device_type must be 1 (CPU)\n"); */
  /*   exit(-1); */
  /* } */
  /* void* ptr = malloc(nbytes); */
  /* int8_t x = ((int8_t*)ptr)[0]; */
  /* return ptr; */
  //assert(false && "TVMBackendAllocWorkspace not implemented");
  return NULL;
}

int TVMBackendFreeWorkspace(int device_type, int device_id, void* ptr) {
  return 0;
  /* if (device_type != 1) { */
  /*   printf("Error: device_type must be 1 (CPU)\n"); */
  /*   exit(-1); */
  /* } */

  /* free(ptr); */
  /* return 0; */
}

float roundf(float x) {
  int integerPart = (int)x;
  float fractionalPart = x - integerPart;
  if (fractionalPart >= 0.5)
    return integerPart + 1;
  else if (fractionalPart < -0.5)
    return integerPart - 1;
  else
    return integerPart;
}


float expf(float x) {
  return x;
}
