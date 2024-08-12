#include "widget.h"

// I can fucking use global state if I want to
static Platform* platform = 0;

//! METHODS
_DEF_RO_PROC_AUTO(init, void)
_DEF_RO_PROC_AUTO(getChildren, RenderObjectVec*)
_DEF_RO_PROC_WRAPPED(handleEvent, void)
_DEF_RO_PROC_WRAPPED(draw, void)
_DEF_RO_PROC_WRAPPED(destroy, void)

void RO_draw(RenderObject* obj) {
    obj->ctx.platform = platform;
    _RO_draw(obj);
    obj->ctx.needsRedraw = false;
}

//* we're sending every event to every widget - i can't think of a better solution currently
//* as imagine a button receives MouseDown that's inside its bounds but then the MouseUp occurs outside - 
//* we need to send the button the MouseUp so it doesn't get stuck mid-press, but to do this while
//* still minimizing the amount of widgets we deliver events to we'd need to idk set up some kind of
//* callback system where a widget can "take ownership" of an event - that's the best idea i've got so far
//* but honestly if sending everything to everything is performant then why change it
// todo: is sending everything to everything performant?
void RO_handleEvent(RenderObject* obj) {
    // Vec2 eventPos = obj->ctx.platform->event.ui.pos;
    // Vec2 objPos = obj->ctx.pos;
    // Vec2 objSize = obj->ctx.size;
    // if (
    //     eventPos.x >= objPos.x &&
    //     eventPos.x <= objPos.x + objSize.x &&
    //     eventPos.y >= objPos.y &&
    //     eventPos.y <= objPos.y + objSize.y
    // ) {
        _RO_handleEvent(obj);
    // }
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
#define _UPDATE_ROOTSIZE() RO_setBounds(root, V2(0, 0), GetSize(platform))
    _UPDATE_ROOTSIZE();
    while (!quit) {
        Event* event;
        while ((event = GetEvent(platform)) != NULL) {
            if (event->class = Event_None) continue;
            if (event->class == Event_HandleAtTopLevel) {
                switch (event->tlType) {
                    case EventType_Quit: { quit = true; goto draw; } // HAHAHAHHAHAHAHHAHAHAHA
                    case EventType_TextInput: { textCB(event->text); continue; }
                    case EventType_WindowResized: { _UPDATE_ROOTSIZE(); RO_markNeedsRedraw(root); break; }
                }
            } else { RO_handleEvent(root); }
        }
        draw: RO_draw(root);
    };
#undef _UPDATE_ROOTSIZE
}