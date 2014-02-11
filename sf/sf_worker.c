#include "sf_worker.h"
#include "sf_module.h"
#include <pthread.h>

static LIST_HEAD(task_queue);
static pthread_mutex_t task_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t task_queue_cond = PTHREAD_COND_INITIALIZER;

static unsigned worker_n;
static unsigned worker_sleep_n;

void 
sf_worker_enqueue(sf_object_t *obj) {
  if (!list_empty(&obj->node)) return;
  pthread_mutex_lock(&task_queue_mutex);
  list_add_tail(&obj->node, &task_queue);
  pthread_mutex_unlock(&task_queue_mutex);
  pthread_cond_broadcast(&task_queue_cond);
}

static void*
worker_routine(void* arg) {
  worker_n++;

  sf_object_t* obj;
  while (1) {
    pthread_mutex_lock(&task_queue_mutex);
    while (list_empty(&task_queue)) {
      worker_sleep_n++;
      pthread_cond_wait(&task_queue_cond, &task_queue_mutex);
      worker_sleep_n--;
    }
    obj = list_first_entry(&task_queue, sf_object_t, node);
    list_del_init(&obj->node);
    pthread_mutex_unlock(&task_queue_mutex);

    if (obj->handler) obj->handler(obj);
  }
  worker_n--;
  return NULL;
}

static void
create_worker() {
  pthread_t thread;
  pthread_create(&thread, NULL, worker_routine, NULL);
}

static void 
worker_start() {
  create_worker();
  create_worker();
  create_worker();
  create_worker();
}

sf_module_t worker_module = {
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
