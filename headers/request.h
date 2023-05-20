#ifndef REQUEST_H_
#define REQUEST_H_

typedef enum {
    REQ_INSCRIPTION = 1,
    REQ_POST_BILLET,
    REQ_GET_BILLET,
    REQ_SUBSCRIBE,
    REQ_ADD_FILE,
    REQ_DW_FILE,

    REQ_SERVER_ERROR = 31,
} codereq_t;

#endif
