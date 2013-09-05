#include <string.h>
#include <unistd.h>
#include "mjopt.h"
#include "mjlf.h"
#include "mjsock.h"
#include "mjproto_txt.h"
#include "offline_store.h"
#include "online_store.h"


#define DPC_MODE_CACHE "online"
#define DPC_MODE_STORE "offline"
#define MAX_CONFIG_FILE_LENGTH 128
#define CONFIG_SERVER_NAME "_server_"


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
  {"lpush", online_lpush},
  {"rpop", online_rpop},
  {"llen", online_llen},
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

static void usage(char **argv) {
  printf("Please Usage: %s -c path_to_config_file\n", argv[0]);
  printf("config file format: \n\tdpc/config/offline.ini for offline\n");
  printf("\tdpc/config/online.ini for online\n");
}

/*
 * mode: mode online for cache/offline for store default offline
 * port: port default 17879
 * thread number: thread default 4
 * read timeout: readtimeout default 1000[ms]
 * write timeout: writetimeout default 1000[ms]
 */
int main(int argc, char **argv) {
  struct dpc_conf conf = {17879, 4, 1000, 1000, DPC_MODE_STORE};
  if (argc >= 3) {
    char config_file[MAX_CONFIG_FILE_LENGTH];
    dpc_option(argc, argv, config_file);
    if (mjopt_parse_conf(config_file)) {
      mjopt_get_value_int(CONFIG_SERVER_NAME, "port", &conf.port);
      mjopt_get_value_int(CONFIG_SERVER_NAME, "thread", &conf.thread);
      mjopt_get_value_int(CONFIG_SERVER_NAME, "readtimeout", &conf.readtimeout);
      mjopt_get_value_int(CONFIG_SERVER_NAME, "writetimeout", &conf.writetimeout);
      mjopt_get_value_string(CONFIG_SERVER_NAME, "mode", conf.mode);
    }
  } else {
    usage(argv);
    return 1;
  }


  int sfd = mjsock_tcp_server(conf.port);
  if (sfd < 0) {
    printf("mjsock_tcp_server error\n");
    return 5;
  }

  mjlf server;
  if (strncasecmp(conf.mode, DPC_MODE_CACHE, 8) == 0) {
    server = mjlf_new(sfd, mjproto_txt_routine, conf.thread, mjproto_txt_init,
          online_routine_list, online_thread_init, NULL);
  } else if (strncasecmp(conf.mode, DPC_MODE_STORE, 8) == 0) {
    server = mjlf_new(sfd, mjproto_txt_routine, conf.thread, mjproto_txt_init,
          offline_routine_list, offline_thread_init, NULL);
  } else {
    printf("mode %s not support\n", conf.mode);
    return 10;
  }

  if (!server) {
    printf("mjlf new error");
    return 15;
  }

  mjlf_set_timeout(server, conf.readtimeout, conf.writetimeout);
  mjlf_run(server);
  mjlf_delete(server);
  return 0;
}

