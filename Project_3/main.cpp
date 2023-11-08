/*
* Author: Allan Verwayne
* Assignment: Lunar Lander
* Date due: 2023-11-08, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
*/

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
const char  SPRITESHEET_FILEPATH[] = "/Users/allan_home/Documents/GitHub/Project-3-Lunar-Lander/Project_3/sprites/gundam.png",
            PILLAR_FILEPATH[]  = "/Users/allan_home/Documents/GitHub/Project-3-Lunar-Lander/Project_3/sprites/flame pillar.png",
            LANDING_FILEPATH[] = "/Users/allan_home/Documents/GitHub/Project-3-Lunar-Lander/Project_3/sprites/tile.png",
            TEXT_FILEPATH[]    = "/Users/allan_home/Documents/GitHub/Project-3-Lunar-Lander/Project_3/sprites/font1.png";

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL  = 0;
const GLint TEXTURE_BORDER   = 0;

// ————— VARIABLES ————— //
GameState game_state;

SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram shader_program;
glm::mat4 view_matrix, projection_matrix;

//text globals
GLuint font_texture_id;
const int FONTBANK_SIZE = 16;

float previous_ticks = 0.0f;
float time_accumulator = 0.0f;


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
    
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;
    
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    for (int i = 0; i < text.size(); i++) {
        int spritesheet_index = (int) text[i];
        float offset = (screen_size + spacing) * i;
        
        float u_coordinate = (float) (spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float) (spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

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
    display_window = SDL_CreateWindow("Lunar Lander A.V. edition",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    view_matrix       = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    shader_program.set_projection_matrix(projection_matrix);
    shader_program.set_view_matrix(view_matrix);

    glUseProgram(shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    font_texture_id = load_texture(TEXT_FILEPATH);

    //player
    game_state.player = new Entity();
    game_state.player->e_type = PLAYER;
//    game_state.player->set_height(0.5f);
//    game_state.player->set_width(0.5f);
    game_state.player->set_position(glm::vec3(-3.0f, 3.0f, 0.0f));
    game_state.player->set_movement(glm::vec3(0.0f));
    game_state.player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY * 0.05f, 0.0f));
    game_state.player->set_speed(1.0f);
    game_state.player->texture_id = load_texture(SPRITESHEET_FILEPATH);

    //pillar
    game_state.pillar = new Entity[NUM_PILLARS];

    for (int i = 0; i < NUM_PILLARS; i++) {
        game_state.pillar[i].set_height(2.0f);
        game_state.pillar[i].set_width(0.5f);
        game_state.pillar[i].e_type = PILLAR;
        game_state.pillar[i].texture_id = load_texture(PILLAR_FILEPATH);
        game_state.pillar[i].set_position(glm::vec3((2.0f * i) - 4.0f, -3.0f + i, 0.0f));
        game_state.pillar[i].update(0.0f, NULL, 0);
    }
    
    //landing
    game_state.landing = new Entity[NUM_LANDINGS];
    game_state.landing->e_type = LANDING;
    
    for (int i = 0; i < NUM_LANDINGS; i++) {
        game_state.landing[i].set_height(0.5f);
        game_state.landing[i].e_type = LANDING;
        game_state.landing[i].texture_id = load_texture(LANDING_FILEPATH);
        game_state.landing[i].set_position(glm::vec3((2.0f * i) - 1.0f, -3.5f, 0.0f));
        game_state.landing[i].update(0.0f, NULL, 0);
    }

    //window
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input() {
    game_state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
                //end game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        //quit the game with a keystroke
                        game_is_running = false;
                        break;
                        
                    default:
                        break;
                }
            default:
                break;
        }
    }
    
    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (game_state.player->get_active() == true) {
        if (key_state[SDL_SCANCODE_LEFT]) {
            game_state.player->set_acceleration(glm::vec3(-0.1, 0.0f, 0.0f));
        }
        else if (key_state[SDL_SCANCODE_RIGHT]) {
            game_state.player->set_acceleration(glm::vec3(0.1, 0.0f, 0.0f));
        }
        else if (key_state[SDL_SCANCODE_DOWN]) {
            game_state.player->set_acceleration(glm::vec3(0.0f,-0.1, 0.0f));
        }
        else if (key_state[SDL_SCANCODE_UP]) {
            game_state.player->set_acceleration(glm::vec3(0.0f, 0.1, 0.0f));
        }
        else {
            game_state.player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY * 0.01f, 0.0f));
        }
        
        //normalize speed
        if (glm::length(game_state.player->get_movement()) > 1.0f) {
            game_state.player->set_movement(glm::normalize(game_state.player->get_movement()));
        }
    }
    
}

void update() {
    //delta time
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;

    //fixed timestep
    delta_time += time_accumulator;

    if (delta_time < FIXED_TIMESTEP) {
        time_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        game_state.player->update(FIXED_TIMESTEP, game_state.pillar, NUM_PILLARS);
        game_state.player->update(FIXED_TIMESTEP, game_state.landing, NUM_LANDINGS);
        delta_time -= FIXED_TIMESTEP;
    }
    time_accumulator = delta_time;

    
    
}

void render() {
    //window
    glClear(GL_COLOR_BUFFER_BIT);

    //player
    game_state.player->render(&shader_program);

    //pillar
    for (int i = 0; i < NUM_PILLARS; i++) game_state.pillar[i].render(&shader_program);
    
    //landing
    for (int i = 0; i < NUM_LANDINGS; i++) game_state.landing[i].render(&shader_program);
    
    if (game_state.player->landed_win) {
        DrawText(&shader_program, font_texture_id, "MISSION SUCCESS!", 0.5f, 0.01f, glm::vec3(-4.0f, 0.0f, 0.0f));
    }
    else if (game_state.player->landed_loss){
        DrawText(&shader_program, font_texture_id, "MISSION FAILED.", 0.5f, 0.01f, glm::vec3(-4.0f, 0.0f, 0.0f));
    }

    //window
    SDL_GL_SwapWindow(display_window);
}

void shutdown() { SDL_Quit(); }

//game
int main(int argc, char* argv[])
{
    initialise();

    while (game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
