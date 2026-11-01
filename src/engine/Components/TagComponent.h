#pragma once

enum class EntityTag {
    Player,
    Enemy,
    Bullet,
    Level
};

struct TagComponent {
    EntityTag tag;

    TagComponent() = default;
	TagComponent(EntityTag tag) : tag(tag) {}
};