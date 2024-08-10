#include "widget.h"

// I can fucking use global state if I want to
static Renderer* renderer = NULL;

// ---------- DECLARE METHODS HERE + 4 PLACES IN WIDGET.H ----------

_DEF_RO_PROC_AUTO(init, void)
_DEF_RO_PROC_WRAPPED(draw, void)
_DEF_RO_PROC_AUTO(getChildren, RenderObjectVec*)
_DEF_RO_PROC_WRAPPED(destroy, void)

void RO_draw(RenderObject* obj) {
    if (obj->ctx.needsRedraw) {
        _RO_draw(obj);
        obj->ctx.needsRedraw = false;
    }
}

void RO_markNeedsRedraw(RenderObject* obj) {
    // Blackout last known position of object
    // todo: does this work??
    SetDrawColour(obj->ctx.ren, COL(0, 0, 0));
    FillRect(obj->ctx.ren, V2(obj->ctx.x, obj->ctx.y), V2(obj->ctx.w, obj->ctx.h));

    obj->ctx.needsRedraw = true;
    RenderObjectVec* children = RO_getChildren(obj);
    if (children != 0) {
        FOREACH(RenderObject*, *children, child) {
            RO_markNeedsRedraw(*child);
        }
    }
}

void RO_destroy(RenderObject* obj) {
    _RO_destroy(obj);
    free(obj->config);
    free(obj);
}

void initUI(Renderer* ren) {
    renderer = ren;
}

RenderContext _create_RenderContext(RenderObject* parent) {
    return (RenderContext){
        .x = 0, .y = 0, .w = 0, .h = 0,
        .ren = renderer,
        .needsRedraw = true,
        .parent = parent
    };
}

void RO_setBounds(RenderObject* obj, int x, int y, int w, int h) {
    obj->ctx.x = x;
    obj->ctx.y = y;
    obj->ctx.w = w;
    obj->ctx.h = h;
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