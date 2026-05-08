#include "config.h"

#include <stdlib.h>
#include <string.h>

typedef enum {
    ENT_STR,
    ENT_U32,
    ENT_BLOB,
} cfg_entry_type_t;

typedef struct cfg_entry {
    char key[16];
    cfg_entry_type_t type;
    char *str;
    uint32_t u32;
    void *blob;
    size_t blob_len;
    struct cfg_entry *next;
} cfg_entry_t;

struct cfg_handle {
    cfg_entry_t *head;
};

static cfg_entry_t *find_entry(cfg_handle_t *h, const char *key)
{
    for (cfg_entry_t *e = h->head; e; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            return e;
        }
    }
    return NULL;
}

static void free_entry_value(cfg_entry_t *e)
{
    free(e->str);
    free(e->blob);
    e->str = NULL;
    e->blob = NULL;
    e->blob_len = 0;
    e->u32 = 0;
}

static cfg_entry_t *get_or_create(cfg_handle_t *h, const char *key, cfg_entry_type_t type)
{
    cfg_entry_t *e = find_entry(h, key);
    if (e) {
        free_entry_value(e);
        e->type = type;
        return e;
    }
    e = calloc(1, sizeof *e);
    if (!e) {
        return NULL;
    }
    strncpy(e->key, key, sizeof e->key - 1);
    e->type = type;
    e->next = h->head;
    h->head = e;
    return e;
}

cfg_handle_t *cfg_open(void)
{
    return calloc(1, sizeof(struct cfg_handle));
}

void cfg_close(cfg_handle_t *h)
{
    if (!h) {
        return;
    }
    cfg_entry_t *e = h->head;
    while (e) {
        cfg_entry_t *next = e->next;
        free(e->str);
        free(e->blob);
        free(e);
        e = next;
    }
    free(h);
}

bool cfg_get_str(cfg_handle_t *h, const char *key, char *buf, size_t buflen)
{
    cfg_entry_t *e = find_entry(h, key);
    if (!e || e->type != ENT_STR) {
        return false;
    }
    size_t need = strlen(e->str) + 1;
    if (need > buflen) {
        return false;
    }
    memcpy(buf, e->str, need);
    return true;
}

bool cfg_set_str(cfg_handle_t *h, const char *key, const char *val)
{
    cfg_entry_t *e = get_or_create(h, key, ENT_STR);
    if (!e) {
        return false;
    }
    char *copy = strdup(val);
    if (!copy) {
        return false;
    }
    e->str = copy;
    return true;
}

bool cfg_get_u32(cfg_handle_t *h, const char *key, uint32_t *out)
{
    cfg_entry_t *e = find_entry(h, key);
    if (!e || e->type != ENT_U32) {
        return false;
    }
    *out = e->u32;
    return true;
}

bool cfg_set_u32(cfg_handle_t *h, const char *key, uint32_t val)
{
    cfg_entry_t *e = get_or_create(h, key, ENT_U32);
    if (!e) {
        return false;
    }
    e->u32 = val;
    return true;
}

bool cfg_get_blob(cfg_handle_t *h, const char *key, void *buf, size_t *inout_len)
{
    cfg_entry_t *e = find_entry(h, key);
    if (!e || e->type != ENT_BLOB) {
        return false;
    }
    if (buf == NULL) {
        *inout_len = e->blob_len;
        return true;
    }
    if (*inout_len < e->blob_len) {
        *inout_len = e->blob_len;
        return false;
    }
    memcpy(buf, e->blob, e->blob_len);
    *inout_len = e->blob_len;
    return true;
}

bool cfg_set_blob(cfg_handle_t *h, const char *key, const void *buf, size_t len)
{
    cfg_entry_t *e = get_or_create(h, key, ENT_BLOB);
    if (!e) {
        return false;
    }
    void *copy = malloc(len);
    if (!copy) {
        return false;
    }
    memcpy(copy, buf, len);
    e->blob = copy;
    e->blob_len = len;
    return true;
}

bool cfg_erase(cfg_handle_t *h, const char *key)
{
    cfg_entry_t **prev = &h->head;
    while (*prev) {
        if (strcmp((*prev)->key, key) == 0) {
            cfg_entry_t *gone = *prev;
            *prev = gone->next;
            free(gone->str);
            free(gone->blob);
            free(gone);
            return true;
        }
        prev = &(*prev)->next;
    }
    return false;
}

bool cfg_commit(cfg_handle_t *h)
{
    (void)h;
    return true;
}
