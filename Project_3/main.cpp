#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define ACC_OF_GRAVITY -9.81f
#define NUM_LANDINGS 5
#define NUM_PILLARS 6

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"

// ————— STRUCTS AND ENUMS —————//
struct GameState
{
    Entity* player;
    Entity* pillar;
    Entity* landing;
    Entity* result;
};

// ————— CONSTANTS ————— //
const int WINDOW_WIDTH = 640 * 1.5,
WINDOW_HEIGHT = 480 * 1.5;

const float BG_RED = 0.0f,
            BG_BLUE = 0.0f,
            BG_GREEN = 0.0f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const char  SPRITESHEET_FILEPATH[] = "/Users/allan_home/Documents/CS-3113/Homework/Project_3/Project_3/sprites/gundam.png",
            PILLAR_FILEPATH[]  = "/Users/allan_home/Documents/CS-3113/Homework/Project_3/Project_3/sprites/flame pillar.png",
            LANDING_FILEPATH[] = "/Users/allan_home/Documents/CS-3113/Homework/Project_3/Project_3/sprites/tile.png",
            TEXT_FILEPATH[]    = "/Users/allan_home/Documents/CS-3113/Homework/Project_3/Project_3/sprites/font1.png";

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL  = 0;
const GLint TEXTURE_BORDER   = 0;

// ————— VARIABLES ————— //
GameState g_game_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

//text globals
GLuint font_texture_id;
const int FONTBANK_SIZE = 16;

float g_previous_ticks = 0.0f;
float g_time_accumulator = 0.0f;


// ———— GENERAL FUNCTIONS ———— //
GLuint load_texture(const char* filepath) {
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL) {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}

void DrawText(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position) {
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int) text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;
        
        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float) (spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float) (spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
        });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
        });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
    
    program->set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());
    
    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
    
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int) (text.size() * 6));
    
    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void initialise() {
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Lunar Lander A.V. edition",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    font_texture_id = load_texture(TEXT_FILEPATH);

    // ————— PLAYER ————— //
    g_game_state.player = new Entity();
    g_game_state.player->e_type = PLAYER;
//    g_game_state.player->set_height(0.5f);
//    g_game_state.player->set_width(0.5f);
    g_game_state.player->set_position(glm::vec3(-3.0f, 3.0f, 0.0f));
    g_game_state.player->set_movement(glm::vec3(0.0f));
    g_game_state.player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY * 0.05f, 0.0f));
    g_game_state.player->set_speed(1.0f);
    g_game_state.player->m_texture_id = load_texture(SPRITESHEET_FILEPATH);

    // ————— PILLAR ————— //
    g_game_state.pillar = new Entity[NUM_PILLARS];

    for (int i = 0; i < NUM_PILLARS; i++) {
        g_game_state.pillar[i].set_height(2.0f);
        g_game_state.pillar[i].set_width(0.5f);
        g_game_state.pillar[i].e_type = PILLAR;
        g_game_state.pillar[i].m_texture_id = load_texture(PILLAR_FILEPATH);
        g_game_state.pillar[i].set_position(glm::vec3((2.0f * i) - 4.0f, -3.0f + i, 0.0f));
        g_game_state.pillar[i].update(0.0f, NULL, 0);
    }
    
    // ————— LANDING ————— //
    g_game_state.landing = new Entity[NUM_LANDINGS];
    g_game_state.landing->e_type = LANDING;
    
    for (int i = 0; i < NUM_LANDINGS; i++) {
        g_game_state.landing[i].set_height(0.5f);
        g_game_state.landing[i].e_type = LANDING;
        g_game_state.landing[i].m_texture_id = load_texture(LANDING_FILEPATH);
        g_game_state.landing[i].set_position(glm::vec3((2.0f * i) - 1.0f, -3.5f, 0.0f));
        g_game_state.landing[i].update(0.0f, NULL, 0);
    }

    // ————— GENERAL ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input() {
    //if nothing is pressed, we don't want to go anywhere
    g_game_state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
                //end game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        //quit the game with a keystroke
                        g_game_is_running = false;
                        break;
                        
                    default:
                        break;
                }
            default:
                break;
        }
    }
    
    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (g_game_state.player->get_active() == true) {
        if (key_state[SDL_SCANCODE_LEFT]) {
            g_game_state.player->set_acceleration(glm::vec3(-0.1, 0.0f, 0.0f));
        }
        else if (key_state[SDL_SCANCODE_RIGHT]) {
            g_game_state.player->set_acceleration(glm::vec3(0.1, 0.0f, 0.0f));
        }
        else if (key_state[SDL_SCANCODE_DOWN]) {
            g_game_state.player->set_acceleration(glm::vec3(0.0f,-0.1, 0.0f));
        }
        else if (key_state[SDL_SCANCODE_UP]) {
            g_game_state.player->set_acceleration(glm::vec3(0.0f, 0.1, 0.0f));
        }
        else {
            g_game_state.player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY * 0.01f, 0.0f));
        }
        
        // This makes sure that the player can't move faster diagonally
        if (glm::length(g_game_state.player->get_movement()) > 1.0f) {
            g_game_state.player->set_movement(glm::normalize(g_game_state.player->get_movement()));
        }
    }
    
}

void update() {
    // ————— DELTA TIME ————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;

    // ————— FIXED TIMESTEP ————— //
    //keep track of how much time has passed since last step
    delta_time += g_time_accumulator;

    //accumulate the ammount of time passed while we're under our fixed timestep
    if (delta_time < FIXED_TIMESTEP) {
        g_time_accumulator = delta_time;
        return;
    }

    //once we exceed our fixed timestep, apply that elapsed time into the objects' update function invocation
    while (delta_time >= FIXED_TIMESTEP) {
        //using FIXED_TIMESTEP as our delta time
        g_game_state.player->update(FIXED_TIMESTEP, g_game_state.pillar, NUM_PILLARS);
        g_game_state.player->update(FIXED_TIMESTEP, g_game_state.landing, NUM_LANDINGS);
        delta_time -= FIXED_TIMESTEP;
    }
    g_time_accumulator = delta_time;

    
    
}

void render() {
    // ————— GENERAL ————— //
    glClear(GL_COLOR_BUFFER_BIT);

    // ————— PLAYER ————— //
    g_game_state.player->render(&g_shader_program);

    // ————— PILLAR ————— //
    for (int i = 0; i < NUM_PILLARS; i++) g_game_state.pillar[i].render(&g_shader_program);
    
    // ————— LANDING ————— //
    for (int i = 0; i < NUM_LANDINGS; i++) g_game_state.landing[i].render(&g_shader_program);
    
    if (g_game_state.player->landed_win) {
        DrawText(&g_shader_program, font_texture_id, "MISSION SUCCESS!", 0.5f, 0.01f, glm::vec3(-4.0f, 0.0f, 0.0f));
    }
    else if (g_game_state.player->landed_loss){
        DrawText(&g_shader_program, font_texture_id, "MISSION FAILED.", 0.5f, 0.01f, glm::vec3(-4.0f, 0.0f, 0.0f));
    }

    // ————— GENERAL ————— //
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

// ————— DRIVER GAME LOOP ————— /
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
