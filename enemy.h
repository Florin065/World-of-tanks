#pragma once

#define GLM_ENABLE_EXPERIMENTAL  // NOLINT(clang-diagnostic-unused-macros)
#include <glm/gtx/rotate_vector.hpp>
using namespace std;

class enemy
{
    enum class state
    {
        forward,
        backward,
        left,
        right
    };

    state state_;
    
public:
    float health;
    glm::vec3 position;
    glm::vec3 forward;
    float tank_rotation_angle;
    float turret_rotation_angle;
    float last_shot_time;
    int is_shooting;
    std::vector<projectiles> bullets;
    float timer;

    enemy() : state_(), health(3), position(), forward(0, 0, -1), tank_rotation_angle(0), turret_rotation_angle(0),
              last_shot_time(0.0f), is_shooting(0), timer(0) {}

    void init()
    {
        constexpr float min_spawn_distance = 20.0f;

        do
        {
            const auto x = static_cast<float>(rand() % 75 - 25);  // NOLINT(concurrency-mt-unsafe)
            const auto z = static_cast<float>(rand() % 75 - 25);  // NOLINT(concurrency-mt-unsafe)

            constexpr float spawn_radius = 25.0f;
            const auto random_angle = static_cast<float>(rand() % 360);  // NOLINT(concurrency-mt-unsafe)
            const float spawn_x = x + spawn_radius * cos(glm::radians(random_angle));
            const float spawn_z = z + spawn_radius * sin(glm::radians(random_angle));

            position = glm::vec3(spawn_x, 0.35f, spawn_z);

        } while (length(position - glm::vec3(0, 0.35f, -2)) < min_spawn_distance);
    }

    void update(const float delta_time_seconds)
    {
        constexpr float speed = 2;
        constexpr float rotate_speed = 100;

        timer += delta_time_seconds;
        last_shot_time -= delta_time_seconds;

        if (last_shot_time < 0.0f) is_shooting = 0;

        if (timer >= 5)
        {
            const int next_state = rand() % 4;  // NOLINT(concurrency-mt-unsafe)
            timer = 0;
            
            switch (next_state)
            {
            case 0:
                state_ = state::forward;
                break;
            case 1:
                state_ = state::backward;
                break;
            case 2:
                state_ = state::left;
                break;
            case 3:
                state_ = state::right;
                break;
            default:
                break;
            }
        }

        switch (state_)
        {
        case state::forward:
            const glm::vec3 fw = normalize(glm::vec3(forward.x, 0, forward.z));
            position += fw * delta_time_seconds * speed;
            break;
        case state::backward:
            const glm::vec3 bw = normalize(glm::vec3(forward.x, 0, forward.z));
            position -= bw * delta_time_seconds * speed;
            break;
        case state::left:
            forward = rotate(forward, RADIANS(delta_time_seconds * rotate_speed), glm::vec3(0, 1, 0));
            tank_rotation_angle += delta_time_seconds * rotate_speed;
            break;
        case state::right:
            forward = rotate(forward, -RADIANS(delta_time_seconds * rotate_speed), glm::vec3(0, 1, 0));
            tank_rotation_angle -= delta_time_seconds * rotate_speed;
            break;
        }
    }

    void shoot_projectile()
    {
        if (is_shooting == 0)
        {
            constexpr auto turret_gun_offset = glm::vec3(0, 0.6f, 0);

            const glm::vec3 turret_gun_position = position + rotate(turret_gun_offset,
                RADIANS(turret_rotation_angle)
                + RADIANS(tank_rotation_angle), glm::vec3(0, 1, 0));
            const glm::vec3 gun_direction = rotate(forward,
                RADIANS(turret_rotation_angle), glm::vec3(0, 1, 0));

            projectiles bullet;
            bullet.init(turret_gun_position, gun_direction);
            bullets.push_back(bullet);
            
            last_shot_time = 3.f;
            is_shooting = 1;
        }
    }

    void rotate_turret_and_gun(const ally& ally)
    {
        const glm::vec3 to_ally = normalize(ally.position - position);
        const glm::vec3 forward_without_y = normalize(glm::vec3(forward.x, 0, forward.z));
        turret_rotation_angle = DEGREES(acos(dot(forward_without_y, to_ally)));
    
        const glm::vec3 cross_product = cross(forward_without_y, to_ally);
        if (cross_product.y < 0) {
            turret_rotation_angle = -turret_rotation_angle;
        }
    }

    void update_projectiles(const float delta_time_seconds, ally& ally, std::vector<aps>& aps)
    {
        for (auto& bullet : bullets)
        {
            bullet.update(delta_time_seconds);
            
            constexpr float projectile_radius = 0.1f;
            constexpr float tank_radius = 1.0f;
            const float distance = glm::distance(glm::vec2(bullet.position.x, bullet.position.z),
                                                 glm::vec2(ally.position.x, ally.position.z));
            constexpr float sum_of_radius = projectile_radius + tank_radius;

            if (distance < sum_of_radius && !bullet.hit)
            {
                ally.health -= 1;
                bullet.time = -1;
                bullet.hit = true;
            }

            for (auto& ap : aps)
            {
                constexpr float aps_radius = 0.6f;
                const float scaled_radius = aps_radius * std::max({ap.scale.x, ap.scale.y, ap.scale.z});
                const float distance1 = glm::distance(glm::vec2(bullet.position.x, bullet.position.z), glm::vec2(ap.position.x, ap.position.z));
                const float sum_of_radius1 = scaled_radius + projectile_radius;

                if (distance1 < sum_of_radius1) bullet.time = -1;
            }
        }

        bullets.erase(std::remove_if(
            bullets.begin(), bullets.end(),
            [](const projectiles& bullet) { return bullet.time <= 0.0f; }),
            bullets.end());
    }

    bool distance_to_ally(const ally& ally) const
    {
        return distance(position, ally.position) < 20;
    }
};
