#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include "vector.h"
#include "platform.h"
#include "ui_types.h"

typedef void (*TextInputHandler)(char*);

typedef struct RenderObject RenderObject;
typedef struct RenderContext RenderContext;
DECL_VEC_NAMED(RenderObject*, RenderObjectVec)

//! METHODS
// Any non-void method must return a POINTER so we can null out if the method doesn't exist
#define _init_RETTYPE void
#define _getChildren_RETTYPE RenderObjectVec*
#define _handleEvent_RETTYPE void
#define _draw_RETTYPE void
#define _destroy_RETTYPE void

#define _METHOD_RETTYPE(method) _##method##_RETTYPE

#define _init_DEFAULT
#define _getChildren_DEFAULT 0
#define _handleEvent_DEFAULT
#define _draw_DEFAULT
#define _destroy_DEFAULT

#define _METHOD_DEFAULTRET(method) _##method##_DEFAULT

#define _METHOD_VAR(method) _METHOD_RETTYPE(method) (*method)(RenderContext*, void*)

struct RenderContext {
    Vec2 pos, constrainSize, actualSize;
    bool needsRedraw;
    RenderObject* parent;
};

//! METHODS
struct RenderObject {
#ifdef NCUI_DEBUG
    char* _name;
#endif
    void* cfg;
    RenderContext ctx;
    _METHOD_VAR(init);
    _METHOD_VAR(getChildren);
    _METHOD_VAR(handleEvent);
    _METHOD_VAR(draw);
    _METHOD_VAR(destroy);
};

#define _DECL_RC_PROC(method) extern inline _METHOD_RETTYPE(method) RC_##method(RenderContext* ctx);
#define _DEF_RC_PROC(method) extern inline _METHOD_RETTYPE(method) RC_##method(RenderContext* ctx) { return RO_##method(ctx->parent); }
#define _DECL_RO_PROC_AUTO(method) extern inline _METHOD_RETTYPE(method) RO_##method(RenderObject* obj); _DECL_RC_PROC(method)
#define _DECL_RO_PROC_WRAPPED(method) extern inline _METHOD_RETTYPE(method) _RO_##method(RenderObject* obj); _METHOD_RETTYPE(method) RO_##method(RenderObject* obj); _DECL_RC_PROC(method)
#define _DEF_RO_PROC_AUTO(method) extern inline _METHOD_RETTYPE(method) RO_##method(RenderObject* obj) { return obj->method(&obj->ctx, obj->cfg); } _DEF_RC_PROC(method)
#define _DEF_RO_PROC_WRAPPED(method) extern inline _METHOD_RETTYPE(method) _RO_##method(RenderObject* obj) { return obj->method(&obj->ctx, obj->cfg); } _DEF_RC_PROC(method)
// User defined method is methodName, wrapper is _methodName, extension defines __methodName which can then call generated wrapper _methodName
#define _DECL_RO_METHOD(method, name) extern inline _METHOD_RETTYPE(method) _##method##name(RenderContext* ctx, void* cfg);
#define _DEF_RO_METHOD(method, name) extern inline _METHOD_RETTYPE(method) _##method##name(RenderContext* ctx, void* cfg) { return method##name(ctx, (name##Config*)cfg); }
#define _RO_METHOD(method, name) .method = _##method##name
#define _RO_EXTMETHOD(method, name) .method = __##method##name

RenderContext _create_RenderContext(RenderObject* parent);
void RO_setBounds(RenderObject* obj, Vec2 pos, Vec2 size);
void RO_markNeedsRedraw(RenderObject* obj);
extern inline void RC_markNeedsRedraw(RenderContext* ctx);
// Utility to create a new ROV from varargs
extern inline RenderObjectVec ROList(int count, ...);
void RunUI(RenderObject* root, TextInputHandler textCB);

#define FOREACH_CHILD(obj, code) { \
    RenderObjectVec* children; \
    if ((children = RO_getChildren(obj)) != 0) { \
        FOREACH(RenderObject*, *children, _child) { \
            RenderObject* child = *_child; \
            { code } \
        } \
    } \
}

#define NOP_METHOD(method, name) _METHOD_RETTYPE(method) method##name(RenderContext* context, name##Config* cfg) { return _METHOD_DEFAULTRET(method); }

//! METHODS
_DECL_RO_PROC_AUTO(init)
_DECL_RO_PROC_AUTO(getChildren)
_DECL_RO_PROC_WRAPPED(handleEvent)
_DECL_RO_PROC_WRAPPED(draw)
_DECL_RO_PROC_WRAPPED(destroy)

#define _DECL_WIDGET_METHOD(method, name) _METHOD_RETTYPE(method) method##name(RenderContext* ctx, name##Config* cfg); 

//! METHODS
#define _DECL_WIDGET_MEMBERS(name, configMembers) typedef struct { configMembers } name##Config; \
_DECL_WIDGET_METHOD(init, name) \
_DECL_WIDGET_METHOD(getChildren, name) \
_DECL_WIDGET_METHOD(handleEvent, name) \
_DECL_WIDGET_METHOD(draw, name) \
_DECL_WIDGET_METHOD(destroy, name)

//! METHODS
#define DECL_WIDGET(name, configMembers) \
_DECL_WIDGET_MEMBERS(name, configMembers) \
_DECL_RO_METHOD(init, name) \
_DECL_RO_METHOD(getChildren, name) \
_DECL_RO_METHOD(handleEvent, name) \
_DECL_RO_METHOD(draw, name) \
_DECL_RO_METHOD(destroy, name) \
RenderObject* name(name##Config cfg);

#ifdef NCUI_DEBUG
#define _RO_NAMEFIELD(name) ._name = #name,
#else
#define _RO_NAMEFIELD(name)
#endif

//! METHODS
#define DEF_WIDGET(name) \
_DEF_RO_METHOD(init, name) \
_DEF_RO_METHOD(getChildren, name) \
_DEF_RO_METHOD(handleEvent, name) \
_DEF_RO_METHOD(draw, name) \
_DEF_RO_METHOD(destroy, name) \
RenderObject* name(name##Config cfg) { name##Config* configPtr = (name##Config*)malloc(sizeof(name##Config)); *configPtr = cfg; RenderObject* out = (RenderObject*)malloc(sizeof(RenderObject)); *out = (RenderObject){_RO_NAMEFIELD(name) .cfg = configPtr, .ctx = _create_RenderContext(out),\
_RO_METHOD(init, name), _RO_METHOD(getChildren, name), _RO_METHOD(handleEvent, name), _RO_METHOD(draw, name), _RO_METHOD(destroy, name) \
}; RO_init(out); return out; }

#define _DECL_EXT_METHOD(ext, method, name) _METHOD_RETTYPE(method) __##method##name(RenderContext* ctx, void* _cfg);
#define _DEF_EXT_METHOD(ext, method, name) _METHOD_RETTYPE(method) __##method##name(RenderContext* ctx, void* _cfg) { name##ExtendedConfig* cfg = (name##ExtendedConfig*) _cfg; ext##_##method(name) }

//! METHODS x2
#define DECL_WIDGET_EXTENDED(name, configMembers, ext) \
_DECL_WIDGET_MEMBERS(name, configMembers) \
typedef struct { ext##_members(name) name##Config cfg; } name##ExtendedConfig; \
_DECL_RO_METHOD(init, name) \
_DECL_RO_METHOD(getChildren, name) \
_DECL_RO_METHOD(handleEvent, name) \
_DECL_RO_METHOD(draw, name) \
_DECL_RO_METHOD(destroy, name) \
_DECL_EXT_METHOD(ext, init, name) \
_DECL_EXT_METHOD(ext, getChildren, name) \
_DECL_EXT_METHOD(ext, handleEvent, name) \
_DECL_EXT_METHOD(ext, draw, name) \
_DECL_EXT_METHOD(ext, destroy, name) \
RenderObject* name(name##ExtendedConfig cfg);

//! MEHODS x3
#define DEF_WIDGET_EXTENDED(name, ext) \
_DEF_RO_METHOD(init, name) \
_DEF_RO_METHOD(getChildren, name) \
_DEF_RO_METHOD(handleEvent, name) \
_DEF_RO_METHOD(draw, name) \
_DEF_RO_METHOD(destroy, name) \
_DEF_EXT_METHOD(ext, init, name) \
_DEF_EXT_METHOD(ext, getChildren, name) \
_DEF_EXT_METHOD(ext, handleEvent, name) \
_DEF_EXT_METHOD(ext, draw, name) \
_DEF_EXT_METHOD(ext, destroy, name) \
RenderObject* name(name##ExtendedConfig cfg) { name##ExtendedConfig* configPtr = (name##ExtendedConfig*)malloc(sizeof(name##ExtendedConfig)); *configPtr = cfg; RenderObject* out = (RenderObject*)malloc(sizeof(RenderObject)); *out = (RenderObject){_RO_NAMEFIELD(name) .cfg = configPtr, .ctx = _create_RenderContext(out),\
_RO_EXTMETHOD(init, name), _RO_EXTMETHOD(getChildren, name), _RO_EXTMETHOD(handleEvent, name), _RO_EXTMETHOD(draw, name), _RO_EXTMETHOD(destroy, name)}; RO_init(out); return out; }

#define EXT_CALLSUPER(method, name) _##method##name(ctx, &cfg->cfg)

//! ---------- FRAMEWORK EXTENSIONS, STRUCTS AND WIDGETS ----------

#define EVENTPASSTHROUGH_members(name)
#define EVENTPASSTHROUGH_init(name) EXT_CALLSUPER(init, name);
#define EVENTPASSTHROUGH_getChildren(name) return EXT_CALLSUPER(getChildren, name);
#define EVENTPASSTHROUGH_handleEvent(name) FOREACH_CHILD(ctx->parent, RO_handleEvent(child);)
#define EVENTPASSTHROUGH_draw(name) EXT_CALLSUPER(draw, name);
#define EVENTPASSTHROUGH_destroy(name) EXT_CALLSUPER(destroy, name);

// Need to #define DEFAULT_FONT ... before #including this header!!
#define DEFAULTFONT_members(name) Font* _font;
#define DEFAULTFONT_init(name) cfg->_font = CreateFont(DEFAULT_FONT); EXT_CALLSUPER(init, name);
#define DEFAULTFONT_getChildren(name) return EXT_CALLSUPER(getChildren, name);
#define DEFAULTFONT_handleEvent(name) EXT_CALLSUPER(handleEvent, name);
#define DEFAULTFONT_draw(name) EXT_CALLSUPER(draw, name);
#define DEFAULTFONT_destroy(name) EXT_CALLSUPER(destroy, name); DestroyFont(cfg->_font);

#define DEFAULTFONT_GET(name) ((name##ExtendedConfig*)ctx->parent->cfg)->_font

// Private `_children` field initialized and handled internally, useful for e.g. Builder
#define HASCHILDREN_members(name) RenderObjectVec _children;
#define HASCHILDREN_init(name) { INIT(cfg->_children); EXT_CALLSUPER(init, name); }
#define HASCHILDREN_getChildren(name) { return &cfg->_children; }
#define HASCHILDREN_handleEvent(name) EXT_CALLSUPER(handleEvent, name);
#define HASCHILDREN_draw(name) EXT_CALLSUPER(draw, name);
#define HASCHILDREN_destroy(name) { \
    EXT_CALLSUPER(destroy, name); \
    DESTROY(cfg->_children); \
}

// Public `children` field passed in, initialized using `ROList()` and destroyed here
#define TAKESCHILDREN_members(name) RenderObjectVec children;
#define TAKESCHILDREN_init(name) EXT_CALLSUPER(init, name);
#define TAKESCHILDREN_getChildren(name) return &cfg->children;
#define TAKESCHILDREN_handleEvent(name) EXT_CALLSUPER(handleEvent, name);
#define TAKESCHILDREN_draw(name) EXT_CALLSUPER(draw, name);
#define TAKESCHILDREN_destroy(name) { \
    EXT_CALLSUPER(destroy, name); \
    DESTROY(cfg->children); /* We didn't initialize it but we're assuming it was passed in with `ROList()` so we've got the only ref */ \
}

typedef RenderObject* (*BuildCB)();
// Yippeeeeeee extension nesting i sure do hope this doesn't eat shit
#define BUILDER_members(name) BuildCB buildCB; HASCHILDREN_members(name)
#define BUILDER_init HASCHILDREN_init
#define BUILDER_getChildren HASCHILDREN_getChildren
#define BUILDER_handleEvent EVENTPASSTHROUGH_handleEvent
#define BUILDER_draw(name) { \
    RenderObjectVec* children = &cfg->_children; \
    if (children->len != 0) { \
        RO_destroy(GET(*children, 0)); \
        REMOVE(*children, 0); \
    } \
    APPEND(*children, cfg->buildCB()); \
    RO_setBounds(GET(*children, 0), ctx->pos, ctx->constrainSize); \
    RO_draw(GET(*children, 0)); \
    ctx->actualSize = GET(*children, 0)->ctx.actualSize; \
}
#define BUILDER_destroy(name) { RenderObjectVec* children = &cfg->_children; if (children->len > 0) RO_destroy(GET(*children, 0)); HASCHILDREN_destroy(name) }

#define DECL_VALUELISTENABLE(type) \
typedef void (*_##type##_subscriberCB)(type, void*); \
typedef struct { _##type##_subscriberCB onChanged; void* callerData; int id; } _##type##_subscriber; \
DECL_VEC(_##type##_subscriber) \
typedef struct { type value; int nextID; _##type##_subscriberVec subscribers; } type##_listenable; \
typedef struct { type* value; int id; } type##_subscription; \
type##_listenable* new_##type##_listenable(type initialValue); \
void update_##type##listenable(type##_listenable* listenable, type newVal); \
void destroy_##type##_listenable(type##_listenable* listenable); \
type##_subscription type##_subscribe(type##_listenable* listenable, _##type##_subscriberCB onChanged, void* callerData); \
void type##_unsubscribe(type##_listenable* listenable, int id);

#define DEF_VALUELISTENABLE(type) \
type##_listenable* new_##type##_listenable(type initialValue) { \
    type##_listenable* out = malloc(sizeof(type##_listenable)); \
    _##type##_subscriberVec subscribers; \
    INIT(subscribers); \
    *out = (type##_listenable){.value = initialValue, .nextID = 0, .subscribers = subscribers}; \
    return out; \
} \
void update_##type##listenable(type##_listenable* listenable, type newVal) { \
    listenable->value = newVal; \
    FOREACH(_##type##_subscriber, listenable->subscribers, subscriber) { \
        subscriber->onChanged(newVal, subscriber->callerData); \
    } \
} \
void destroy_##type##_listenable(type##_listenable* listenable) { \
    DESTROY(listenable->subscribers); \
    free(listenable); \
} \
type##_subscription type##_subscribe(type##_listenable* listenable, _##type##_subscriberCB onChanged, void* callerData) { \
    int id = listenable->nextID++; \
    APPEND(listenable->subscribers, ((_##type##_subscriber){.onChanged = onChanged, .callerData = callerData, .id = id})); \
    return (type##_subscription){.value = &listenable->value, .id = id}; \
} \
void type##_unsubscribe(type##_listenable* listenable, int id) { \
    for (int i = 0; i < listenable->subscribers.len; i++) { \
        if (GET(listenable->subscribers, i).id == id) { REMOVE(listenable->subscribers, i); break; } \
    } \
}

#define DECL_VALUELISTENER(type) DECL_WIDGET_EXTENDED(type##_listener, type##_listenable* listenable; type##_subscription _subscription;, BUILDER)

#define DEF_VALUELISTENER(type) \
DEF_WIDGET_EXTENDED(type##_listener, BUILDER) \
void _onChanged_##type##_listener(type newVal, void* ctx) { RO_markNeedsRedraw(((RenderContext*)ctx)->parent); } \
void init##type##_listener(RenderContext* ctx, type##_listenerConfig* cfg) { \
    cfg->_subscription = type##_subscribe(cfg->listenable, _onChanged_##type##_listener, ctx); \
} \
RenderObjectVec* getChildren##type##_listener(RenderContext* ctx, type##_listener##Config* cfg) { return 0; } \
void handleEvent##type##_listener(RenderContext* ctx, type##_listener##Config* cfg) {} \
void draw##type##_listener(RenderContext* ctx, type##_listener##Config* cfg) {} \
void destroy##type##_listener(RenderContext* ctx, type##_listenerConfig* cfg) { type##_unsubscribe(cfg->listenable, cfg->_subscription.id); }
