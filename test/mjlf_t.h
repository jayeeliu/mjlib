#ifndef MJLF_T
#define MJLF_T 1

#define MJLF_MAX_SQL_LENGTH 512
#define MJLF_MAX_TABLE_NAME_LENGTH 32
#define MJLF_MAX_KEY_LENGTH 64
#define MJLF_MAX_VALUE_LENGTH 65536

#define MJLF_VALID_SUCCESS 0
#define MJLF_VALID_FAIL -1

#define ERR_COMMEND_FORMAT_ERROR 50001
#define ERR_TABLE_NAME_LENGTH -50011
#define ERR_TABLE_NAME_ILLEGAL_CHAR -500012
#define ERR_KEY_LENGTH -50021
#define ERR_KEY_ILLEGAL_CHAR -50022
#define ERR_VALUE_LENGTH -50032

#define ERR_SQL_QUERY_FAILE -51001
#define ERR_SYSTEM_ERROR -52001


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
