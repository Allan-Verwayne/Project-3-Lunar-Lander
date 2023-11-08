#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"

Entity::Entity() {
    // ––––– PHYSICS ––––– //
    position = glm::vec3(0.0f);
    velocity = glm::vec3(0.0f);
    acceleration = glm::vec3(0.0f);

    // ––––– TRANSFORMATION ––––– //
    movement = glm::vec3(0.0f);
    speed = 0;
    model_matrix = glm::mat4(1.0f);
    
}

Entity::~Entity() {}

void Entity::update(float delta_time, Entity* collidable_entities, int collidable_entity_count) {
    
    if (!is_active) return;

    collided_top = false;
    collided_bottom = false;
    collided_left = false;
    collided_right = false;

    // ––––– GRAVITY ––––– //
    velocity += acceleration * delta_time; //starting by falling
    
    position.y += velocity.y * delta_time;
    check_collision_y(collidable_entities, collidable_entity_count);

    position.x += velocity.x * delta_time;
    check_collision_x(collidable_entities, collidable_entity_count);

    // ––––– TRANSFORMATIONS ––––– //
    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
    model_matrix = glm::scale(model_matrix, glm::vec3(width, height, 0.0f));
}

void Entity::render(ShaderProgram* program) {
    program->set_model_matrix(model_matrix);

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count) {
    for (int i = 0; i < collidable_entity_count; i++) {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity)) {
            float y_distance = fabs(position.y - collidable_entity->position.y);
            float y_overlap = fabs(y_distance - (height / 2.0f) - (collidable_entity->height / 2.0f));

            if (velocity.y > 0) {
                position.y -= y_overlap;
                velocity.y = 0;
                collided_top = true;
                deactivate();
                
            }
            else if (velocity.y < 0) {
                position.y += y_overlap;
                velocity.y = 0;
                collided_bottom = true;
                deactivate();
            }
            if (collidable_entity->e_type == LANDING) { landed_win = true; }
            else { landed_loss = true; }
        }
    }
}

void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count) {
    for (int i = 0; i < collidable_entity_count; i++) {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity)) {
            float x_distance = fabs(position.x - collidable_entity->position.x);
            float x_overlap = fabs(x_distance - (width / 2.0f) - (collidable_entity->width / 2.0f));
            if (velocity.x > 0) {
                position.x -= x_overlap;
                velocity.x = 0;
                collided_right = true;
                deactivate();
            }
            else if (velocity.x < 0) {
                position.x += x_overlap;
                velocity.x = 0;
                collided_left = true;
                deactivate();
            }
            if (collidable_entity->e_type == LANDING) { landed_win = true; }
            else { landed_loss = true; }
        }
    }
}

bool const Entity::check_collision(Entity* other) const {
    
    if (!is_active || !other->is_active) return false;
    float x_distance = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float y_distance = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    return (x_distance < 0.0f && y_distance < 0.0f) ? true : false;
}
