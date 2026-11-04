#pragma once
#include <SDL3/SDL.h>
#include <algorithm>

inline bool HasAABBIntersection(const SDL_FRect& a, const SDL_FRect& b)
{
    return (a.x < b.x + b.w && a.x + a.w > b.x &&
        a.y < b.y + b.h && a.y + a.h > b.y);
}

inline bool CheckAABB(const SDL_FRect& a, const SDL_FRect& b, SDL_FRect& outOverlap)
{
    float overlapX = std::min(a.x + a.w, b.x + b.w) - std::max(a.x, b.x);
    float overlapY = std::min(a.y + a.h, b.y + b.h) - std::max(a.y, b.y);

    if (overlapX <= 0.0f || overlapY <= 0.0f) return false;

    outOverlap.x = std::max(a.x, b.x);
    outOverlap.y = std::max(a.y, b.y);
    outOverlap.w = overlapX;
    outOverlap.h = overlapY;

    return true;
}