#include "ui_types.h"

bool V2ContainedByBox(Vec2 point, Vec2 boxPos, Vec2 boxSize) {
    return (
        point.x >= boxPos.x &&
        point.x <= boxPos.x + boxSize.x &&
        point.y >= boxPos.y &&
        point.y <= boxPos.y + boxSize.y
    );
}