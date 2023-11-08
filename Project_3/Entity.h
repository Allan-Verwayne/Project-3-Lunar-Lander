
enum Type {PLAYER, PILLAR, LANDING};

class Entity {
private:
    
    bool is_active = true;
    
    // ––––– PHYSICS (GRAVITY) ––––– //
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    // ————— TRANSFORMATIONS ————— //
    float     speed;
    glm::vec3 movement;
    glm::mat4 model_matrix;

    float width = 1.0f;
    float height = 1.0f;

public:
    // ––––– PHYSICS (COLLISIONS) ––––– //
    bool collided_top    = false;
    bool collided_bottom = false;
    bool collided_left   = false;
    bool collided_right  = false;
    bool landed_win = false;
    bool landed_loss = false;

    GLuint    texture_id;
    Type      e_type;

    // ————— METHODS ————— //
    Entity();
    ~Entity();
    
    bool const check_collision(Entity* other) const;
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);

    void update(float delta_time, Entity* collidable_entities, int collidable_entity_count);
    void render(ShaderProgram* program);
    
    void activate()   { is_active = true; };
    void deactivate() { is_active = false; };

    // ————— GETTERS ————— //
    glm::vec3 const get_position()     const { return position; };
    glm::vec3 const get_velocity()     const { return velocity; };
    glm::vec3 const get_acceleration() const { return acceleration; };
    glm::vec3 const get_movement()     const { return movement; };
    float     const get_speed()        const { return speed; };
    int       const get_width()        const { return width; };
    int       const get_height()       const { return height; };
    bool      const get_active()       const { return is_active; };

    // ————— SETTERS ————— //
    void const set_position(glm::vec3 new_position)     { position = new_position; };
    void const set_velocity(glm::vec3 new_velocity)     { velocity = new_velocity; };
    void const set_acceleration(glm::vec3 new_position) { acceleration = new_position; };
    void const set_movement(glm::vec3 new_movement)     { movement = new_movement; };
    void const set_speed(float new_speed)               { speed = new_speed; };
    void const set_width(float new_width)               { width = new_width; };
    void const set_height(float new_height)             { height = new_height; };
};
