#ifndef OFFLINE_STORE_H
#define OFFLINE_STORE_H 1

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "validator.h"
#include "offline_store.h"
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


#endif
