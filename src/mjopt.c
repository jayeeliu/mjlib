#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include "mjopt.h"
#include "mjlog.h"
#include "mjio.h"

static LIST_HEAD(options);

/*
===============================================================================
mjopt_set_value
    set option define
===============================================================================
*/
static bool mjopt_set_value(char* section, char* key, char* value) {
  // sanity check
  if (section && strlen(section) >= MAX_SECTION_LEN) {
    MJLOG_ERR("section is too long");
    return false;
  }
  if (!key || strlen(key) >= MAX_KEY_LEN) {
    MJLOG_ERR("key is too long");
    return false;
  }
  if (!value || strlen(value) >= MAX_VALUE_LEN) {
    MJLOG_ERR("value is too long");
    return false;
  }
  // create mjopt struct
  mjopt opt = (mjopt) calloc (1, sizeof(struct mjopt));
  if (!opt) {
    MJLOG_ERR("mjopt alloc error");
    return false;
  }
  // set section and key 
  if (!section) {
    strncpy(opt->section, "global", MAX_SECTION_LEN);
  } else {
    strncpy(opt->section, section, MAX_SECTION_LEN);
  }
  strncpy(opt->key, key, MAX_KEY_LEN);
  strncpy(opt->value, value, MAX_VALUE_LEN);
  // add to options list
  INIT_LIST_HEAD(&opt->node);
  list_add_tail(&opt->node, &options);
  return true;
}

bool mjopt_get_value_int(char* section, char* key, int* retval) {
  if (!section) section = "global";
  mjopt entry = NULL;
  list_for_each_entry(entry, &options, node) {
    if (!strcmp(section, entry->section) && !strcmp(key, entry->key)) {
      *retval = atoi(entry->value);
      return true;
    }
  }
  return false;
}

bool mjopt_get_value_string(char* section, char* key, char* retval) {
  if (!section) section = "global";
  mjopt entry = NULL;
  list_for_each_entry(entry, &options, node) {
    if (!strcmp(section, entry->section) && !strcmp(key, entry->key)) {
      strncpy(retval, entry->value, MAX_VALUE_LEN);
      return true;
    }
  }
  return false;
}


/*
===============================================================================
mjopt_ParseConf
    parse conf file
===============================================================================
*/
bool mjopt_parse_conf(const char* fileName) {
  char section[MAX_SECTION_LEN];
  char key[MAX_KEY_LEN];
  char value[MAX_VALUE_LEN];

  mjIO io = mjIO_New(fileName);
  if (!io) {
    MJLOG_ERR("mjio alloc error");
    return false;
  }
  mjStr line = mjStr_New();
  if (!line) {
    MJLOG_ERR("mjStr_New error");
    mjIO_Delete(io);
    return false;
  }
  // set default section
  strcpy(section, "global");
  while (1) {
    // get one line from file
    if (mjIO_ReadLine(io, line) <= 0) break;
    mjStr_Strim(line);
    // ignore empty line
    if (line->length == 0) continue;
    // ignore comment line
    if (line->data[0] == '#') continue;
    // section line, get section
    if (line->data[0] == '[' && line->data[line->length-1] == ']') {
      mjStr_Consume(line, 1);
      mjStr_RConsume(line, 1);
      mjStr_Strim(line);
      // section can't be null
      if (line->length == 0) {
        MJLOG_ERR("section is null");
        mjIO_Delete(io);
        mjStr_Delete(line);
        return false;
      }
      strncpy(section, line->data, MAX_SECTION_LEN);
      continue;
    }
    // split key and value
    mjStrList strList = mjStrList_New();
    mjStr_Split(line, "=", strList);
    if (strList->length != 2) {
      MJLOG_ERR("conf error");
      mjStrList_Delete(strList);
      mjStr_Delete(line);
      mjIO_Delete(io);
      return false;
    }
    mjStr keyStr = mjStrList_Get(strList, 0);
    mjStr valueStr = mjStrList_Get(strList, 1);
    mjStr_Strim(keyStr);
    mjStr_Strim(valueStr);
    strncpy(key, keyStr->data, MAX_KEY_LEN);
    strncpy(value, valueStr->data, MAX_VALUE_LEN);
    mjStrList_Delete(strList);
    // set option value
    mjopt_set_value(section, key, value);
  }
  mjStr_Delete(line);
  mjIO_Delete(io);
  return true;
}
