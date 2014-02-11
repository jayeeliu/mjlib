#include "sf_module.h"
#include <pthread.h>

static LIST_HEAD(task_queue);
static pthread_mutex_t task_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t task_queue_cond = PTHREAD_COND_INITIALIZER;

static unsigned worker_n;
static unsigned worker_sleep_n;

void 
sf_task_enqueu(sf_object_t *obj) {
  if (!list_empty(&obj->node)) return;
  pthread_mutex_lock(&task_queue_mutex);
  list_add_tail(&obj->node, &task_queue);
  pthread_mutex_unlock(&task_queue_mutex);
  if (worker_sleep_n > 0) pthread_cond_signal(&task_queue_cond);
}

static sf_object_t* 
sf_task_dequeue_lock() {
  if (list_empty(&task_queue)) return NULL;
  sf_object_t* obj = NULL; 
  obj = list_first_entry(&task_queue, sf_object_t, node);
  list_del(&obj->node);
  return obj;
}

static void*
worker_routine(void* arg) {
  worker_n++;

  sf_object_t* obj;
  while (1) {
    pthread_mutex_lock(&task_queue_mutex);
    obj = sf_task_dequeue_lock();
    while (!obj) {
      worker_sleep_n++;
      pthread_cond_wait(&task_queue_cond, &task_queue_mutex);
      worker_sleep_n--;
      obj = sf_task_dequeue_lock();
    }
    pthread_mutex_unlock(&task_queue_mutex);
    if (obj->handler) obj->handler(obj);
  }
  return NULL;
}

void
sf_create_worker() {
  pthread_t thread;
  pthread_create(&thread, NULL, worker_routine, NULL);
}
