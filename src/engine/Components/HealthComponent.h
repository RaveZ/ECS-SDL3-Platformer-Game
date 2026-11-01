#pragma once

struct HealthComponent
{
	int health = 100;
	int maxHealth = 100;

	HealthComponent() = default;
	HealthComponent(int health, int maxHealth) : health(health), maxHealth(maxHealth) {}
};