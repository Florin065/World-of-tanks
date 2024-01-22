#pragma once
#include <random>

class aps
{
public:
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 color;

    void init()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> position_distribution(-50.0f, 50.0f);
        std::uniform_real_distribution<float> scale_distribution(5.0f, 20.0f);
        std::uniform_real_distribution<float> color_distribution(0.0f, 1.0f);

        position = glm::vec3(position_distribution(gen), 0.0f, position_distribution(gen));
        scale = glm::vec3(scale_distribution(gen));
        color = glm::vec3(color_distribution(gen), color_distribution(gen), color_distribution(gen));
    }
};
