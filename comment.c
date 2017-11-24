#include "il.h"
#include "allocate.h"
#include "comment.h"
#include "format.h"
#include <string.h>

comment_block_pt
new_comment_block()
{
    return (comment_block_pt) allocate( sizeof(struct comment_block) );
}

void
add_comment_line( comment_block_pt block, char * line )
{
    comment_block_pt prev;

    while (block && block->count == COMMENT_BLOCKSIZE) {
	prev  = block;
	block = block->next;
    }
    if (!block) {
	block = prev->next = new_comment_block();
    }
    block->line[block->count++] = line;
}


void
put_comment_block( comment_block_pt block, int indent )
{
    comment_block_pt cb;
    char *           str;
    int		     i;

    /* TBD: This routine might be the place to deallocate comment blocks. */

    if (!block)
	return;
    for (cb = block; cb; cb=cb->next) {
	for( i=0; i<cb->count; i++) {
	    for (str = strtok(cb->line[i],"\n"); str; str = strtok(0,"\n")){
		putf("%>%-- %s\n", indent, str);
	    }
	}
    }
}



