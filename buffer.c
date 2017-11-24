/*
 * General purpose unbounded character buffers.  This allows
 * us to buffer our scanning without arbitrary limitations
 * on any lexeme size.
 */

#include <assert.h>
#include <sys/types.h>
#include <stdlib.h>

#include "errors.h"
#include "buffer.h"
#include "allocate.h"

#undef NULL
#define NULL	0L

#define NEXT_INDEX(x)			(((x) + 1) & (MAX_OUTBUF_LEN - 1))

static buffer_t *free_list;

static void
reclaim(buf)
	buffer_t *buf;
{
	buf->next = free_list;
	free_list = buf;
}

void
buf_destroy(buf)
	buffer_t *buf;
{
	buffer_t *next, *p;

	for (p = buf->next; p; p = next) {
		next = p->next;
		reclaim(p);
	}

	buf->next = NULL;
	buf->last = NULL;
	buf->head = 0;
	buf->tail = 0;
}

void
buf_init(buf)
	buffer_t *buf;
{
	buf->next = NULL;
	buf->last = NULL;
	buf->head = 0;
	buf->tail = 0;
}

int
buf_empty(buf)
	buffer_t *buf;
{
	buffer_t *next;

	if (buf == NULL) {
		return 1;
	}

	if (buf->head == buf->tail) {
		next = buf->next;
		if (next != NULL) {
			*buf = *next;
			if (next == buf->last) {
				buf->last = NULL;
			}

			reclaim(next);
			return 0;
		}

		return 1;
	}

	return 0;
}

char
buf_get(buf)
	buffer_t *buf;
{
	if (buf->head == buf->tail) {
		if (buf_empty(buf)) {
			/* End of file */
			return 0;
		}
	}

	buf->head = NEXT_INDEX(buf->head);
	return buf->buf[buf->head];
}

static buffer_t*
gen_overflow_buf()
{
	buffer_t *buf;

	if ( (buf = free_list) ) {
		free_list = buf->next;
	}
	else {
		buf = (buffer_t*) malloc(sizeof(buffer_t));
		assert(buf != NULL);
	}

	buf_init(buf);
	return buf;
}

void
buf_add(buf, c)
	buffer_t *buf;
	int c;
{
	int index;
	buffer_t *l;

	l = buf->last;
	if (l != NULL) {
		if (l->tail == (MAX_OUTBUF_LEN - 1)) {
			l->next = gen_overflow_buf();
			buf->last = l->next;
			l = l->next;
		}

		l->tail = NEXT_INDEX(l->tail);
		l->buf[l->tail] = (char)c;
		return;
	}

	index = NEXT_INDEX(buf->tail);
	if (index == buf->head) {
		buf->next = gen_overflow_buf();
		l = buf->last = buf->next;
		l->buf[1] = (char)c;
		l->tail = 1;
		return;
	}

	buf->buf[index] = (char)c;
	buf->tail = index;
}

void
buf_concat(buf, from_buf)
	buffer_t *buf, *from_buf;
{
	char c;

	for (;;) {
		c = buf_get(from_buf);
		if (c == 0) break;
		buf_add(buf, (int)c);
	}
}

void
buf_add_str(buf, str)
	buffer_t *buf;
	char *str;
{
	if (str == NULL) {
		return;
	}

	for (; *str; str++) {
		buf_add(buf, (int) *str);
	}
}

int
buf_count(buf)
	buffer_t *buf;
{
	buffer_t *p;
	int count;

	if (buf == NULL) {
		return 0;
	}

	if (buf->tail >= buf->head) {
		count = buf->tail - buf->head;
	}
	else {
		count = MAX_OUTBUF_LEN - buf->head + buf->tail;
	}

	for (p = buf->next; p; p = p->next) {
		count += p->tail;
	}

	return count;
}

void
buf_move_to(buf, str)
	buffer_t *buf;
	char *str;
{
	int index = buf->head;
	int tail = buf->tail;
	buffer_t *next, *p;

	while (index != tail) {
		index = NEXT_INDEX(index);
		*str++ = buf->buf[index];
	}

	for (p = buf->next; p; p = next) {
		next = p->next;
		for (index = 1; index <= p->tail; index++) {
			*str++ = p->buf[index];
		}
		reclaim(p);
	}

	*str = 0;
	buf_init(buf);
}

char*
buf_get_str(buf)
	buffer_t *buf;
{
	int len = buf_count(buf);
	char *str;

	if (len == 0) {
		return NULL;
	}

	str = (char*) malloc(len + 1);
	assert(str != NULL);

	buf_move_to(buf, str);

	return str;
}
