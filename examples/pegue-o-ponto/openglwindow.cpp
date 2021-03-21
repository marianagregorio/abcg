#include "openglwindow.hpp"

#include <fmt/core.h>
#include <imgui.h>

#include <cppitertools/itertools.hpp>

#include "abcg.hpp"
void OpenGLWindow::handleEvent(SDL_Event &event) {
  // Keyboard events
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_SPACE)
      m_gameData.m_input.set(static_cast<size_t>(Input::Fire));
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
      m_gameData.m_input.set(static_cast<size_t>(Input::Up));
    if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
      m_gameData.m_input.set(static_cast<size_t>(Input::Down));
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.set(static_cast<size_t>(Input::Left));
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_gameData.m_input.set(static_cast<size_t>(Input::Right));
  }
  if (event.type == SDL_KEYUP) {
    if (event.key.keysym.sym == SDLK_SPACE)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Fire));
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Up));
    if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Down));
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Left));
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Right));
  }

  // Mouse events
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    if (event.button.button == SDL_BUTTON_LEFT)
      m_gameData.m_input.set(static_cast<size_t>(Input::Fire));
    if (event.button.button == SDL_BUTTON_RIGHT)
      m_gameData.m_input.set(static_cast<size_t>(Input::Up));
  }
  if (event.type == SDL_MOUSEBUTTONUP) {
    if (event.button.button == SDL_BUTTON_LEFT)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Fire));
    if (event.button.button == SDL_BUTTON_RIGHT)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Up));
  }
  if (event.type == SDL_MOUSEMOTION) {
    glm::ivec2 mousePosition;
    SDL_GetMouseState(&mousePosition.x, &mousePosition.y);

    glm::vec2 direction{glm::vec2{mousePosition.x - m_viewportWidth / 2,
                                  mousePosition.y - m_viewportHeight / 2}};
    direction.y = -direction.y;
  }
}

void OpenGLWindow::initializeGL() {
  // Load a new font
  ImGuiIO &io{ImGui::GetIO()};
  auto filename{getAssetsPath() + "Inconsolata-Medium.ttf"};
  m_font = io.Fonts->AddFontFromFileTTF(filename.c_str(), 60.0f);
  if (m_font == nullptr) {
    throw abcg::Exception{abcg::Exception::Runtime("Cannot load font file")};
  }

  // Create program to render the objects
  m_program = createProgramFromFile(getAssetsPath() + "object.vert",
                                         getAssetsPath() + "object.frag");

  glClearColor(0, 0, 0, 1);

#if !defined(__EMSCRIPTEN__)
  glEnable(GL_PROGRAM_POINT_SIZE);
#endif

  // Start pseudo-random number generator
  auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);

  restart();
}

void OpenGLWindow::restart() {
  m_gameData.m_state = State::Playing;

  m_dots.initializeGL(m_program, 10);
  m_player.initializeGL(m_program, 1);
}

void OpenGLWindow::update() {
  float deltaTime{static_cast<float>(getDeltaTime())};

  // Wait 5 seconds before restarting
  if (m_gameData.m_state != State::Playing &&
      m_restartWaitTimer.elapsed() > 5) {
    restart();
    return;
  }

  m_player.update(m_gameData, deltaTime);

  if (m_gameData.m_state == State::Playing) {
    checkCollisions();
  }
}

void OpenGLWindow::paintGL() {
  update();

  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  m_dots.paintGL();
  m_player.paintGL();
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();

  {
    auto widgetSize{ImVec2(300, 85)};
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(widgetSize);
    auto windowFlags{ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs};
    ImGui::Begin("score", nullptr, windowFlags);
    // ImGui::PushFont(m_font);
    ImGui::Text("Score: %ld", m_gameData.m_score);

    // ImGui::PopFont();

    ImGui::End();
    ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2.0f,
                                   (m_viewportHeight - widgetSize.y) / 2.0f));
    ImGui::SetNextWindowSize(widgetSize);
    // auto windowFlags{ImGuiWindowFlags_NoBackground |
    //                  ImGuiWindowFlags_NoTitleBar |
    //                  ImGuiWindowFlags_NoInputs};
    ImGui::Begin("game_state", nullptr, windowFlags);
    // ImGui::Text("Score: %d", m_gameData.m_score);
    ImGui::PushFont(m_font);

    if (m_gameData.m_state == State::GameOver) {
      ImGui::Text("Game Over!");
    } else if (m_gameData.m_state == State::Win) {
      ImGui::Text("*You Win!*");
    }

    ImGui::PopFont();

    ImGui::End();
  }
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLWindow::terminateGL() {
  // Release shader program, VBO and VAO
  glDeleteProgram(m_program);

  m_dots.terminateGL();
  m_player.terminateGL();
}

void OpenGLWindow::checkCollisions() {
  for (auto &&[index, dot] :
       iter::enumerate(m_dots.m_starLayers.at(0).m_positions)) {
    auto dotTranslation{dot};
    auto distance{
        glm::distance(m_player.m_player.m_translation, dotTranslation)};

    if (distance < 0.05) {
      if (m_dots.m_starLayers.at(0).m_hit.at(index) == 0) {
        m_gameData.m_score += 1;
        m_dots.m_starLayers.at(0).m_hit.at(index) = 1;
        if (m_gameData.m_score == m_dots.m_starLayers.at(0).m_hit.size()) {
          m_gameData.m_score = 0;
          m_gameData.m_state = State::Win;
          m_restartWaitTimer.restart();
        }
      } else if (m_dots.m_starLayers.at(0).m_hit.at(index) == 2) {
        m_gameData.m_score = 0;
        m_gameData.m_state = State::GameOver;
        m_restartWaitTimer.restart();
      }
    } else if (m_dots.m_starLayers.at(0).m_hit.at(index) == 1) {
      m_dots.m_starLayers.at(0).m_hit.at(index) = 2;
    }
  }
}