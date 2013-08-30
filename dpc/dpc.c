#include <string.h>
#include <unistd.h>
#include "mjopt.h"
#include "mjlf.h"
#include "mjproto_txt.h"
#include "offline_store.h"
#include "online_store.h"


#define DPC_MODE_CACHE "online"
#define DPC_MODE_STORE "offline"
#define MAX_CONFIG_FILE_LENGTH 128


struct dpc_conf {
  int port;
  int thread;
  int readtimeout;
  int writetimeout;
  char mode[8];
};

struct mjproto_txt_routine_list online_routine_list[] = {
  {"get", online_get},
  {"put", online_put},
  {"del", online_del},
  {"create", online_create},
  {"drop", online_drop},
  {"quit", online_quit},
  {NULL, NULL},
};

struct mjproto_txt_routine_list offline_routine_list[] = {
  {"get", offline_get},
  {"put", offline_put},
  {"del", offline_del},
  {"create", offline_create},
  {"drop", offline_drop},
  {"quit", offline_quit},
  {NULL, NULL},
};

static void dpc_option(int argc, char **argv, char *value) {
  char ch;
  while ((ch = getopt(argc, argv, "c:")) != EOF) {
    switch(ch) {
      case 'c':
        value = strncpy(value, optarg, MAX_CONFIG_FILE_LENGTH-1);
        break;
      }
  }
}

/*
 * mode: -m / mode online for cache/offline for store default offline
 * port: -p / port default 17879
 * thread number: -t / thread default 4
 * read timeout: -r / readtimeout default 1000[ms]
 * write timeout: -w / writetimeout default 1000[ms]
 */
int main(int argc, char **argv) {
  struct dpc_conf conf = {17879, 4, 1000, 1000, DPC_MODE_STORE};
  if (argc >= 3) {
    char config_file[MAX_CONFIG_FILE_LENGTH];
    dpc_option(argc, argv, config_file);
    if (mjopt_parse_conf(config_file)) {
      mjopt_get_value_int(NULL, "port", &conf.port);
      mjopt_get_value_int(NULL, "thread", &conf.thread);
      mjopt_get_value_int(NULL, "readtimeout", &conf.readtimeout);
      mjopt_get_value_int(NULL, "writetimeout", &conf.writetimeout);
      mjopt_get_value_string(NULL, "mode", &conf.mode);
    }
  }

  int sfd = mjsock_tcp_server(conf.port);
  if (sfd < 0) {
    printf("mjsock_tcp_server error\n");
    return 1;
  }

  mjlf server;
  if (strncasecmp(conf.mode, DPC_MODE_CACHE, 8)) {
    server = mjlf_new(sfd, mjproto_txt_routine, conf.thread, mjproto_txt_init,
          online_routine_list, online_thread_init, NULL);
  } else if (strncasecmp(conf.mode, DPC_MODE_STORE, 8)) {
    server = mjlf_new(sfd, mjproto_txt_routine, conf.thread, mjproto_txt_init,
          offline_routine_list, offline_thread_init, NULL);
  } else {
    printf("mode %s not support\n", conf.mode);
    return 1;
  }

  if (!server) {
    printf("mjlf_New error");
    return 1;
  }

  mjlf_set_timeout(server, conf.readtimeout, conf.writetimeout);
  mjlf_run(server);
  mjlf_delete(server);
  return 0;
}

