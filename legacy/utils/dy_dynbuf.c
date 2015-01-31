#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "dy_dynbuf.h"

#define DY_FREE(d) ((char *) d->buf + d->len)

static int dy_expand(struct dy_dynbuf *d, size_t min);

struct dy_dynbuf *dy_create() {
    return dy_create_cap(DY_DEFAULT_CAP);
}

struct dy_dynbuf *dy_create_cap(size_t cap)
{
    struct dy_dynbuf *d;

    d = calloc(1, sizeof *d);
    if (d == NULL) {
        return NULL;
    }

    d->buf = malloc(cap);
    if (d->buf == NULL) { 
        free(d);
        return NULL;
    }

    d->cap = cap;
    return d;
}

int dy_add(struct dy_dynbuf *d, const void *buf, size_t len)
{
    if (d == NULL || buf == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (len == 0) {
        return 0;
    }

    if (d->len + len > d->cap) {
        if (dy_expand(d, len) < 0) {
            return -1;
        }
    }

    memcpy(DY_FREE(d), buf, len);
    d->len += len;
    return 0;
}

static int dy_expand(struct dy_dynbuf *d, size_t min)
{
    size_t new_cap = d->cap;

    if (d->len + min > SIZE_MAX) {
        errno = ENOMEM;
        return -1;
    }

    do {
        if (new_cap * 2 > SIZE_MAX) {
            return -1;
        }
        new_cap *= 2;
    } while (new_cap < min);
    
    if (realloc(d->buf, new_cap) == NULL) {
        return -1;
    }
    d->cap = new_cap;

    return 0;
}

void dy_free(struct dy_dynbuf *d)
{
    if (d == NULL) {
        return;
    }

    free(d->buf);
    free(d);
}