#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "so_internal.h"
#include "so_linker.h"

static so_context *_linker_context = NULL;

int so_linker_init() {
    if (_linker_context) {
        return 0;
    }

    _linker_context = (so_context *)calloc(1, sizeof(so_context));
    if (!_linker_context) {
        return -1;
    }

    return 0;
}

void so_linker_fini() {
    so_context *context = _linker_context;
    if (!context) {
        return;
    }
    _linker_context = NULL;

    so_module_node *node = context->modules_head;
    while (node) {
        so_module_node *next = node->next;
        if (node->mod) {
            node->mod->cookie = NULL;
            so_module_release(node->mod);
        }
        free(node);
        node = next;
    }
    free(context);
}

so_context *so_linker_get_context() {
    return _linker_context;
}

so_module *so_linker_find_module(const char *soname) {
    if (!_linker_context || !soname) {
        return NULL;
    }

    uint32_t        hash = so_sysv_hash(soname);
    so_module_node *node = _linker_context->modules_head;
    while (node) {
        so_module *mod = node->mod;
        if (mod && mod->soname && node->hash == hash && strcmp(mod->soname, soname) == 0) {
            return mod;
        }
        node = node->next;
    }

    return NULL;
}

int so_linker_register_module(so_module *mod) {
    if (!_linker_context || !mod) {
        return -1;
    }

    if (mod->cookie) {  // 已注册
        return 0;
    }

    so_module_node *node = (so_module_node *)calloc(1, sizeof(so_module_node));
    if (!node) {
        return -1;
    }

    mod->cookie = node;
    so_module_retain(mod);
    node->mod     = mod;
    node->context = _linker_context;
    node->hash    = so_sysv_hash(mod->soname);

    if (_linker_context->modules_tail) {
        _linker_context->modules_tail->next = node;
        node->prev                          = _linker_context->modules_tail;
        _linker_context->modules_tail       = node;
    } else {
        _linker_context->modules_head = node;
        _linker_context->modules_tail = node;
    }

    _linker_context->num_modules++;

    return 0;
}

int so_linker_unregister_module(so_module *mod) {
    if (!_linker_context || !mod) {
        return -1;
    }

    so_module_node *node = (so_module_node *)mod->cookie;
    if (!node || node->context != _linker_context) {
        return -1;  // 未注册，或不属于当前上下文
    }
    mod->cookie = NULL;  // 清理 cookie，防止二次注销

    if (node->prev) {
        node->prev->next = node->next;
    } else {
        _linker_context->modules_head = node;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        _linker_context->modules_tail = node;
    }

    _linker_context->num_modules--;

    so_module_release(mod);
    free(node);
    return 0;
}
