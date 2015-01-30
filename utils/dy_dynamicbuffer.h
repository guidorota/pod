#ifndef _DA_DYNARRAY_H
#define _DA_DYNARRAY_H

#include <unistd.h>

#define DY_DEFAULT_CAP 2048

struct dy_dynamicbuffer {
    void *buf;
    size_t len;
    size_t cap;
};

struct dy_dynamicbuffer *dy_create();

struct dy_dynamicbuffer *dy_create_cap(size_t cap);

int dy_add(struct dy_dynamicbuffer *d, const void *buf, size_t len);

void dy_free(struct dy_dynamicbuffer *d);

#endif /* _DA_DYNARRAY_H */
