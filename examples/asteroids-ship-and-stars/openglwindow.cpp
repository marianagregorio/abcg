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

  // Create program to render the other objects
  m_objectsProgram = createProgramFromFile(getAssetsPath() + "objects.vert",
                                           getAssetsPath() + "objects.frag");

  // Create program to render the stars
  m_starsProgram = createProgramFromFile(getAssetsPath() + "stars.vert",
                                         getAssetsPath() + "stars.frag");

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

  m_dots.initializeGL(m_starsProgram, 10);
  m_asteroids.initializeGL(m_objectsProgram, 3);
  m_player.initializeGL(m_starsProgram, 1);
}

void OpenGLWindow::update() {
  float deltaTime{static_cast<float>(getDeltaTime())};

  // Wait 5 seconds before restarting
  if (m_gameData.m_state != State::Playing &&
      m_restartWaitTimer.elapsed() > 5) {
    restart();
    return;
  }

  m_dots.update(m_gameData, deltaTime);
  m_player.update(m_gameData, deltaTime);
  m_asteroids.update(deltaTime);

  if (m_gameData.m_state == State::Playing) {
    checkCollisions();
    // checkWinCondition();
  }
}

void OpenGLWindow::paintGL() {
  update();

  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  // m_asteroids.paintGL();
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
    ImGui::Text("Score: %d", m_gameData.m_score);

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
  glDeleteProgram(m_starsProgram);
  glDeleteProgram(m_objectsProgram);

  m_asteroids.terminateGL();
  m_dots.terminateGL();
  m_player.terminateGL();
}

void OpenGLWindow::checkCollisions() {
  // Check collision between ship and asteroids
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
        // fmt::print("Score: " + m_gameData.m_score);
        m_gameData.m_score = 0;
        m_gameData.m_state = State::GameOver;
        m_restartWaitTimer.restart();
      }
    } else if (m_dots.m_starLayers.at(0).m_hit.at(index) == 1) {
      m_dots.m_starLayers.at(0).m_hit.at(index) = 2;
    }
  }

  // Check collision between bullets and asteroids
  // for (auto &bullet : m_bullets.m_bullets) {
  //   if (bullet.m_dead) continue;

  //   for (auto &asteroid : m_asteroids.m_asteroids) {
  //     for (auto i : {-2, 0, 2}) {
  //       for (auto j : {-2, 0, 2}) {
  //         auto asteroidTranslation{asteroid.m_translation + glm::vec2(i, j)};
  //         auto distance{
  //             glm::distance(bullet.m_translation, asteroidTranslation)};

  //         if (distance < m_bullets.m_scale + asteroid.m_scale * 0.85f) {
  //           asteroid.m_hit = true;
  //           bullet.m_dead = true;
  //         }
  //       }
  //     }
  //   }

  //   m_asteroids.m_asteroids.remove_if(
  //       [](const Asteroids::Asteroid &a) { return a.m_hit; });
  // }
}

// void OpenGLWindow::checkCollisions() {
//   // Check collision between ship and asteroids
//   for (auto &asteroid : m_asteroids.m_asteroids) {
//     auto asteroidTranslation{asteroid.m_translation};
//     auto distance{glm::distance(m_player.m_player.m_translation,
//     asteroidTranslation)};

//     if (distance < asteroid.m_scale * 0.85f) {
//       m_gameData.m_state = State::GameOver;
//       m_restartWaitTimer.restart();
//     }
//   }
// }