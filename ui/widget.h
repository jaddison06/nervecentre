#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include "vector.h"
#include "renderer.h"

typedef struct {
    int x, y, w, h;
    Renderer* ren;
    bool needsRedraw;
    RenderObject* parent;
} RenderContext;

typedef struct RenderObject RenderObject;
DECL_VEC_NAMED(RenderObject*, RenderObjectVec)
typedef void (*RenderObjectMethod)(RenderContext*, void*);
typedef RenderObjectVec* (*RenderObjectGetChildren)(RenderContext*, void*);

#define _DECL_RO_PROC(name, type) type RO_##name(RenderObject* obj);
#define _DEF_RO_PROC_AUTO(name, type) type RO_##name(RenderObject* obj) { return obj->name(&obj->ctx, obj->config); }
#define _DEF_RO_PROC_WRAPPED(name, type) type _RO_##name(RenderObject* obj) { return obj->name(&obj->ctx, obj->config); }
#define _DEF_RO_METHOD_NORETURN(name, method) inline void _##method##name(RenderContext* ctx, void* cfg) { if (method##name != 0) { method##name(ctx, (name##Config*)cfg); } }
// MUST RETURN A POINTER SO WE CAN NULL OUT IF THE METHOD DOESN'T EXIST!!!!!!!!!!!
#define _DEF_RO_METHOD_WITHRETURN(name, method, type) inline type _##method##name(RenderContext* ctx, void* cfg) { if (method##name != 0) { return method##name(ctx, (name##Config*)cfg); } return 0; }
#define _RO_METHOD(name, method) .method = _##method##name

// ---------- 4 PLACES IN THIS FILE WHERE YOU HAVE TO DECLARE METHODS + 1 IN WIDGET.C ----------

struct RenderObject {
    void* config;
    RenderContext ctx;
    RenderObjectMethod init;
    RenderObjectGetChildren getChildren;
    RenderObjectMethod draw;
    RenderObjectMethod destroy;
};

_DECL_RO_PROC(init, void)
_DECL_RO_PROC(draw, void)
_DECL_RO_PROC(getChildren, RenderObjectVec*)
_DECL_RO_PROC(destroy, void)

#define DECL_WIDGET(name) \
_DEF_RO_METHOD_NORETURN(name, init) \
_DEF_RO_METHOD_NORETURN(name, draw) \
_DEF_RO_METHOD_WITHRETURN(name, getChildren, RenderObjectVec*) \
_DEF_RO_METHOD_NORETURN(name, destroy) \
RenderObject* name(name##Config config) { name##Config* configPtr = (name##Config*)malloc(sizeof(name##Config)); *configPtr = config; RenderObject* out = malloc(sizeof(RenderObjec)); *out = (RenderObject){.config = configPtr, .ctx = _create_RenderContext(out),\
_RO_METHOD(name, init), _RO_METHOD(name, draw), _RO_METHOD(getChildren), _RO_METHOD(name, destroy)}; return out; }

#define DECL_VALUELISTENABLE(type) \
typedef void (*_##type##_subscriberCB)(type); \
typedef struct { _##type##_subscriberCB onChanged; int id; } _##type##_subscriber; \
DECL_VEC(_##type##_subscriber) \
typedef struct { type value; int nextID; _##type##_subscriberVec subscribers; } type##_listenable; \
typedef struct { type* value; int id; } type##_subscription \
type##_subscription type##_subscribe(type##_listenable* listenable, _##type##_subscriberCB onChanged); \
void type##_unsubscribe(type##_listenable* listenable, int id); \
type##_listenable new_##type##_listenable(type initialValue); \
void update_##type##listenable(type##_listenable* listenable, type newVal); \
void destroy_##type##_listenable(type##_listenable* listenable);

#define DEF_VALUELISTENABLE(type) \
type##_subscription type##_subscribe(type##_listenable* listenable, _##type##_subscriberCB onChanged) { \
    int id = listenable->nextID++; \
    APPEND(listenable->subscribers, (_##type##_subscriber){.onChanged = onChanged, .id = id}); \
    return (type##_subscription){.value = &listenable.value, .id = id}; \
} \
void type##_unsubscribe(type##_listenable* listenable, int id) { \
    for (int i = 0; i < listenable->subscribers.len; i++) { \
        if (GET(listenable->subscribers, i).id == id) { REMOVE(listenable->subscribers, i); break; } \
    } \
} \
type##_listenable new_##type##_listenable(type initialValue) { \
    _##type##_subscriberVec subscribers; \
    INIT(subscribers); \
    return (type##_listenable){.value = initialValue, .nextID = 0, .subscribers = subscribers}; \
} \
void update_##type##listenable(type##_listenable* listenable, type newVal) { \
    listenable->value = newVal; \
    FOREACH(_##type##_subscriber, listenable->subscribers, subscriber) { \
        subscriber->onChanged(newVal); \
    } \
} \
void destroy_##type##_listenable(type##_listenable* listenable) { \
    DESTROY(listenable->subscribers); \
}

#define DECL_VALUELISTENER(type) \
typedef RenderObject (*_##type##_listener_buildCB)(type value); \
typedef struct { _##type##_listener_buildCB buildCB; type##_listenable* listenable; type##_subscription _subscription;} type##_listenerConfig \
void init##type##_listener(RenderContext* context, type##_listenerConfig* config) \
RenderObjectGetChildren getChildren##type##_listener = 0; \
void draw##type##_listener(RenderContext* context, type##_listenerConfig* config) \
void destroy##type##_listener(RenderContext* context, type##_listenerConfig* config) \
DECL_WIDGET(type##_listener)

#define DEF_VALUELISTENER(type) \
void init##type##_listener(RenderContext* context, type##_listenerConfig* config) { \
    void onChanged(type newVal) { \
        RO_markNeedsRedraw(context->parent); \
    } \
    config->_subscription = type##_subscribe(config->listenable, onChanged); \
} \
void draw##type##_listener(RenderContext* context, type##_listenerConfig* config) { \
    RenderObject* child = config->buildCB(*config->_subscription.value); \
    RO_setBounds(child, context->x, context->y, context->w, context->h); \
    RO_draw(child); \
    RO_destroy(child); \
} \
void destroy##type##_listener(RenderContext* context, type##_listenerConfig* config) { \
    type##_unsubscribe(config->listenable, config->subscription.id); \
}

RenderContext _create_RenderContext(RenderObject* parent);
void initUI(Renderer* ren);
void RO_setBounds(RenderObject* obj, int x, int y, int w, int h);
void RO_markNeedsRedraw(RenderObject* obj);
// Utility to create a new ROV from varargs
inline RenderObjectVec ROList(int count, ...);