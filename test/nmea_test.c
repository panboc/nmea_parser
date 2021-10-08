/**
 * @file nmea_test.c
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
 * 2021-09-30   Remy SHI            First commit.
 */

#include "unity_fixture.h"
#include <string.h>
#include "nmea_parser.h"

static char nmea_buff[200] = {0};
static nmea_token_t nmea_token = {0};

#define NORMAL_STR1     "$GPRMC,030004.00,A,3149.300037,N,11706.980654,E,0.0,,060821,4.3,W,A,V*71\r\n"
#define NORMAL_STR2     "$GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n"
#define ABNORMAL_STR1_1 "GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n"
#define ABNORMAL_STR1_2 ",GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n"
#define ABNORMAL_STR1_3 "*GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n"
#define ABNORMAL_STR1_4 "\rGNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n"
#define ABNORMAL_STR1_5 "\nGNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n"
#define ABNORMAL_STR1_6 ""
#define ABNORMAL_STR2_1 "$$GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n"
#define ABNORMAL_STR2_2 "$GN*GSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n"
#define ABNORMAL_STR2_3 "$GN\rGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n"
#define ABNORMAL_STR2_4 "$GN\nGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n"
#define ABNORMAL_STR2_5 "$GN"
#define ABNORMAL_STR3_1 "$GNGSA,A,3,71,72,87,,,,$,,,,,,0.9,0.5,0.7,2*35\r\n"
#define ABNORMAL_STR3_2 "$GNGSA,A,3,71,72,87,,,,\r,,,,,,0.9,0.5,0.7,2*35\r\n"
#define ABNORMAL_STR3_3 "$GNGSA,A,3,71,72,87,,,,\n,,,,,,0.9,0.5,0.7,2*35\r\n"
#define ABNORMAL_STR3_4 "$GNGSA,A,3,71,72,87,,,,"
#define ABNORMAL_STR4_1 "$GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*3$5\r\n"
#define ABNORMAL_STR4_2 "$GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*3,5\r\n"
#define ABNORMAL_STR4_3 "$GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*3*5\r\n"
#define ABNORMAL_STR4_4 "$GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*3$5\n"
#define ABNORMAL_STR4_5 "$GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*3$5"
#define ABNORMAL_STR5_1 "$GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n$"
#define ABNORMAL_STR5_2 "$GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\nA"
#define ABNORMAL_STR5_3 "$GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n,"
#define ABNORMAL_STR5_4 "$GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n*"
#define ABNORMAL_STR5_5 "$GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n\r"
#define ABNORMAL_STR5_6 "$GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35\r\n\n"
#define ABNORMAL_STR5_7 "$GNGSA,A,3,71,72,87,,,,,,,,,,0.9,0.5,0.7,2*35"

struct {
    char *string;
    int result;
    nmea_token_t token;
}parse_test_list[] = {
    {NORMAL_STR1, 0, {{"030004.00", "A", "3149.300037", "N", "11706.980654", "E", "0.0", "", "060821", "4.3", "W", "A", "V"}, .name = "GPRMC", .args_num = 13, .csum = "71", .str_len = strlen(NORMAL_STR1)}},
    {NORMAL_STR2, 0, {{"A", "3", "71", "72", "87", "", "", "", "", "", "", "", "", "", "0.9", "0.5", "0.7", "2"}, .name = "GNGSA", .args_num = 18, .csum = "35", .str_len = strlen(NORMAL_STR2)}},
    {ABNORMAL_STR1_1, -1},
    {ABNORMAL_STR1_2, -1},
    {ABNORMAL_STR1_3, -1},
    {ABNORMAL_STR1_4, -1},
    {ABNORMAL_STR1_5, -1},
    {ABNORMAL_STR1_6, -1},
    {ABNORMAL_STR2_1, -1},
    {ABNORMAL_STR2_2, -1},
    {ABNORMAL_STR2_3, -1},
    {ABNORMAL_STR2_4, -1},
    {ABNORMAL_STR2_5, -1},
    {ABNORMAL_STR3_1, -1},
    {ABNORMAL_STR3_2, -1},
    {ABNORMAL_STR3_3, -1},
    {ABNORMAL_STR3_4, -1},
    {ABNORMAL_STR4_1, -1},
    {ABNORMAL_STR4_2, -1},
    {ABNORMAL_STR4_3, -1},
    {ABNORMAL_STR4_4, -1},
    {ABNORMAL_STR4_5, -1},
    {ABNORMAL_STR5_1, -1},
    {ABNORMAL_STR5_2, -1},
    {ABNORMAL_STR5_3, -1},
    {ABNORMAL_STR5_4, -1},
    {ABNORMAL_STR5_5, -1},
    {ABNORMAL_STR5_6, -1},
    {ABNORMAL_STR5_7, -1},
};

TEST_GROUP(nmea);

TEST_SETUP(nmea)
{
    UnityMalloc_StartTest();
}

TEST_TEAR_DOWN(nmea)
{
    UnityMalloc_EndTest();
}


TEST(nmea, parse)
{
    int i = 0;
    for(i = 0; i < sizeof(parse_test_list) / sizeof(parse_test_list[0]); i++){
        memset(nmea_buff, 0, sizeof(nmea_buff));
        memcpy(nmea_buff, parse_test_list[i].string, strlen(parse_test_list[i].string) + 1);
        TEST_ASSERT_EQUAL_INT(parse_test_list[i].result, nmea_parse(nmea_buff, &nmea_token));
        if(0 == parse_test_list[i].result){
            TEST_ASSERT_EQUAL_STRING(parse_test_list[i].token.name, nmea_token.name);
            TEST_ASSERT_EQUAL_STRING(parse_test_list[i].token.csum, nmea_token.csum);
            TEST_ASSERT_EQUAL_STRING_ARRAY(parse_test_list[i].token.args, nmea_token.args, NMEA_ARGS_MAX);
            TEST_ASSERT_EQUAL_CHAR(parse_test_list[i].token.args_num, nmea_token.args_num);
            TEST_ASSERT_EQUAL_INT(parse_test_list[i].token.str_len, nmea_token.str_len);
        }
    }
}

TEST_GROUP_RUNNER(nmea)
{
    RUN_TEST_CASE(nmea, parse);
}
