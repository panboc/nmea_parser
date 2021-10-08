/**
 * @file nmea_parser.h
 * 
 * @brief 
 * 
 * @copyright Copyright (c) 2021 Panboc All Rights Reserved.
 * 
 * ============================== EDIT HISTORY FOR MODULE ==============================
 * This section contains comments describing the changes made to the module.
 * Notice that the latest change comment should be at the bottom.
 * WHEN         WHO                 WHAT,WHERE,WHY
 * ----------   -----------------   ----------------------------------------------------
 * 2021-09-24   Remy SHI            First commit.
 */

#ifndef NMEA_PARSER_H
#define NMEA_PARSER_H

#define NMEA_ARGS_MAX (30)

typedef struct {
    char *args[NMEA_ARGS_MAX];
    char *name;
    char *csum;
    unsigned char args_num;
    unsigned int str_len;
}nmea_token_t;

int nmea_parse(char *string, nmea_token_t *token);

#endif /* NMEA_PARSER_H */
