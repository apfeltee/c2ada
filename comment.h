#ifndef _H_COMMENT_
#define _H_COMMENT_

#define COMMENT_BLOCKSIZE 10

struct comment_block {
    struct comment_block *      next;
    int                         count;
    char *                      line[COMMENT_BLOCKSIZE];
};

typedef struct comment_block * comment_block_pt;

extern void add_comment_line( comment_block_pt block, char * line );

extern comment_block_pt new_comment_block( void );

extern void put_comment_block( comment_block_pt block, int indent );

#endif
