
enum Type {PLAYER, PILLAR, LANDING};

class Entity {
private:
    
    bool m_is_active = true;
    
    // ––––– PHYSICS (GRAVITY) ––––– //
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    // ————— TRANSFORMATIONS ————— //
    float     m_speed;
    glm::vec3 m_movement;
    glm::mat4 m_model_matrix;

    float m_width = 1.0f;
    float m_height = 1.0f;

public:
    // ––––– PHYSICS (COLLISIONS) ––––– //
    bool m_collided_top    = false;
    bool m_collided_bottom = false;
    bool m_collided_left   = false;
    bool m_collided_right  = false;
    bool landed_win = false;
    bool landed_loss = false;

    GLuint    m_texture_id;
    Type      e_type;

    // ————— METHODS ————— //
    Entity();
    ~Entity();
    
    bool const check_collision(Entity* other) const;
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);

    void update(float delta_time, Entity* collidable_entities, int collidable_entity_count);
    void render(ShaderProgram* program);
    
    void activate()   { m_is_active = true; };
    void deactivate() { m_is_active = false; };

    // ————— GETTERS ————— //
    glm::vec3 const get_position()     const { return m_position; };
    glm::vec3 const get_velocity()     const { return m_velocity; };
    glm::vec3 const get_acceleration() const { return m_acceleration; };
    glm::vec3 const get_movement()     const { return m_movement; };
    float     const get_speed()        const { return m_speed; };
    int       const get_width()        const { return m_width; };
    int       const get_height()       const { return m_height; };
    bool      const get_active()       const { return m_is_active; };

    // ————— SETTERS ————— //
    void const set_position(glm::vec3 new_position)     { m_position = new_position; };
    void const set_velocity(glm::vec3 new_velocity)     { m_velocity = new_velocity; };
    void const set_acceleration(glm::vec3 new_position) { m_acceleration = new_position; };
    void const set_movement(glm::vec3 new_movement)     { m_movement = new_movement; };
    void const set_speed(float new_speed)               { m_speed = new_speed; };
    void const set_width(float new_width)               { m_width = new_width; };
    void const set_height(float new_height)             { m_height = new_height; };
};
