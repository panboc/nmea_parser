/**
 * @file main.c
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

#include "nmea_parser.h"
#include "unity_fixture.h"

static void run_all_testcases(void)
{
    RUN_TEST_GROUP(nmea);
}

int main(int argc, char **argv)
{
    UnityMain(argc, (const char **)argv, run_all_testcases);
    return 0;
}
