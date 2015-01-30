#ifndef _DY_DYNBUF_H
#define _DY_DYNBUF_H

#include <unistd.h>

#define DY_DEFAULT_CAP 2048

struct dy_dynbuf {
    void *buf;
    size_t len;
    size_t cap;
};

struct dy_dynbuf *dy_create();

struct dy_dynbuf *dy_create_cap(size_t cap);

int dy_add(struct dy_dynbuf *d, const void *buf, size_t len);

void dy_free(struct dy_dynbuf *d);

#endif /* _DY_DYNBUF_H */
