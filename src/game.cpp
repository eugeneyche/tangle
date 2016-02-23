#include "error.hpp"
#include "game.hpp"
#include "shader.hpp"
#include "bezier.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <SDL2/SDL_opengl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const std::string GAME_TITLE = "Tangle";
static const int BOARD_WIDTH = 9;
static const int BOARD_HEIGHT = 9;

static const float SQRT3_OVER_2 = 0.866025;
static const float BOARD_SCALE = 32.0f;

static int next_tile = 0;
static Tile tile_pool [BOARD_WIDTH * BOARD_HEIGHT];

Tile* AllocTile()
{ 
    Tile* p_tile = &tile_pool[next_tile++];
    p_tile->clearPaths();
    return p_tile;
}

static const glm::vec2 Vec2Lerp(const glm::vec2 src, const glm::vec2& dest, float alpha)
{
    return src * (1.f - alpha) + dest * alpha;
}

static const std::vector<glm::vec2> TILE_VERTICES = {
    glm::vec2 {1.f, 0.f},
    glm::vec2 {0.5f, SQRT3_OVER_2}, 
    glm::vec2 {-0.5f, SQRT3_OVER_2}, 
    glm::vec2 {-1.f, 0.f},
    glm::vec2 {-0.5f, -SQRT3_OVER_2}, 
    glm::vec2 {0.5f, -SQRT3_OVER_2}
};

static const std::vector<glm::vec2> TILE_NORMALS = {
    glm::vec2 {SQRT3_OVER_2, 0.5f},
    glm::vec2 {0.f, 1.f},
    glm::vec2 {-SQRT3_OVER_2, 0.5f},
    glm::vec2 {-SQRT3_OVER_2, -0.5f},
    glm::vec2 {0.f, -1.f},
    glm::vec2 {SQRT3_OVER_2, -0.5f}
};

static const std::vector<glm::vec2> TILE_POSITIONS = {
    Vec2Lerp(TILE_VERTICES[0], TILE_VERTICES[1], 0.3f),
    Vec2Lerp(TILE_VERTICES[0], TILE_VERTICES[1], 0.7f),
    Vec2Lerp(TILE_VERTICES[1], TILE_VERTICES[2], 0.3f),
    Vec2Lerp(TILE_VERTICES[1], TILE_VERTICES[2], 0.7f),
    Vec2Lerp(TILE_VERTICES[2], TILE_VERTICES[3], 0.3f),
    Vec2Lerp(TILE_VERTICES[2], TILE_VERTICES[3], 0.7f),
    Vec2Lerp(TILE_VERTICES[3], TILE_VERTICES[4], 0.3f),
    Vec2Lerp(TILE_VERTICES[3], TILE_VERTICES[4], 0.7f),
    Vec2Lerp(TILE_VERTICES[4], TILE_VERTICES[5], 0.3f),
    Vec2Lerp(TILE_VERTICES[4], TILE_VERTICES[5], 0.7f),
    Vec2Lerp(TILE_VERTICES[5], TILE_VERTICES[0], 0.3f),
    Vec2Lerp(TILE_VERTICES[5], TILE_VERTICES[0], 0.7f)
};

Game::Game(int width, int height)
    : board_ (BOARD_WIDTH, BOARD_HEIGHT)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        FatalError("Failed to initialize SDL.");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    p_window_ = SDL_CreateWindow(
            GAME_TITLE.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_OPENGL);
    if (!p_window_)
        FatalError("Failed to create SDL window.");
    SDL_GLContext context = SDL_GL_CreateContext(p_window_);
    if (!context)
        FatalError("Failed to initialize SDL context.");
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
        FatalError("Failed to initialize GLEW.");
    glGetError();
    int half_width = width / 2;
    int half_height = height / 2;
    view_ = glm::ortho(
            static_cast<float>(-half_width), 
            static_cast<float>(width - half_width), 
            static_cast<float>(-half_height),
            static_cast<float>(height - half_height),
            -1.0f, 1.0f);
    glViewport(0, 0, width, height);
}

Game::~Game()
{
    SDL_DestroyWindow(p_window_);
    SDL_Quit();
}

void Game::run()
{
    setupShaders();
    setupMeshes();
    setupBoard();
    is_running_ = true;
    while (is_running_)
    {
        if (GLint error = glGetError())
            std::cerr << "GL Error (" << error << ")" << std::endl;
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        processInput();
        drawBoard();
        SDL_GL_SwapWindow(p_window_);
        if (!player_tile_) 
        {
            std::cout << "Out of Bounds." << std::endl;
            is_running_ = false;
        }
        else if (player_tile_->isPathTaken(player_pos_))
        {
            std::cout << "No more paths." << std::endl;
            is_running_ = false;
        }
    }
    std::cout << "Final Score: " << score << std::endl;
    destroyMeshes();
    destroyShaders();
}

void Game::setupShaders()
{
    std::vector<GLuint> base_shaders = {
        LoadShader(GL_VERTEX_SHADER, "shaders/base.vert"),
        LoadShader(GL_FRAGMENT_SHADER, "shaders/base.frag")
    };
    base_program_ = LoadProgram(base_shaders);
    for (GLuint shader : base_shaders)
    {
        glDeleteShader(shader);
    }
    std::vector<GLuint> path_shaders = {
        LoadShader(GL_VERTEX_SHADER, "shaders/path.vert"),
        LoadShader(GL_FRAGMENT_SHADER, "shaders/path.frag")
    };
    path_program_ = LoadProgram(path_shaders);
    for (GLuint shader : path_shaders)
    {
        glDeleteShader(shader);
    }
}

void Game::destroyShaders()
{
    if (base_program_)
        glDeleteProgram(base_program_);
    if (path_program_)
        glDeleteProgram(path_program_);
}

void Game::setupMeshes()
{
    glGenVertexArrays(1, &tile_vao_);
    glGenBuffers(1, &tile_vbo_);
    std::vector<GLfloat> tile_vertex_buffer = {
        0.f, 0.f,
        TILE_VERTICES[0].x, TILE_VERTICES[0].y,
        TILE_VERTICES[1].x, TILE_VERTICES[1].y,
        TILE_VERTICES[2].x, TILE_VERTICES[2].y,
        TILE_VERTICES[3].x, TILE_VERTICES[3].y,
        TILE_VERTICES[4].x, TILE_VERTICES[4].y,
        TILE_VERTICES[5].x, TILE_VERTICES[5].y,
        TILE_VERTICES[0].x, TILE_VERTICES[0].y
    };
    glBindVertexArray(tile_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, tile_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * tile_vertex_buffer.size(), tile_vertex_buffer.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0u);

    std::vector<GLfloat> path_vertex_buffer;
    std::vector<glm::vec2> path_position_buffer;
    std::vector<GLfloat> path_alpha_buffer;
    int current_offset = 0u;
    for (int i = 0; i < POS_LAST; i++)
    {
        for (int j = i + 1; j < POS_LAST; j++)
        {
            path_position_buffer.clear();
            path_alpha_buffer.clear();
            int begin_offset = current_offset;
            glm::vec2 p0 = TILE_POSITIONS[i];
            glm::vec2 p3 = TILE_POSITIONS[j];
            glm::vec2 p1 = p0 - 0.3f * TILE_NORMALS[i / 2];
            glm::vec2 p2 = p3 - 0.3f * TILE_NORMALS[j / 2];
            GenBezierCurve({p0, p1, p2, p3}, path_position_buffer, path_alpha_buffer);
            current_offset += path_position_buffer.size();
            for (size_t k = 0; k < path_position_buffer.size(); k++)
            {
                path_vertex_buffer.push_back(path_position_buffer[k].x);
                path_vertex_buffer.push_back(path_position_buffer[k].y);
                path_vertex_buffer.push_back(path_alpha_buffer[k]);
            }
            int end_offset = current_offset;
            path_offsets_[i][j][0] = begin_offset;
            path_offsets_[i][j][1] = end_offset;
        }
    }
    glGenVertexArrays(1, &path_vao_);
    glGenBuffers(1, &path_vbo_);
    glBindVertexArray(path_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, path_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * path_vertex_buffer.size(), path_vertex_buffer.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<void*>(2 * sizeof(GLfloat)));
    glBindVertexArray(0u);
}

void Game::destroyMeshes()
{
    if (tile_vao_)
        glDeleteVertexArrays(1, &tile_vao_);
    if (tile_vbo_)
        glDeleteBuffers(1, &tile_vbo_);
}

void Game::setupBoard()
{
    for (int i = 0; i < BOARD_WIDTH; i++)
    {
        for (int j = 0; j < BOARD_HEIGHT; j++)
        {
            if (i + j <= 3)
                continue;
            if (BOARD_WIDTH + BOARD_HEIGHT - i - j - 2 <= 3)
                continue;
            Tile* p_tile = AllocTile();
            p_tile->randomlyGeneratePaths();
            board_.setTile(p_tile, i, j);
        }
    }
    player_pos_ = static_cast<Position>(0);
    player_tile_ = board_.getTile(BOARD_HEIGHT / 2, BOARD_WIDTH / 2);
}

void Game::processInput()
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
        switch(ev.type)
        {
        case SDL_QUIT:
            is_running_ = false;
            break;
        case SDL_KEYDOWN:
            if (ev.key.keysym.sym == SDLK_SPACE)
            {
                player_pos_ = board_.getTile(player_tile_->getI(), player_tile_->getJ())->traverse(player_pos_);
                player_tile_ = board_.getTileInAdjacentPosition(player_pos_, player_tile_->getI(), player_tile_->getJ());
                score++;
            }
            if (ev.key.keysym.sym == SDLK_LEFT)
            {
                if (player_tile_->canRotate())
                    player_tile_->rotateLeft();
            }
            if (ev.key.keysym.sym == SDLK_RIGHT)
            {
                if (player_tile_->canRotate())
                    player_tile_->rotateRight();
            }
        }
    }
}

void Game::drawBoard()
{
    GLint world_view_loc = -1;
    float half_width = (board_.getWidth() - 1) / 2.f;
    float half_height = (board_.getHeight() - 1) / 2.f;
    glUseProgram(base_program_);
    glBindVertexArray(tile_vao_);
    world_view_loc = glGetUniformLocation(base_program_, "world_view");
    for (int i = 0; i < board_.getHeight(); i++)
    {
        for (int j = 0; j < board_.getWidth(); j++)
        {
            Tile* p_tile = board_.getTile(i, j);
            if (p_tile)
            {
                float x = (j - half_width) * 1.5;
                float y = ((half_height - i) * 2 - (j - half_width)) * SQRT3_OVER_2;
                x *= 1.1f;
                y *= 1.1f;
                glm::mat4 world_view = glm::translate(glm::scale(view_, glm::vec3(BOARD_SCALE, BOARD_SCALE, 1.f)), glm::vec3(x, y, 0.f));
                glUniformMatrix4fv(world_view_loc, 1, GL_FALSE, glm::value_ptr(world_view));
                glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
            }
        }
    }
    glUseProgram(path_program_);
    glBindVertexArray(path_vao_);
    world_view_loc = glGetUniformLocation(path_program_, "world_view");
    GLint is_taken_loc = glGetUniformLocation(path_program_, "is_taken");
    for (int i = 0; i < board_.getHeight(); i++)
    {
        for (int j = 0; j < board_.getWidth(); j++)
        {
            Tile* p_tile = board_.getTile(i, j);
            if (p_tile)
            {
                float x = (j - half_width) * 1.5;
                float y = ((half_height - i) * 2 - (j - half_width)) * SQRT3_OVER_2;
                x *= 1.1f;
                y *= 1.1f;
                glm::mat4 world_view = glm::rotate(glm::translate(glm::scale(view_, glm::vec3(BOARD_SCALE, BOARD_SCALE, 1.f)), glm::vec3(x, y, 0.f)), glm::pi<float>() / 3 * p_tile->getOrientation(), glm::vec3(0, 0, 1));
                glUniformMatrix4fv(world_view_loc, 1, GL_FALSE, glm::value_ptr(world_view));
                for (Path path : p_tile->getPaths())
                {
                    glUniform1i(is_taken_loc, path.taken);
                    GLint offset_begin = path_offsets_[path.begin][path.end][0];
                    GLint offset_end = path_offsets_[path.begin][path.end][1];
                    GLint count = offset_end - offset_begin;
                    glDrawArrays(GL_LINE_STRIP, offset_begin, count);
                }
            }
        }
    }
}
