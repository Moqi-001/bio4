#include <stdlib.h>
#include <string.h>

#include "so_internal.h"
#include "so_linker.h"

so_context *so_context_create() {
    so_context *context = (so_context *)calloc(1, sizeof(so_context));

    return context;
}

void so_context_destroy(so_context *context) {
    if (context) {
        so_module_node *node = context->modules;
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
}

so_module *so_context_find_module(so_context *context, const char *soname) {
    if (!context || !soname) return NULL;

    so_module_node *node = context->modules;
    while (node) {
        if (node->mod && node->mod->soname && strcmp(node->mod->soname, soname) == 0) {
            return node->mod;
        }
        node = node->next;
    }

    return NULL;
}

int so_context_add_module(so_context *context, so_module *mod) {
    if (!context || !mod || mod->cookie) return -1;

    so_module_node *node = calloc(1, sizeof(so_module_node));
    if (!node) {
        return -1;
    }

    node->mod     = mod;
    node->context = context;
    mod->cookie   = node;

    if (!context->modules) {
        context->modules = node;
    } else {
        context->modules->prev = node;
        node->next             = context->modules;
        context->modules       = node;
    }

    so_module_retain(mod);
    context->num_modules++;

    return 0;
}

int so_context_remove_module(so_context *context, so_module *mod) {
    if (!context || !mod || !mod->cookie) return -1;

    so_module_node *node = (so_module_node *)mod->cookie;

    if (node->context != context) {  // 当前module不位于目标context里，终止操作
        return -1;
    }

    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
    if (context->modules == node) {
        context->modules = node->next;
    }

    mod->cookie = NULL;  // 清理 cookie，防止二次使用
    so_module_release(mod);
    free(node);
    context->num_modules--;
    return 0;
}
