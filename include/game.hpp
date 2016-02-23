#pragma once

#include "board.hpp"
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>

class Game
{
public:
    Game(int width, int height);
    ~Game();

    void run();

private:
    SDL_Window* p_window_ = nullptr;
    bool is_running_ = false;

    Board board_;

    int score = 0;
    Position player_pos_;
    Tile* player_tile_;

    GLuint base_program_ = 0u;
    GLuint path_program_ = 0u;
    GLuint tile_vao_ = 0u;
    GLuint tile_vbo_ = 0u;
    GLuint path_vao_ = 0u;
    GLuint path_vbo_ = 0u;
    GLuint path_offsets_ [POS_LAST][POS_LAST][2];
    glm::mat4 view_;

    void setupShaders();
    void destroyShaders();
    void setupMeshes();
    void destroyMeshes();
    void setupBoard();
    void processInput();
    void drawBoard();
};
