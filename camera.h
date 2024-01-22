#pragma once

#include "utils/glm_utils.h"

namespace tank_camera
{
    class camera
    {
     public:
        camera()
        {
            position    = glm::vec3(0, 2, 5);
            forward     = glm::vec3(0, 0, -1);
            up          = glm::vec3(0, 1, 0);
            right       = glm::vec3(1, 0, 0);
            distance_to_target = sqrt(20);
        }

        camera(const glm::vec3 &position, const glm::vec3 &center, const glm::vec3 &up)
        {
            set(position, center, up);
        }

        ~camera()
        = default;

        // Update camera
        void set(const glm::vec3 &position, const glm::vec3 &center, const glm::vec3 &up)
        {
            this->position = position;
            forward     = normalize(center - position);
            right       = cross(forward, up);
            this->up    = cross(right, forward);
        }

        void move_forward(const float distance)
        {
            const glm::vec3 dir = normalize(glm::vec3(forward.x, 0, forward.z));
            position += dir * distance;
        }

        void translate_forward(const float distance)
        {
            position += normalize(forward) * distance;
        }

        void translate_upward(const float distance)
        {
            position += normalize(up) * distance;
        }

        void translate_right(const float distance)
        {
            position += normalize(right) * distance;
        }

        void rotate_first_person_ox(const float angle)
        {
            forward = normalize(rotate(glm::mat4(1.f), angle, right) * glm::vec4(forward, 0));
            up        = normalize(cross(right, forward));
        }

        void rotate_first_person_oy(const float angle)
        {
            forward = normalize(rotate(glm::mat4(1.f), angle, glm::vec3(0, 1, 0)) * glm::vec4(forward, 0));
            right   = normalize(rotate(glm::mat4(1.f), angle, glm::vec3(0, 1, 0)) * glm::vec4(right, 0));
            up      = normalize(cross(right, forward));
        }

        void rotate_first_person_oz(const float angle)
        {
            right = normalize(rotate(glm::mat4(1.f), angle, forward) * glm::vec4(right, 0));
            up = normalize(rotate(glm::mat4(1.f), angle, forward) * glm::vec4(up, 0));
            forward = normalize(cross(up, right));
        }

        void rotate_third_person_ox(const float angle)
        {
            translate_forward(distance_to_target);
            rotate_first_person_ox(angle);
            translate_forward(-distance_to_target);
        }

        void rotate_third_person_oy(const float angle)
        {
            translate_forward(distance_to_target);
            rotate_first_person_oy(angle);
            translate_forward(-distance_to_target);
        }

        void rotate_third_person_oz(const float angle)
        {
            translate_forward(distance_to_target);
            rotate_first_person_oz(angle);
            translate_forward(-distance_to_target);
        }

        glm::mat4 get_view_matrix() const
        {
            return lookAt(position, position + forward, up);
        }

        glm::vec3 get_target_position() const
        {
            return position + forward * distance_to_target;
        }

        float distance_to_target;
        glm::vec3 position;
        glm::vec3 forward;
        glm::vec3 right;
        glm::vec3 up;
    };
}   // namespace tank_camera
