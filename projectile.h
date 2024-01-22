#pragma once

class projectiles
{
public:
    glm::vec3 position;
    glm::vec3 direction;
    float speed;
    float time;
    int id;
    bool hit;

    projectiles() : position(), direction(), speed(3), time(5), id(0), hit(false) {}

    void init(const glm::vec3& position, const glm::vec3& direction)
    {
        this->position = position;
        this->direction = direction;
    }

    void update(const float delta_time_seconds)
    {
        position += direction * speed * delta_time_seconds;
        time -= delta_time_seconds;
    }
};
