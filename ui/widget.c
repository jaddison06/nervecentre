#include "widget.h"

// I can fucking use global state if I want to
static Platform* platform = NULL;

//! METHODS
_DEF_RO_PROC_AUTO(init, void)
_DEF_RO_PROC_AUTO(getChildren, RenderObjectVec*)
_DEF_RO_PROC_AUTO(handleEvent, void)
_DEF_RO_PROC_WRAPPED(draw, void)
_DEF_RO_PROC_WRAPPED(destroy, void)

void RO_draw(RenderObject* obj) {
    obj->ctx.platform = platform;
    _RO_draw(obj);
    obj->ctx.needsRedraw = false;
}

void RO_destroy(RenderObject* obj) {
    _RO_destroy(obj);
    free(obj->config);
    free(obj);
}

RenderContext _create_RenderContext(RenderObject* parent) {
    return (RenderContext){
        .pos = V2(0, 0), .size = V2(0, 0),
        .platform = 0,
        .needsRedraw = true,
        .parent = parent
    };
}

void RO_setBounds(RenderObject* obj, Vec2 pos, Vec2 size) {
    obj->ctx.pos = pos;
    obj->ctx.size = size;
}

void RO_markNeedsRedraw(RenderObject* obj) {
    // Blackout last known position of object
    // todo: does this work??
    SetDrawColour(obj->ctx.platform, COL(0, 0, 0));
    FillRect(obj->ctx.platform, obj->ctx.pos, obj->ctx.size);

    obj->ctx.needsRedraw = true;
    RenderObjectVec* children = RO_getChildren(obj);
    if (children != 0) {
        FOREACH(RenderObject*, *children, child) {
            RO_markNeedsRedraw(*child);
        }
    }
}

inline RenderObjectVec ROList(int count, ...) {
    va_list va;
    va_start(va, count);
    RenderObjectVec out;
    INIT(out);
    for (int i = 0; i < count; i++) {
        APPEND(out, va_arg(va, RenderObject*));
    }
    va_end(va);
    return out;
}

void RunUI(Platform* p, RenderObject* root, TextInputHandler textCB) {
    platform = p;
    bool quit = false;
    while (!quit) {
        Event* event;
        while ((event = GetEvent(platform)) != NULL) {
            if (event->class == Event_HandleAtTopLevel) {
                if (event->tlType == EventType_Quit) { quit = true; break; }
                if (event->tlType == EventType_TextInput) { textCB(event->text); continue; }
            } else {
                RO_handleEvent(root);
            }
        }
        RO_draw(root);
    };
}