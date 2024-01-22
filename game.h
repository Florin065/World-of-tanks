#pragma once

#include "ally.h"
#include "aps.h"
#include "camera.h"
#include "enemy.h"
#include "components/simple_scene.h"
#include "components/transform.h"

struct viewport_area
{
    viewport_area() : x(0), y(0), width(1), height(1) {}
    viewport_area(const int x, const int y, const int width, const int height)
        : x(x), y(y), width(width), height(height) {}
    int x;
    int y;
    int width;
    int height;
};

namespace wot
{
    class game final : public gfxc::SimpleScene
    {
    public:
        // Constructor and destructor
        game() = default;
        ~game() override = default;

        // Copy constructor
        game(const game& other) : SimpleScene(other), camera_(), projection_matrix_()
        {
            // Perform copy of data members or resource ownership logic
        }

        // Copy assignment operator
        game& operator=(const game& other) {
            if (this != &other) {
                // Perform assignment of data members or resource ownership logic
            }
            return *this;
        }

        // Move constructor
        game(game&&) noexcept: camera_(), projection_matrix_()
        {
            // Perform move of data members or resource ownership logic
        }

        // Move assignment operator
        game& operator=(game&& other) noexcept {
            if (this != &other) {
                // Perform move assignment of data members or resource ownership logic
            }
            return *this;
        }

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float delta_time_seconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float delta_time, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouse_x, int mouse_y, int delta_x, int delta_y) override;
        void OnMouseBtnPress(int mouse_x, int mouse_y, int button, int mods) override;
        void OnMouseBtnRelease(int mouse_x, int mouse_y, int button, int mods) override;
        void OnMouseScroll(int mouse_x, int mouse_y, int offset_x, int offset_y) override;
        void OnWindowResize(int width, int height) override;

        void render_simple_mesh(const Mesh *mesh, const Shader *shader, const glm::mat4 &model_matrix, const glm::vec3 &color = glm::vec3(1), float hp = 3) const;
        void render_ally();
        void render_ground();
        void rotate_ally_turret_and_gun(float delta_x, float delta_y);
        void render_enemy();
        void generate_random_buildings();
        static bool is_too_close_to_other_buildings(const aps& new_building, const std::vector<aps>& existing_buildings, float min_distance);
        static bool is_enemy_too_close_to_a_building(const aps& building, const std::vector<enemy>& enemies, float min_distance);
        static void handle_enemy_enemy_collision(enemy& enemy1, enemy& enemy2);
        void handle_enemy_ally_collision(enemy& enemy);
        static void handle_enemy_collision_with_buildings(aps& building, enemy& enemy);
        void handle_ally_collision_with_buildings(aps& building);
        void ally_shoot_projectile();
        void update_ally_projectiles(float delta_time_seconds);
        
    protected:
        ally ally_;
        std::vector<enemy> enemies_;
        std::vector<aps> aps_;
        float time_ = 100;
        int score_ = 0;

        viewport_area mini_viewport_area_;
        tank_camera::camera *camera_;
        glm::mat4 projection_matrix_;
    };
}   // namespace wot
