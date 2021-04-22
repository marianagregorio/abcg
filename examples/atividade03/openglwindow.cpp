#include "openglwindow.hpp"

#include <fmt/core.h>
#include <imgui.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

// Custom specialization of std::hash injected in namespace std
namespace std {
template <>
struct hash<Vertex> {
  size_t operator()(Vertex const& vertex) const noexcept {
    std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
    return h1;
  }
};
}  // namespace std

void OpenGLWindow::handleEvent(SDL_Event& ev) {
  if (ev.type == SDL_KEYDOWN) {
    if (ev.key.keysym.sym == SDLK_UP || ev.key.keysym.sym == SDLK_w)
      m_dollySpeed = 1.0f;
    if (ev.key.keysym.sym == SDLK_DOWN || ev.key.keysym.sym == SDLK_s)
      m_dollySpeed = -1.0f;
    if (ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_a)
      m_panSpeed = 1.0f;
    if (ev.key.keysym.sym == SDLK_RIGHT || ev.key.keysym.sym == SDLK_d)
      m_panSpeed = -1.0f;
    if (ev.key.keysym.sym == SDLK_q) m_truckSpeed = -1.0f;
    if (ev.key.keysym.sym == SDLK_e) m_truckSpeed = 1.0f;
  }
  if (ev.type == SDL_KEYUP) {
    if ((ev.key.keysym.sym == SDLK_UP || ev.key.keysym.sym == SDLK_w) &&
        m_dollySpeed > 0)
      m_dollySpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_DOWN || ev.key.keysym.sym == SDLK_s) &&
        m_dollySpeed < 0)
      m_dollySpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_a) &&
        m_panSpeed > 0)
      m_panSpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_RIGHT || ev.key.keysym.sym == SDLK_d) &&
        m_panSpeed < 0)
      m_panSpeed = 0.0f;
    if (ev.key.keysym.sym == SDLK_q && m_truckSpeed < 0) m_truckSpeed = 0.0f;
    if (ev.key.keysym.sym == SDLK_e && m_truckSpeed > 0) m_truckSpeed = 0.0f;
  }
}

void OpenGLWindow::initializeGL() {
  glClearColor(0, 0, 0, 1);

  // Enable depth buffering
  glEnable(GL_DEPTH_TEST);

  // Create program
  m_programBunny = createProgramFromFile(getAssetsPath() + "lookat.vert",
                                    getAssetsPath() + "lookat.frag");

  m_model.loadFromFile(getAssetsPath() + "bunny.obj");
  m_model.setupVAO(m_programBunny);


  m_modelTeapot.loadFromFile(getAssetsPath() + "teapot.obj");
  m_modelTeapot.setupVAO(m_programBunny);

  m_modelTRex.loadFromFile(getAssetsPath() + "trex.obj");
  m_modelTRex.setupVAO(m_programBunny);

  resizeGL(getWindowSettings().width, getWindowSettings().height);
}

void OpenGLWindow::paintGL() {
  update();

  // Clear color buffer and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  glUseProgram(m_programBunny);

  // Get location of uniform variables (could be precomputed)
  GLint viewMatrixLoc{glGetUniformLocation(m_programBunny, "viewMatrix")};
  GLint projMatrixLoc{glGetUniformLocation(m_programBunny, "projMatrix")};
  GLint modelMatrixLoc{glGetUniformLocation(m_programBunny, "modelMatrix")};
  GLint colorLoc{glGetUniformLocation(m_programBunny, "color")};

  // Set uniform variables for viewMatrix and projMatrix
  // These matrices are used for every scene object
  glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_camera.m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_camera.m_projMatrix[0][0]);

  // Draw white bunny
  glm::mat4 model{1.0f};
  model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.5f));

  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

  m_model.render(-1);

  // // Draw orange t-rex
  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.0f));
  model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(1.0f));

  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLoc, 1.0f, 0.5f, 0.0f, 1.0f);
  m_modelTRex.render(-1);

  // Draw extra bunny
  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(3.0f, 0.0f, 0.0f));
  model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.5f));

  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f);
  m_model.render(-1);

  // Draw red bunny
  model = glm::mat4(1.0);
  model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.1f));

  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLoc, 1.0f, 0.25f, 0.25f, 1.0f);

  m_model.render(-1);

  // Draw gray Teapot
  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(1.0f, 0.0f, 1.0f));
  // model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.25f));

  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLoc, 0.5f, 0.5f, 0.5f, 1.0f);
  m_modelTeapot.render(-1);

  glUseProgram(0);
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();
}

void OpenGLWindow::update() {
  float deltaTime{static_cast<float>(getDeltaTime())};

  // Update LookAt camera
  m_camera.dolly(m_dollySpeed * deltaTime);
  m_camera.truck(m_truckSpeed * deltaTime);
  m_camera.pan(m_panSpeed * deltaTime);
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;
  m_camera.computeProjectionMatrix(width, height);
}

void OpenGLWindow::terminateGL() {
  glDeleteProgram(m_programBunny);
}