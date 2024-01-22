#include "game.h"

#include <vector>
#include <string>
#include <random>

#define GLM_ENABLE_EXPERIMENTAL  // NOLINT(clang-diagnostic-unused-macros)
#include <iostream>
#include <glm/gtx/rotate_vector.hpp>

#include "ally.h"
#include "aps.h"
#include "camera.h"

using namespace std;
using namespace wot;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */

void game::Init()
{
    {
        const auto mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        const auto mesh = new Mesh("plane");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        const auto mesh = new Mesh("tracks");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tank_build"), "tracks.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    
    {
        const auto mesh = new Mesh("base");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tank_build"), "base.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        const auto mesh = new Mesh("turret");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tank_build"), "turret.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        const auto mesh = new Mesh("gun");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tank_build"), "gun.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        const auto mesh = new Mesh("eiffel");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "buildings"), "eiffel.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        const auto mesh = new Mesh("pisa");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "buildings"), "pisa.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        const auto mesh = new Mesh("aps");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        auto *shader = new Shader("TankShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "WorldOfTanks", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "WorldOfTanks", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    ally_.init(0, 0.35f, -2);

    camera_ = new tank_camera::camera;
    camera_->set(glm::vec3(ally_.position.x, ally_.position.y + 2.f, ally_.position.z + 4.f), glm::vec3(0, 0.35f, -2), glm::vec3(0, 1, 0));
    projection_matrix_ = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);

    for (int i = 0; i < 9; ++i) {
        enemy enemy;
        enemy.init();
        enemies_.push_back(enemy);
    }

    for (int i = 0; i < 20; ++i)
    {
        constexpr float min_distance_from_tank = 35.0f;
        constexpr float min_distance_between_buildings = 15.0f;
        aps aps;
        do
        {
            aps.init();
        } while (glm::distance(aps.position, ally_.position) < min_distance_from_tank ||
                 is_too_close_to_other_buildings(aps, aps_, min_distance_between_buildings) ||
                 is_enemy_too_close_to_a_building(aps, enemies_, min_distance_between_buildings));
        aps_.push_back(aps);
    }

    const glm::ivec2 resolution = window->GetResolution();
    mini_viewport_area_ = viewport_area(50, 50, resolution.x / 5, resolution.y / 5);
}

void game::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
    
    // Render the mini map
    /*glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(mini_viewport_area_.x, mini_viewport_area_.y, mini_viewport_area_.width, mini_viewport_area_.height);*/
}

void game::Update(const float delta_time_seconds)
{
    time_ -= delta_time_seconds;
    cout << time_ << '\n';
    if (time_ <= 0.f)
    {
        cout << "You won!" << '\n';
        cout << "Your score is: " << score_ << '\n';
        exit(0);  // NOLINT(concurrency-mt-unsafe)
    }
    
    render_ground();
    render_ally();
    ally_.last_shot_time -= delta_time_seconds;

    if (ally_.health <= 0.f)
    {
        cout << "You lost!" << '\n';
        
       exit(0);  // NOLINT(concurrency-mt-unsafe)
    }

    for (auto& enemy : enemies_)
    {
        if (enemy.health > 0.f) enemy.update(delta_time_seconds);
        handle_enemy_ally_collision(enemy);

        if (enemy.distance_to_ally(ally_) && enemy.health > 0)
        {
            enemy.rotate_turret_and_gun(ally_);
            enemy.shoot_projectile();
        }

        if (enemy.health <= 0.f) score_ += 1;

        for (auto& bullet : enemy.bullets)
        {
            auto model_matrix = glm::mat4(1);
            model_matrix = translate(model_matrix, bullet.position);
            model_matrix = scale(model_matrix, glm::vec3(0.1f, 0.1f, 0.1f));
            render_simple_mesh(meshes["sphere"], shaders["TankShader"], model_matrix, glm::vec3(64.f / 255.f, 64.f / 255.f, 64.f / 255.f));
        }
        
        enemy.update_projectiles(delta_time_seconds, ally_, aps_);
    }

    render_enemy();

    auto model_matrix = glm::mat4(1);
    model_matrix = translate(model_matrix, glm::vec3(0, 0.35f, -20));
    model_matrix = scale(model_matrix, glm::vec3(10.f, 10.f, 10.f));
    render_simple_mesh(meshes["eiffel"], shaders["TankShader"], model_matrix, glm::vec3(40.f / 255.f, 2.f / 255.f, 2.f / 255.f));
    model_matrix = glm::mat4(1);
    model_matrix = translate(model_matrix, glm::vec3(0, -0.2f, 20));
    model_matrix = scale(model_matrix, glm::vec3(0.3f, 0.3f, 0.3f));
    render_simple_mesh(meshes["pisa"], shaders["TankShader"], model_matrix, glm::vec3(1, 229.f / 255.f, 204.f / 255.f));
    
    generate_random_buildings();

    for (size_t i = 0; i < enemies_.size(); ++i)
    {
        for (size_t j = i + 1; j < enemies_.size(); ++j)
        {
            handle_enemy_enemy_collision(enemies_[i], enemies_[j]);
        }
    }

    for (auto& aps : aps_)
    {
        handle_ally_collision_with_buildings(aps);

        for (auto& enemy : enemies_)
        {
            handle_enemy_collision_with_buildings(aps, enemy);
        }
    }

    update_ally_projectiles(delta_time_seconds);

    for (auto& projectile : ally_.bullets)
    {
        model_matrix = glm::mat4(1);
        model_matrix = translate(model_matrix, projectile.position);
        model_matrix = scale(model_matrix, glm::vec3(0.1f, 0.1f, 0.1f));
        render_simple_mesh(meshes["sphere"], shaders["TankShader"], model_matrix, glm::vec3(64.f / 255.f, 64.f / 255.f, 64.f / 255.f));
    }
}

void game::FrameEnd()
{
    /*DrawCoordinateSystem(camera_->get_view_matrix(), projection_matrix_);*/
}

void game::render_simple_mesh(const Mesh *mesh, const Shader *shader, const glm::mat4 & model_matrix, const glm::vec3 &color, const float hp) const
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    // Get shader location for uniform mat4 "Model"
    const int location_model = glGetUniformLocation(shader->program, "Model");

    // Set shader uniform "Model" to modelMatrix
    glUniformMatrix4fv(location_model, 1, GL_FALSE, value_ptr(model_matrix));

    // Get shader location for uniform mat4 "View"
    const int location_view = glGetUniformLocation(shader->program, "View");

    // Set shader uniform "View" to viewMatrix
    glUniformMatrix4fv(location_view, 1, GL_FALSE, value_ptr(camera_->get_view_matrix()));

    // Get shader location for uniform mat4 "Projection"
    const int location_projection = glGetUniformLocation(shader->program, "Projection");
    
    // Set shader uniform "Projection" to projectionMatrix
    glUniformMatrix4fv(location_projection, 1, GL_FALSE, value_ptr(projection_matrix_));

    const int loc_object_color = glGetUniformLocation(shader->program, "Color");
    glUniform3fv(loc_object_color, 1, value_ptr(color));

    glUseProgram(shader->program);
    const int loc_hp = glGetUniformLocation(shader->program, "v_hp");
    glUniform1f(loc_hp, hp);

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, nullptr);
    
    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, nullptr);
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */

void game::OnInputUpdate(const float delta_time, int mods)
{
    if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        constexpr float speed = 10;
        constexpr float rotate_speed = 100;

        // Control tank position using on W, A, S, D
        if (window->KeyHold(GLFW_KEY_W))
        {
            const glm::vec3 dir = normalize(glm::vec3(ally_.forward.x, 0, ally_.forward.z));
            ally_.position += dir * delta_time * speed;
            camera_->move_forward(delta_time * speed);
        }
        if (window->KeyHold(GLFW_KEY_A))
        {
            ally_.forward = rotate(ally_.forward, RADIANS(delta_time * rotate_speed), glm::vec3(0, 1, 0));
            ally_.tank_rotation_angle += delta_time * rotate_speed;
            camera_->rotate_third_person_oy(RADIANS(delta_time * rotate_speed));
        }
        if (window->KeyHold(GLFW_KEY_S))
        {
            const glm::vec3 dir = normalize(glm::vec3(ally_.forward.x, 0, ally_.forward.z));
            ally_.position -= dir * delta_time * speed;
            camera_->move_forward(-delta_time * speed);
        }
        if (window->KeyHold(GLFW_KEY_D))
        {
            ally_.forward = rotate(ally_.forward, -RADIANS(delta_time * rotate_speed), glm::vec3(0, 1, 0));
            ally_.tank_rotation_angle -= delta_time * rotate_speed;
            camera_->rotate_third_person_oy(-RADIANS(delta_time * rotate_speed));
        }
    }
}

void game::OnKeyPress(const int key, int mods) { }

void game::OnKeyRelease(const int key, int mods) { }

void game::OnMouseMove(int mouse_x, int mouse_y, const int delta_x, const int delta_y)
{
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        camera_->rotate_third_person_ox(-0.001f * static_cast<float>(delta_y));
        camera_->rotate_third_person_oy(-0.001f * static_cast<float>(delta_x));
    } else
    {
        rotate_ally_turret_and_gun(static_cast<float>(delta_x), static_cast<float>(delta_y));
    }
}

void game::OnMouseBtnPress(int mouse_x, int mouse_y, const int button, int mods)
{
    if (button == 1) ally_shoot_projectile();
}

void game::OnMouseBtnRelease(int mouse_x, int mouse_y, const int button, int mods)
{
    if (button == 2)
    {
        glm::vec3 pos = normalize(glm::vec3(ally_.forward.x, 0, ally_.forward.z)) * 4.f;
        pos.y -= 2.f;
        camera_->set(glm::vec3(ally_.position.x - pos.x, ally_.position.y - pos.y, ally_.position.z - pos.z), glm::vec3(ally_.position.x, ally_.position.y, ally_.position.z), glm::vec3(0, 1, 0));
    }
}

void game::OnMouseScroll(int mouse_x, int mouse_y, int offset_x, int offset_y) { }

void game::OnWindowResize(int width, int height) { }

void game::render_ally()
{
    auto model_matrix = glm::mat4(1);
    model_matrix = translate(model_matrix, glm::vec3(ally_.position.x, ally_.position.y, ally_.position.z));
    model_matrix = rotate(model_matrix, RADIANS(ally_.tank_rotation_angle), glm::vec3(0, 1, 0));
    render_simple_mesh(meshes["tracks"], shaders["TankShader"], model_matrix, glm::vec3(32.f/255.f, 32.f/255.f, 32.f/255.f), ally_.health);
    render_simple_mesh(meshes["base"], shaders["TankShader"], model_matrix, glm::vec3(1, 204.f/255.f, 153.f/255.f), ally_.health);
    model_matrix = glm::mat4(1);
    model_matrix = translate(model_matrix, glm::vec3(ally_.position.x, ally_.position.y, ally_.position.z));
    model_matrix = rotate(model_matrix, RADIANS(ally_.tank_rotation_angle), glm::vec3(0, 1, 0));
    model_matrix = rotate(model_matrix, RADIANS(ally_.turret_rotation_angle), glm::vec3(0, 1, 0));
    render_simple_mesh(meshes["turret"], shaders["TankShader"], model_matrix, glm::vec3(1, 178.f/255.f, 102.f/255.f), ally_.health);
    model_matrix = glm::mat4(1);
    model_matrix = translate(model_matrix, glm::vec3(ally_.position.x, ally_.position.y, ally_.position.z));
    /*
    model_matrix = rotate(model_matrix, RADIANS(-ally_.gun_rotation_angle_x), glm::vec3(1, 0, 0));
    */
    model_matrix = rotate(model_matrix, RADIANS(ally_.tank_rotation_angle), glm::vec3(0, 1, 0));
    model_matrix = rotate(model_matrix, RADIANS(ally_.gun_rotation_angle_y), glm::vec3(0, 1, 0));
    render_simple_mesh(meshes["gun"], shaders["TankShader"], model_matrix, glm::vec3(32.f/255.f, 32.f/255.f, 32.f/255.f), ally_.health);
}

void game::render_ground()
{
    auto model_matrix = glm::mat4(1);
    model_matrix = translate(model_matrix, glm::vec3(0, 0, 0));
    model_matrix = scale(model_matrix, glm::vec3(100, 0, 100));
    render_simple_mesh(meshes["plane"], shaders["TankShader"], model_matrix, glm::vec3(0, 1, 0));
}

void game::rotate_ally_turret_and_gun(const float delta_x, const float delta_y)
{
        constexpr float turret_rotation_speed = 0.3f;
        // Turret rotation (left and right)
        ally_.turret_rotation_angle -= delta_x * turret_rotation_speed;
        ally_.gun_rotation_angle_y -= delta_x * turret_rotation_speed;

        /*constexpr float gun_rotation_speed = 0.07f;
        ally_.gun_rotation_angle_x += delta_y * gun_rotation_speed;
        ally_.gun_rotation_angle_x = glm::clamp(ally_.gun_rotation_angle_x, -45.0f, 0.0f);*/
}

void game::render_enemy()
{
    for (const auto &enemy : enemies_)
    {
        auto model_matrix = glm::mat4(1);
        model_matrix = translate(model_matrix, enemy.position);
        model_matrix = rotate(model_matrix, RADIANS(enemy.tank_rotation_angle), glm::vec3(0, 1, 0));
        render_simple_mesh(meshes["tracks"], shaders["TankShader"], model_matrix, glm::vec3(204.f/255.f, 204.f/255.f, 1), enemy.health);
        render_simple_mesh(meshes["base"], shaders["TankShader"], model_matrix, glm::vec3(102.f/255.f, 0, 0), enemy.health);
        model_matrix = glm::mat4(1);
        model_matrix = translate(model_matrix, enemy.position);
        model_matrix = rotate(model_matrix, RADIANS(enemy.tank_rotation_angle), glm::vec3(0, 1, 0));
        model_matrix = rotate(model_matrix, RADIANS(enemy.turret_rotation_angle), glm::vec3(0, 1, 0));
        render_simple_mesh(meshes["turret"], shaders["TankShader"], model_matrix, glm::vec3(102.f/255.f, 0, 0), enemy.health);
        model_matrix = glm::mat4(1);
        model_matrix = translate(model_matrix, enemy.position);
        model_matrix = rotate(model_matrix, RADIANS(enemy.tank_rotation_angle), glm::vec3(0, 1, 0));
        model_matrix = rotate(model_matrix, RADIANS(enemy.turret_rotation_angle), glm::vec3(0, 1, 0));
        render_simple_mesh(meshes["gun"], shaders["TankShader"], model_matrix, glm::vec3(204.f/255.f, 204.f/255.f, 1), enemy.health);
    }
}

void game::generate_random_buildings()
{
    for (const auto &aps : aps_)
    {
        auto model_matrix = glm::mat4(1);
        model_matrix = translate(model_matrix, aps.position);
        model_matrix = scale(model_matrix, aps.scale);
        render_simple_mesh(meshes["aps"], shaders["TankShader"], model_matrix, aps.color);
    }
}

bool game::is_too_close_to_other_buildings(const aps& new_building, const std::vector<aps>& existing_buildings, const float min_distance)
{
    return std::any_of(existing_buildings.begin(), existing_buildings.end(), [&](const aps& existing_building) {
        return glm::distance(new_building.position, existing_building.position) < min_distance;
    });
}

bool game::is_enemy_too_close_to_a_building(const aps& building, const std::vector<enemy>& enemies, const float min_distance)
{
    return std::any_of(enemies.begin(), enemies.end(), [&](const enemy& enemy) {
        return distance(building.position, enemy.position) < min_distance;
    });
}

void game::handle_enemy_ally_collision(enemy& enemy)
{
    constexpr float tank_radius = 1.0f;
    const float distance = glm::distance(glm::vec2(ally_.position.x, ally_.position.z), glm::vec2(enemy.position.x, enemy.position.z));
    constexpr float sum_of_radius = 2 * tank_radius;

    if (distance < sum_of_radius)
    {
        const glm::vec3 displacement = normalize(enemy.position - ally_.position) * (sum_of_radius - distance);
        ally_.position += displacement * -0.5f;
        enemy.position += displacement * 0.5f;

        glm::vec3 pos = normalize(glm::vec3(ally_.forward.x, 0, ally_.forward.z)) * 4.f;
        pos.y -= 2.f;
        camera_->set(glm::vec3(ally_.position.x - pos.x, ally_.position.y - pos.y, ally_.position.z - pos.z), glm::vec3(ally_.position.x, ally_.position.y, ally_.position.z), glm::vec3(0, 1, 0));
    }
}

void game::handle_enemy_enemy_collision(enemy& enemy1, enemy& enemy2)
{
    constexpr float tank_radius = 1.0f;
    const float distance = glm::distance(glm::vec2(enemy1.position.x, enemy1.position.z), glm::vec2(enemy2.position.x, enemy2.position.z));
    constexpr float sum_of_radius = 2 * tank_radius;

    if (distance < sum_of_radius)
    {
        const glm::vec3 displacement = normalize(enemy2.position - enemy1.position) * (sum_of_radius - distance);
        enemy1.position += displacement * -0.5f;
        enemy2.position += displacement * 0.5f;
    }
}

void game::handle_ally_collision_with_buildings(aps& building)
{
    constexpr float tank_radius = 1.0f;
    constexpr float aps_radius = 0.6f;
    const float scaled_radius = aps_radius * std::max({building.scale.x, building.scale.y, building.scale.z});
    const float distance = glm::distance(glm::vec2(ally_.position.x, ally_.position.z), glm::vec2(building.position.x, building.position.z));
    const float sum_of_radius = tank_radius + scaled_radius;

    if (distance < sum_of_radius)
    {
        const glm::vec3 displacement = normalize(ally_.position - building.position) * (sum_of_radius - distance);
        ally_.position += displacement * 0.5f;
        ally_.position.y = 0.35f;

        glm::vec3 pos = normalize(glm::vec3(ally_.forward.x, 0, ally_.forward.z)) * 4.f;
        pos.y -= 2.f;
        camera_->set(glm::vec3(ally_.position.x - pos.x, ally_.position.y - pos.y, ally_.position.z - pos.z), glm::vec3(ally_.position.x, ally_.position.y, ally_.position.z), glm::vec3(0, 1, 0));
    }
}

void game::handle_enemy_collision_with_buildings(aps& building, enemy& enemy)
{
    constexpr float tank_radius = 1.0f;
    constexpr float aps_radius = 0.6f;
    const float scaled_radius = aps_radius * std::max({building.scale.x, building.scale.y, building.scale.z});
    const float distance = glm::distance(glm::vec2(enemy.position.x, enemy.position.z), glm::vec2(building.position.x, building.position.z));
    const float sum_of_radius = tank_radius + scaled_radius;

    if (distance < sum_of_radius)
    {
        const glm::vec3 displacement = normalize(enemy.position - building.position) * (sum_of_radius - distance);
        enemy.position += displacement * 0.5f;
        enemy.position.y = 0.35f;
    }
}

void game::ally_shoot_projectile()
{
    if (ally_.last_shot_time <= 0.f)
    {
        constexpr auto turret_gun_offset = glm::vec3(0.0f, 0.6f, -1.1f);

        const glm::vec3 turret_gun_position = ally_.position + rotate(turret_gun_offset,
            RADIANS(ally_.turret_rotation_angle)
            + RADIANS(ally_.tank_rotation_angle), glm::vec3(0, 1, 0));
        const glm::vec3 gun_direction = rotate(ally_.forward,
            RADIANS(ally_.turret_rotation_angle), glm::vec3(0, 1, 0));
        
        projectiles projectile;
        projectile.init(turret_gun_position, gun_direction);
        ally_.bullets.push_back(projectile);

        ally_.last_shot_time = 1.f;
    }
}

void game::update_ally_projectiles(const float delta_time_seconds)
{
    for (auto& projectile : ally_.bullets)
    {
        projectile.update(delta_time_seconds);
        constexpr float projectile_radius = 0.1f;

        for (auto& enemy : enemies_)
        {
            constexpr float tank_radius = 1.0f;
            const float distance = glm::distance(glm::vec2(projectile.position.x, projectile.position.z), glm::vec2(enemy.position.x, enemy.position.z));
            constexpr float sum_of_radius = tank_radius + projectile_radius;

            if (distance < sum_of_radius)
            {
                enemy.health -= 1;
                projectile.time = -1;
            }
        }

        for (auto& aps : aps_)
        {
            constexpr float aps_radius = 0.6f;
            const float scaled_radius = aps_radius * std::max({aps.scale.x, aps.scale.y, aps.scale.z});
            const float distance = glm::distance(glm::vec2(projectile.position.x, projectile.position.z), glm::vec2(aps.position.x, aps.position.z));
            const float sum_of_radius = scaled_radius + projectile_radius;

            if (distance < sum_of_radius)
            {
                projectile.time = -1;
            }
        }
    }

    ally_.bullets.erase(std::remove_if(
            ally_.bullets.begin(), ally_.bullets.end(),
            [](const projectiles& projectile) { return projectile.time <= 0.0f; }),
            ally_.bullets.end());
}

