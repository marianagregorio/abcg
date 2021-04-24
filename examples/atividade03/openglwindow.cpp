#include "openglwindow.hpp"

#include <fmt/core.h>
#include <imgui.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

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
  glClearColor(0, 0.1, 0, 1);

  // Enable depth buffering
  glEnable(GL_DEPTH_TEST);

  // Create program
  m_programPhong = createProgramFromFile(getAssetsPath() + "phong.vert",
                                         getAssetsPath() + "phong.frag");

  m_programNormal = createProgramFromFile(getAssetsPath() + "normal.vert",
                                          getAssetsPath() + "normal.frag");

  m_programTexture = createProgramFromFile(getAssetsPath() + "texture.vert",
                                           getAssetsPath() + "texture.frag");

  loadModel(getAssetsPath() + "bunny.obj");
  // loadModel(getAssetsPath() + "teapot.obj");
  // loadModel(getAssetsPath() + "trex.obj");
  // m_model.loadFromFile(getAssetsPath() + "bunny.obj");
  // m_model.setupVAO(m_programPhong);

  m_modelTeapot.loadFromFile(getAssetsPath() + "teapot.obj");
  m_modelTeapot.setupVAO(m_programPhong);

  m_modelTRex.loadFromFile(getAssetsPath() + "trex.obj");
  m_modelTRex.setupVAO(m_programPhong);

  resizeGL(getWindowSettings().width, getWindowSettings().height);
}

void OpenGLWindow::loadModel(std::string_view path) {
  m_model.loadDiffuseTexture(getAssetsPath() + "maps/pattern.png");
  m_model.loadFromFile(path);
  m_model.setupVAO(m_programTexture);
  // m_trianglesToDraw = m_model.getNumTriangles();

  // Use material properties from the loaded model
  m_Ka = m_model.getKa();
  m_Kd = m_model.getKd();
  m_Ks = m_model.getKs();
  m_shininess = m_model.getShininess();
}

void OpenGLWindow::paintGL() {
  update();

  // Clear color buffer and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  glUseProgram(m_programPhong);

  // Get location of uniform variables (could be precomputed)
  GLint viewMatrixLoc{glGetUniformLocation(m_programPhong, "viewMatrix")};
  GLint projMatrixLoc{glGetUniformLocation(m_programPhong, "projMatrix")};
  GLint modelMatrixLoc{glGetUniformLocation(m_programPhong, "modelMatrix")};
  GLint colorLoc{glGetUniformLocation(m_programPhong, "color")};

  // Get location of uniform variables
  // GLint viewMatrixLoc{glGetUniformLocation(m_programPhong, "viewMatrix")};
  // GLint projMatrixLoc{glGetUniformLocation(m_programPhong, "projMatrix")};
  // GLint modelMatrixLoc{glGetUniformLocation(m_programPhong, "modelMatrix")};
  GLint normalMatrixLoc{glGetUniformLocation(m_programPhong, "normalMatrix")};
  GLint lightDirLoc{glGetUniformLocation(m_programPhong, "lightDirWorldSpace")};
  GLint shininessLoc{glGetUniformLocation(m_programPhong, "shininess")};
  GLint IaLoc{glGetUniformLocation(m_programPhong, "Ia")};
  GLint IdLoc{glGetUniformLocation(m_programPhong, "Id")};
  GLint IsLoc{glGetUniformLocation(m_programPhong, "Is")};
  GLint KaLoc{glGetUniformLocation(m_programPhong, "Ka")};
  GLint KdLoc{glGetUniformLocation(m_programPhong, "Kd")};
  GLint KsLoc{glGetUniformLocation(m_programPhong, "Ks")};

  // Set uniform variables for viewMatrix and projMatrix
  // These matrices are used for every scene object
  glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_camera.m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_camera.m_projMatrix[0][0]);
  // glUniform1i(diffuseTexLoc, 0);
  // glUniform1i(mappingModeLoc, 3); // mesh

  auto lightDirRotated{m_lightDir};
  glUniform4fv(lightDirLoc, 1, &lightDirRotated.x);
  glUniform1f(shininessLoc, m_shininess);
  glUniform4fv(IaLoc, 1, &m_Ia.x);
  glUniform4fv(IdLoc, 1, &m_Id.x);
  glUniform4fv(IsLoc, 1, &m_Is.x);
  glUniform4fv(KaLoc, 1, &m_Ka.x);
  glUniform4fv(KdLoc, 1, &m_Kd.x);
  glUniform4fv(KsLoc, 1, &m_Ks.x);

  // Draw white bunny
  glm::mat4 model{1.0f};
  model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.5f));

  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);

  auto modelViewMatrix{glm::mat3(m_camera.m_viewMatrix * model)};
  glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
  glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);
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

  glUseProgram(m_programTexture);

  GLint diffuseTexLocTexture{
      glGetUniformLocation(m_programTexture, "diffuseTex")};
  GLint mappingModeLocTexture{
      glGetUniformLocation(m_programTexture, "mappingMode")};
  GLint viewMatrixLocTexture{
      glGetUniformLocation(m_programTexture, "viewMatrix")};
  GLint projMatrixLocTexture{
      glGetUniformLocation(m_programTexture, "projMatrix")};
  GLint modelMatrixLocTexture{
      glGetUniformLocation(m_programTexture, "modelMatrix")};
  GLint normalMatrixLocTexture{
      glGetUniformLocation(m_programTexture, "normalMatrix")};
  GLint lightDirLocTexture{
      glGetUniformLocation(m_programTexture, "lightDirWorldSpace")};
  GLint shininessLocTexture{
      glGetUniformLocation(m_programTexture, "shininess")};
  GLint IaLocTexture{glGetUniformLocation(m_programTexture, "Ia")};
  GLint IdLocTexture{glGetUniformLocation(m_programTexture, "Id")};
  GLint IsLocTexture{glGetUniformLocation(m_programTexture, "Is")};
  GLint KaLocTexture{glGetUniformLocation(m_programTexture, "Ka")};
  GLint KdLocTexture{glGetUniformLocation(m_programTexture, "Kd")};
  GLint KsLocTexture{glGetUniformLocation(m_programTexture, "Ks")};
  // Draw red bunny
  model = glm::mat4(1.0);
  model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.3f));

  glUniformMatrix4fv(modelMatrixLocTexture, 1, GL_FALSE, &model[0][0]);


  glUniformMatrix4fv(viewMatrixLocTexture, 1, GL_FALSE, &m_camera.m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLocTexture, 1, GL_FALSE, &m_camera.m_projMatrix[0][0]);
  glUniform1i(diffuseTexLocTexture, 0);
  glUniform1i(mappingModeLocTexture, 3);  // mesh
  glUniform4f(colorLoc, 1.0f, 0.25f, 0.25f, 1.0f);

  glUniform4fv(lightDirLocTexture, 1, &lightDirRotated.x);
  glUniform4fv(IaLocTexture, 1, &m_Ia.x);
  glUniform4fv(IdLocTexture, 1, &m_Id.x);
  glUniform4fv(IsLocTexture, 1, &m_Is.x);

  auto modelViewMatrixTexture{glm::mat3(m_camera.m_viewMatrix * model)};
  glm::mat3 textureMatrix{glm::inverseTranspose(modelViewMatrixTexture)};
  glUniformMatrix3fv(normalMatrixLocTexture, 1, GL_FALSE, &textureMatrix[0][0]);

  glUniform1f(shininessLocTexture, m_shininess);
  glUniform4fv(KaLocTexture, 1, &m_Ka.x);
  glUniform4fv(KdLocTexture, 1, &m_Kd.x);
  glUniform4fv(KsLocTexture, 1, &m_Ks.x);

  m_model.render(-1);
  // glUseProgram(0);

  glUseProgram(m_programNormal);

  // Get location of uniform variables (could be precomputed)
  GLint viewMatrixLocNormal{
      glGetUniformLocation(m_programNormal, "viewMatrix")};
  GLint projMatrixLocNormal{
      glGetUniformLocation(m_programNormal, "projMatrix")};
  GLint modelMatrixLocNormal{
      glGetUniformLocation(m_programNormal, "modelMatrix")};
  GLint colorLocNormal{glGetUniformLocation(m_programNormal, "color")};

  // These matrices are used for every scene object
  glUniformMatrix4fv(viewMatrixLocNormal, 1, GL_FALSE,
                     &m_camera.m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLocNormal, 1, GL_FALSE,
                     &m_camera.m_projMatrix[0][0]);

  // Draw gray Teapot
  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(1.0f, 0.0f, 1.0f));
  // model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.25f));

  glUniformMatrix4fv(modelMatrixLocNormal, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLocNormal, 0.5f, 0.5f, 0.5f, 1.0f);
  m_modelTeapot.render(-1);

  glUseProgram(0);
}

void OpenGLWindow::paintUI() { abcg::OpenGLWindow::paintUI(); }

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

void OpenGLWindow::terminateGL() { glDeleteProgram(m_programPhong); }