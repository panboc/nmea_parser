/**
 * @file nmea_parser.c
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
#include "string.h"

typedef enum fsm_err{
    FSM_EOK = 0,
    FSM_ERROR,
    FSM_EINVAL,
}fsm_err_t;

enum {
    FSM_STATE_INIT = 0,
    FSM_STATE_ABORT,
    FSM_STATE_COMPLETE,
    FSM_STATE_NAME_RECV,
    FSM_STATE_ARGS_RECV,
    FSM_STATE_CSUM_RECV,
    FSM_STATE_CRLF_RECV,
    FSM_STATE_MAX,
};

typedef enum {
    FSM_EVENT_DOLLAR_CHAR = 0,
    FSM_EVENT_ORDINARY_CHAR,
    FSM_EVENT_COMMA_CHAR,
    FSM_EVENT_ASTERISK_CHAR,
    FSM_EVENT_CR_CHAR,
    FSM_EVENT_LF_CHAR,
    FSM_EVENT_NUL_CHAR,
    FSM_EVENT_MAX,
}fsm_event_t;

enum {
    FSM_ERROR_DOLLAR_REPEAT = 1,
    FSM_ERROR_ORDINARY_EARLY,
    FSM_ERROR_ORDINARY_REPEAT,
    FSM_ERROR_COMMA_EARLY,
    FSM_ERROR_COMMA_REPEAT,
    FSM_ERROR_ASTERISK_EARLY,
    FSM_ERROR_ASTERISK_REPEAT,
    FSM_ERROR_CR_EARLY,
    FSM_ERROR_CR_REPEAT,
    FSM_ERROR_LF_EARLY,
    FSM_ERROR_LF_REPEAT,
    FSM_ERROR_NUL_EARLY,
};

typedef struct fsm fsm_t;

typedef struct fsm_state
{
    fsm_err_t (*event_handler[FSM_EVENT_MAX])(fsm_t *fsm);
    void *data;
}fsm_state_t;

struct fsm
{
    fsm_state_t state;
    nmea_token_t *token_ptr;
    char *string_ptr;
    unsigned char wait_nul;
    unsigned char name_len;
    unsigned char csum_len;
    unsigned char arg_sn;
    unsigned char error_code;
    unsigned int str_offset;
}global_fsm_obj;

fsm_err_t fsm_state_set(fsm_t *fsm, fsm_state_t *state);
fsm_state_t *fsm_state_get(fsm_t *fsm);

static fsm_err_t fsm_do_nothing(fsm_t *fsm);
static fsm_err_t fsm_abort(fsm_t *fsm);
static fsm_err_t fsm_init_recv_dollar(fsm_t *fsm);
static fsm_err_t fsm_name_recv_comma(fsm_t *fsm);
static fsm_err_t fsm_args_recv_comma(fsm_t *fsm);
static fsm_err_t fsm_args_recv_asterisk(fsm_t *fsm);
static fsm_err_t fsm_csum_recv_cr(fsm_t *fsm);
static fsm_err_t fsm_crlf_recv_lf(fsm_t *fsm);
static fsm_err_t fsm_crlf_recv_nul(fsm_t *fsm);

static fsm_state_t state_list[FSM_STATE_MAX] = {
    /* dollar               ordinary        comma                   asterisk                CR                  LF                  NUL                 data */
    {{fsm_init_recv_dollar, fsm_abort,      fsm_abort,              fsm_abort,              fsm_abort,          fsm_abort,          fsm_abort},         "INIT"},
    {{fsm_do_nothing,       fsm_do_nothing, fsm_do_nothing,         fsm_do_nothing,         fsm_do_nothing,     fsm_do_nothing,     fsm_do_nothing},    "ABORT"},
    {{fsm_do_nothing,       fsm_do_nothing, fsm_do_nothing,         fsm_do_nothing,         fsm_do_nothing,     fsm_do_nothing,     fsm_do_nothing},    "COMPLETE"},
    {{fsm_abort,            fsm_do_nothing, fsm_name_recv_comma,    fsm_abort,              fsm_abort,          fsm_abort,          fsm_abort},         "NAME_RECV"},
    {{fsm_abort,            fsm_do_nothing, fsm_args_recv_comma,    fsm_args_recv_asterisk, fsm_abort,          fsm_abort,          fsm_abort},         "ARGS_RECV"},
    {{fsm_abort,            fsm_do_nothing, fsm_abort,              fsm_abort,              fsm_csum_recv_cr,   fsm_abort,          fsm_abort},         "CSUM_RECV"},
    {{fsm_abort,            fsm_abort,      fsm_abort,              fsm_abort,              fsm_abort,          fsm_crlf_recv_lf,   fsm_crlf_recv_nul}, "CRLF_RECV"},
};

static fsm_err_t fsm_do_nothing(fsm_t *fsm)
{
    return FSM_EOK;
}

static fsm_err_t fsm_abort(fsm_t *fsm)
{
    fsm->state = state_list[FSM_STATE_ABORT];
    return FSM_EOK;
}

static fsm_err_t fsm_init_recv_dollar(fsm_t *fsm)
{
    fsm->token_ptr->name = &fsm->string_ptr[fsm->str_offset + 1];
    fsm->state = state_list[FSM_STATE_NAME_RECV];
    return FSM_EOK;
}

static fsm_err_t fsm_name_recv_comma(fsm_t *fsm)
{
    fsm->token_ptr->args[fsm->arg_sn++] = &fsm->string_ptr[fsm->str_offset + 1];
    fsm->string_ptr[fsm->str_offset] = '\0';
    fsm->state = state_list[FSM_STATE_ARGS_RECV];
    return FSM_EOK;
}

static fsm_err_t fsm_args_recv_comma(fsm_t *fsm)
{
    if(fsm->arg_sn >= NMEA_ARGS_MAX){
        fsm->state = state_list[FSM_STATE_ABORT];
        return FSM_EOK;
    }
    fsm->token_ptr->args[fsm->arg_sn++] = &fsm->string_ptr[fsm->str_offset + 1];
    fsm->string_ptr[fsm->str_offset] = '\0';
    return FSM_EOK;
}

static fsm_err_t fsm_args_recv_asterisk(fsm_t *fsm)
{
    fsm->token_ptr->args_num = fsm->arg_sn;
    fsm->token_ptr->csum = &fsm->string_ptr[fsm->str_offset + 1];
    fsm->string_ptr[fsm->str_offset] = '\0';
    fsm->state = state_list[FSM_STATE_CSUM_RECV];
    return FSM_EOK;
}

static fsm_err_t fsm_csum_recv_cr(fsm_t *fsm)
{
    fsm->string_ptr[fsm->str_offset] = '\0';
    fsm->state = state_list[FSM_STATE_CRLF_RECV];
    return FSM_EOK;
}

static fsm_err_t fsm_crlf_recv_lf(fsm_t *fsm)
{
    if(0 == fsm->wait_nul){
        fsm->wait_nul = 1;
    }
    else{
        fsm->state = state_list[FSM_STATE_ABORT];
    }
    return FSM_EOK;
}

static fsm_err_t fsm_crlf_recv_nul(fsm_t *fsm)
{
    if(1 == fsm->wait_nul){
        fsm->state = state_list[FSM_STATE_COMPLETE];
    }
    else{
        fsm->state = state_list[FSM_STATE_ABORT];
    }
    return FSM_EOK;
}

/**
 * @brief Parse NMEA sentence from string to token. 
 *        WARNING: The string will be destroyed, and the life of token is same as string.
 * @param string    NMEA sentence. 
 * @param token     NMEA token.
 * @return int      The result of parsing.
 *                  0   - Success
 *                  !0  - Fail
 */
int nmea_parse(char *string, nmea_token_t *token)
{
    if((NULL == string) || (NULL == token)){
        return -2;
    }
    memset(&global_fsm_obj, 0, sizeof(global_fsm_obj));
    global_fsm_obj.token_ptr = token;
    global_fsm_obj.string_ptr = string;
    global_fsm_obj.state = state_list[FSM_STATE_INIT];
    char ch = string[global_fsm_obj.str_offset];
    fsm_event_t event = FSM_EVENT_MAX;
    while(ch){
        switch(ch){
        case '$':
            event = FSM_EVENT_DOLLAR_CHAR;
            break;
        case ',':
            event = FSM_EVENT_COMMA_CHAR;
            break;
        case '*':
            event = FSM_EVENT_ASTERISK_CHAR;
            break;
        case '\r':
            event = FSM_EVENT_CR_CHAR;
            break;
        case '\n':
            event = FSM_EVENT_LF_CHAR;
            break;
        default:
            event = FSM_EVENT_ORDINARY_CHAR;
            break;
        }
        global_fsm_obj.state.event_handler[event](&global_fsm_obj);
        if(!strcmp(state_list[FSM_STATE_ABORT].data, global_fsm_obj.state.data)){
            return -1;
        }
        ch = string[++global_fsm_obj.str_offset];
    }
    global_fsm_obj.state.event_handler[FSM_EVENT_NUL_CHAR](&global_fsm_obj);
    if(!strcmp(state_list[FSM_STATE_COMPLETE].data, global_fsm_obj.state.data)){
        global_fsm_obj.token_ptr->str_len = global_fsm_obj.str_offset;
        return 0;
    }

    return -1;
}
