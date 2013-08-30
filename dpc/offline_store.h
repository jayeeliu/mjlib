#ifndef OFFLINE_STORE_H
#define OFFLINE_STORE_H 1

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "validator.h"
#include "mjlf.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjcomm.h"
#include "mjproto_txt.h"
#include "mjsql.h"


#define MJ_GET(sql, table, key) sprintf(sql, \
    "SELECT `value` FROM `%s` WHERE `key`='%s'", table, key)
#define MJ_SET(sql, table, key, value) sprintf(sql, \
    "INSERT INTO `%s` \
    (`key`, `value`) VALUES ('%s', '%s') \
    ON DUPLICATE KEY UPDATE `value`='%s'", \
    table, key, value, value)
#define MJ_DEL(sql, table, key) sprintf(sql, \
    "DELETE FROM `%s` WHERE `key`='%s'", table, key)
#define MJ_CREATE(sql, table) sprintf(sql, \
    "CREATE TABLE `%s` (\
      `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,\
      `key` VARCHAR(64) NULL,\
      `value` TEXT NULL,\
      `ts` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
      PRIMARY KEY (`id`),\
      UNIQUE INDEX `key_UNIQUE` (`key` ASC)\
    ) ENGINE = InnoDB DEFAULT CHARACTER SET = latin1", table)
#define MJ_DROP(sql, table) sprintf(sql, "DROP TABLE `%s`", table)


extern void* offline_get(void* arg);
extern void* offline_put(void* arg);
extern void* offline_del(void* arg);
extern void* offline_quit(void* arg);
extern void* offline_create(void* arg);
extern void* offline_drop(void* arg);
extern void* offline_thread_init(void* arg);

#endif
