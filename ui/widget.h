#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include "vector.h"
#include "platform.h"
#include "ui_types.h"

typedef struct {
    Vec2 pos, size;
    Platform* platform;
    bool needsRedraw;
    RenderObject* parent;
} RenderContext;

typedef void (*TextInputHandler)(char*);

typedef struct RenderObject RenderObject;
DECL_VEC_NAMED(RenderObject*, RenderObjectVec)
typedef void (*RenderObjectMethod)(RenderContext*, void*);
typedef RenderObjectVec* (*RenderObjectGetChildren)(RenderContext*, void*);

#define _DECL_RO_PROC(name, type) type RO_##name(RenderObject* obj);
#define _DEF_RO_PROC_AUTO(name, type) type RO_##name(RenderObject* obj) { return obj->name(&obj->ctx, obj->config); }
#define _DEF_RO_PROC_WRAPPED(name, type) type _RO_##name(RenderObject* obj) { return obj->name(&obj->ctx, obj->config); }
// User defined method is methodName, no-extension wrapper is _methodName, extension defines _methodName which can then call generated wrapper __methodName
#define _DEF_RO_METHOD_NORETURN(name, method) inline void _##method##name(RenderContext* ctx, void* cfg) { if (method##name != 0) { method##name(ctx, (name##Config*)cfg); } }
#define _DEF_RO_METHOD_NORETURN_EXT(name, method) inline void __##method##name(RenderContext* ctx, void* _cfg) { name##ExtendedConfig* cfg = (name##ExtendedConfig*)_cfg; if (_method##name != 0) _method##name(ctx, cfg); } }
// MUST RETURN A POINTER SO WE CAN NULL OUT IF THE METHOD DOESN'T EXIST!!!!!!!!!!!
#define _DEF_RO_METHOD_WITHRETURN(name, method, type) inline type _##method##name(RenderContext* ctx, void* _cfg) { if (method##name != 0) { return method##name(ctx, (name##Config*)cfg); }return 0; }
#define _DEF_RO_METHOD_WITHRETURN_EXT(name, method, type) inline type __##method##name(RenderContext* ctx, void* cfg) { name##ExtendedConfig* cfg = (name##ExtendedConfig*)_cfg; if (_method##name != 0) { return _method##name(ctx, cfg); } return 0; }
#define _RO_METHOD(name, method) .method = _##method##name

//! METHODS
struct RenderObject {
    void* config;
    RenderContext ctx;
    RenderObjectMethod init;
    RenderObjectGetChildren getChildren;
    RenderObjectMethod handleEvent;
    RenderObjectMethod draw;
    RenderObjectMethod destroy;
};

RenderContext _create_RenderContext(RenderObject* parent);
void RO_setBounds(RenderObject* obj, Vec2 pos, Vec2 size);
void RO_markNeedsRedraw(RenderObject* obj);
// Utility to create a new ROV from varargs
inline RenderObjectVec ROList(int count, ...);
void RunUI(Platform* p, RenderObject* root, TextInputHandler textCB);

//! METHODS
_DECL_RO_PROC(init, void)
_DECL_RO_PROC(getChildren, RenderObjectVec*)
_DECL_RO_PROC(handleEvent, void)
_DECL_RO_PROC(draw, void)
_DECL_RO_PROC(destroy, void)

//! METHODS x2
#define DECL_WIDGET(name) \
_DEF_RO_METHOD_NORETURN(name, init) \
_DEF_RO_METHOD_WITHRETURN(name, getChildren, RenderObjectVec*) \
_DEF_RO_METHOD_NORETURN(name, handleEvent) \
_DEF_RO_METHOD_NORETURN(name, draw) \
_DEF_RO_METHOD_NORETURN(name, destroy) \
RenderObject* name(name##Config cfg) { name##Config* configPtr = (name##Config*)malloc(sizeof(name##Config)); *configPtr = cfg; RenderObject* out = malloc(sizeof(RenderObject)); *out = (RenderObject){.config = configPtr, .ctx = _create_RenderContext(out),\
_RO_METHOD(name, init), _RO_METHOD(getChildren), _RO_METHOD(handleEvent), _RO_METHOD(name, draw), _RO_METHOD(name, destroy)}; return out; }

//! METHODS x3
#define DECL_WIDGET_EXTENDED(name, ext) \
typedef struct name##ExtendedConfig { ext##_members(name) name##Config cfg; } \
_DEF_RO_METHOD_NORETURN_EXT(name, init) \
_DEF_RO_METHOD_WITHRETURN_EXT(name, getChildren, RenderObjectVec*) \
_DEF_RO_METHOD_NORETURN_EXT(name, handleEvent) \
_DEF_RO_METHOD_NORETURN_EXT(name, draw) \
_DEF_RO_METHOD_NORETURN_EXT(name, destroy) \
ext##_init(name) \
ext##_getChildren(name) \
ext##_handleEvent(name) \
ext##_draw(name) \
ext##_destroy(name) \
RenderObject* name(name##ExtendedConfig cfg) { name##ExtendedConfig* configPtr = (name##ExtendedConfig*)malloc(sizeof(name##ExtendedConfig)); *configPtr = cfg; RenderObject* out = malloc(sizeof(RenderObject)); *out = (RenderObject){.config = configPtr, .ctx = _create_RenderContext(out),\
_RO_METHOD(name, init), _RO_METHOD(getChildren), _RO_METHOD(handleEvent), _RO_METHOD(name, draw), _RO_METHOD(name, destroy)}; return out; }

#define _EVENT_PASSTHROUGH { \
    RenderObjectVec* children; \
    if ((children = RO_getChildren(ctx->parent)) != 0) { \
        FOREACH(RenderObject*, *children, child) { \
            RO_handleEvent(child); \
        } \
    } \
}

#define DEF_EVENT_PASSTHROUGH(name) void handleEvent##name(RenderContext* ctx, name##Config cfg) _EVENT_PASSTHROUGH

#define _EXT_CALLSUPER(name, method) return __method##name(ctx, &cfg->cfg);
#define _DEF_EXT_METHOD(name, method, returnType, code, callAfer) returnType _##method##name(RenderContext* ctx, name##ExtendedConfig* cfg) { code callAfter }
#define EXT_INSERTBEFORE(name, method, returnType, code) _DEF_EXT_METHOD(name, method, returnType, code, _EXT_CALLSUPER(name, method))
#define EXT_REPLACE(name, method, returnType, code) _DEF_EXT_METHOD(name, method, returnType, code,)
#define EXT_PASSTHROUGH(name, method, returnType) inline returnType _method##name(RenderContext* ctx, name##ExtendedConfig* cfg) { _EXT_CALLSUPER(name, method) }

//! ---------- FRAMEWORK WIDGETS, STRUCTS & EXTENSIONS ----------

//* todo: how do we
//*   a) compose multiple extensions together for a widget (preferably dynamically)
//*   b) nest extensions (eg builder <-- singlechild)

typedef RenderObject (*BuildCB)();
#define BUILDER_members(name) BuildCB buildCB; RenderObjectVec _child;
#define BUILDER_init(name) EXT_INSERTBEFORE(name, init, void, { INIT(cfg->_child) })
#define BUILDER_getChildren(name) EXT_REPLACE(name, getChildren, RenderObjectVec*, { return &cfg->_child; })
#define BUILDER_handleEvent(name) EXT_REPLACE(name, handleEvent, void, _EVENT_PASSTHROUGH)
#define BUILDER_draw(name) EXT_REPLACE(name, draw, void, { \
    if (ctx->needsRedraw) { \
        if (cfg->child.len != 0) RO_destroy(GET(cfg->_child, 0)); \
        APPEND(cfg->_child, cfg->buildCB()); \
    } \
    RenderObject* child = GET(cfg->_child, 0); \
    RO_setBounds(child, ctx->pos, ctx->size); \
    RO_draw(child); \
})
#define BUILDER_destroy(name) EXT_INSERTBEFORE(name, destroy, void, { \
    if (cfg->_child != 0) RO_destroy(cfg->_child); \
})

//* todo:
//*   a) this doesn't actually initialize shit
//*   b) generalize to haschildren
#define SINGLECHILD_members(name) RenderObjectVec _children;
#define SINGLECHILD_init(name) EXT_INSERTBEFORE(name, init, void, INIT(cfg->_children))
#define SINGLECHILD_getChildren(name) EXT_REPLACE(name, getChildren, RenderObjectVec* { \
    return cfg->_children; \
})
#define SINGLECHILD_handleEvent(name) EXT_PASSTHROUGH(name, handleEvent, void)
#define SINGLECHILD_draw(name) EXT_PASSTHROUGH(name, draw, void)
#define SINGLECHILD_destroy(name) EXT_INSERTBEFORE(name, destroy, void, { \
    DESTROY(cfg->_children); \
})

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
typedef struct { type##_listenable* listenable; type##_subscription _subscription;} type##_listenerConfig \
void init##type##_listener(RenderContext* ctx, type##_listenerConfig* cfg) \
RenderObjectGetChildren getChildren##type##_listener = 0; \
void handleEvent##type##_listener(RenderContext* ctx, type##_listenerConfig* cfg) = 0; \
void draw##type##_listener(RenderContext* ctx, type##_listenerConfig* cfg) = 0; \
void destroy##type##_listener(RenderContext* ctx, type##_listenerConfig* cfg) \
DECL_WIDGET_EXTENDED(type##_listener, BUILDER)

#define DEF_VALUELISTENER(type) \
void init##type##_listener(RenderContext* ctx, type##_listenerConfig* cfg) { \
    void onChanged(type newVal) { \
        RO_markNeedsRedraw(ctx->parent); \
    } \
    cfg->_subscription = type##_subscribe(cfg->listenable, onChanged); \
} \
void destroy##type##_listener(RenderContext* ctx, type##_listenerConfig* cfg) { \
    type##_unsubscribe(cfg->listenable, cfg->subscription.id); \
}