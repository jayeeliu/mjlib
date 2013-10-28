#include "mjlocklist.h"
#include <stdlib.h>

/*
===============================================================================
mjlocklist_new
  alloc new lock list
===============================================================================
*/
mjlocklist mjlocklist_new(mjProc FREE) {
  mjlocklist llist = (mjlocklist)calloc(1, sizeof(struct mjlocklist));
  if (!llist) return NULL;
  pthread_mutex_init(&llist->_lock, NULL);
  INIT_LIST_HEAD(&llist->_list);
  llist->_FREE = FREE;
  return llist;
}

bool mjlocklist_delete(mjlocklist llist) {
  if (!llist) return false;
}

