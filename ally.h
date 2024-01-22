#pragma once
#include <vector>
#include <glm/fwd.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "projectile.h"

class ally
{
public:
    float health;
    glm::vec3 position;
    glm::vec3 forward;
    float tank_rotation_angle;
    float turret_rotation_angle;
    float gun_rotation_angle_x;
    float gun_rotation_angle_y;
    float last_shot_time;
    std::vector<projectiles> bullets;

    ally() : health(3), position(), forward(0, 0, -1), tank_rotation_angle(0), turret_rotation_angle(0), gun_rotation_angle_x(0), gun_rotation_angle_y(0), last_shot_time(0) {}

    void init(const float x, const float y, const float z)
    {
        this->position = glm::vec3(x, y, z);
    }
};
