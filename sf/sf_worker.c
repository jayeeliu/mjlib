#include "sf_worker.h"
#include "sf_module.h"
#include "sf_util.h"
#include <pthread.h>

#define MAX_WORKER  32

static LIST_HEAD(worker_queue);
static pthread_mutex_t worker_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t worker_queue_cond = PTHREAD_COND_INITIALIZER;

static unsigned worker_n;
static unsigned worker_sleep_n;

/*
===============================================================================
sf_worker_do
===============================================================================
*/
void 
sf_worker_do(sf_object_t *obj) {
  if (!obj || !list_empty(&obj->worker_node)) return;
  pthread_mutex_lock(&worker_queue_mutex);
  list_add_tail(&obj->worker_node, &worker_queue);
  pthread_mutex_unlock(&worker_queue_mutex);
  pthread_cond_broadcast(&worker_queue_cond);
}

/*
===============================================================================
worker_routine
===============================================================================
*/
static void*
worker_routine(void* arg) {
  worker_n++;

  sf_object_t* obj;
  while (1) {
    pthread_mutex_lock(&worker_queue_mutex);
    while (list_empty(&worker_queue)) {
      worker_sleep_n++;
      pthread_cond_wait(&worker_queue_cond, &worker_queue_mutex);
      worker_sleep_n--;
    }
    obj = list_first_entry(&worker_queue, sf_object_t, worker_node);
    list_del_init(&obj->worker_node);
    pthread_mutex_unlock(&worker_queue_mutex);

    if (obj->handler) obj->handler(obj);
  }
  worker_n--;
  return NULL;
}

/*
===============================================================================
create_worker
===============================================================================
*/
static void
create_worker() {
  pthread_t thread;
  pthread_create(&thread, NULL, worker_routine, NULL);
}

/*
===============================================================================
worker_start
===============================================================================
*/
static void 
worker_start() {
  int num = get_cpu_count(); 
  if (num > MAX_WORKER) num = MAX_WORKER; 
  for (int i=0; i<num; i++) {
    create_worker();
  }
}

static sf_module_t worker_module = {
  NULL,
  NULL,
  worker_start,
  NULL,
};

__attribute__((constructor))
static void
__worker_init(void) {
  sf_register_module(&worker_module);
}
