#include "mjvec.h"
#include <stdlib.h>

/*
===============================================================================
mjvec_ready
  mjvec ready need_size
===============================================================================
*/
static bool mjvec_ready(mjvec vec, unsigned int need_size) {
  // vec is enough
  unsigned int total = vec->_total;
  if (need_size <= total) return true;
  // realloc space
  vec->_total = 30 + need_size + (need_size >> 3);
  void** new_data = (void**) realloc(vec->_data, vec->_total * sizeof(void*));
  if (!new_data) {
    vec->_total = total;
    return false;
  }
  // copy new_ndata
  vec->_data = new_data;
  for (int i = vec->length; i < vec->_total; i++) {
    vec->_data[i] = NULL; 
  }
  return true;
}

/*
===============================================================================
mjvec_readplus
===============================================================================
*/
static bool mjvec_readyplus(mjvec vec, unsigned int n) {
  return mjvec_ready(vec, vec->length + n);
}

bool mjvec_add(mjvec vec, void* value) {
  if (!vec || !value) return false;
  if (!mjvec_readyplus(vec, 1)) return false;
  vec->_data[vec->length] = value;
  vec->length++;
  return true;
}

void* mjvec_get(mjvec vec, unsigned int idx) {
  if (!vec || idx >= vec->length) return NULL;
  return vec->_data[idx];
}

mjvec mjvec_new(mjProc data_free) {
  mjvec vec = (mjvec) calloc(1, sizeof(struct mjvec));
  if (!vec) return NULL;
  vec->_data_free = data_free;
  return vec;
}

bool mjvec_delete(mjvec vec) {
  if (!vec) return false;
  if (vec->_data_free) {
    for (int i = 0; i < vec->length; i++) {
      if (vec->_data[i]) vec->_data_free(vec->_data[i]);
    }
  }
  free(vec->_data);
  free(vec);
  return true;
}
