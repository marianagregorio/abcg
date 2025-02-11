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
  glClearColor(0, 0.08, 0, 1);

  // Enable depth buffering
  glEnable(GL_DEPTH_TEST);

  // Create program
  m_programPhong = createProgramFromFile(getAssetsPath() + "phong.vert",
                                         getAssetsPath() + "phong.frag");

  m_programNormal = createProgramFromFile(getAssetsPath() + "normal.vert",
                                          getAssetsPath() + "normal.frag");

  m_programTexture = createProgramFromFile(getAssetsPath() + "texture.vert",
                                           getAssetsPath() + "texture.frag");

  m_modelHeart.loadDiffuseTexture(getAssetsPath() + "maps/redpattern.png");
  m_modelHeart.loadFromFile(getAssetsPath() + "12190_Heart_v1_L3.obj",
                            m_programTexture);

  m_modelFlyingSaucer.loadFromFile(
      getAssetsPath() + "11681_Flying_saucer_v1_L3.obj", m_programTexture);

  m_modelTree.loadFromFile(getAssetsPath() + "Tree2.obj", m_programNormal);

  m_modelBunny.loadFromFile(getAssetsPath() + "bunny.obj", m_programPhong);

  m_modelTeapot.loadFromFile(getAssetsPath() + "teapot.obj", m_programNormal);

  m_modelTRex.loadDiffuseTexture(getAssetsPath() + "maps/rainbow.png");
  m_modelTRex.loadFromFile(getAssetsPath() + "T-Rex Model.obj", m_programPhong);

  resizeGL(getWindowSettings().width, getWindowSettings().height);
}

void OpenGLWindow::paintGL() {
  glClearColor(m_camera.m_at.r * 0.3, m_camera.m_at.g * 0.3, m_camera.m_at.b * 0.3, 1);
  update();

  // Clear color buffer and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  glUseProgram(m_programPhong);
  paintPhongIlluminatedModels();

  glUseProgram(m_programTexture);
  paintModelsWithTexture();

  glUseProgram(m_programNormal);
  paintNormalModels();

  glUseProgram(0);
}

void OpenGLWindow::paintPhongIlluminatedModels() {
  // Get location of uniform variables (could be precomputed)
  GLint viewMatrixLoc{glGetUniformLocation(m_programPhong, "viewMatrix")};
  GLint projMatrixLoc{glGetUniformLocation(m_programPhong, "projMatrix")};
  GLint modelMatrixLoc{glGetUniformLocation(m_programPhong, "modelMatrix")};
  GLint colorLoc{glGetUniformLocation(m_programPhong, "color")};

  // Get location of uniform variables
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

  glm::vec4 ka{0.1f, 0.1f, 0.1f, 1.0f};
  glm::vec4 kd{0.0f, 1.0f, 0.0f, 1.0f};
  auto lightDirRotated{m_lightDir};
  glUniform4fv(lightDirLoc, 1, &lightDirRotated.x);
  glUniform1f(shininessLoc, 12.5f);
  glUniform4fv(IaLoc, 1, &m_Ia.x);
  glUniform4fv(IdLoc, 1, &m_Id.x);
  glUniform4fv(IsLoc, 1, &m_Is.x);
  glUniform4fv(KaLoc, 1, &ka.x);
  glUniform4fv(KdLoc, 1, &kd.x);
  glUniform4fv(KsLoc, 1, &m_Ks.x);

  // Draw green bunny
  glm::mat4 model{1.0f};
  model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.5f));

  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);

  auto modelViewMatrix{glm::mat3(m_camera.m_viewMatrix * model)};
  glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
  glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);
  glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f);

  m_modelBunny.render(-1);

  glUniform1f(shininessLoc, m_shininess);
  glUniform4fv(KaLoc, 1, &m_Ka.x);
  glUniform4fv(KdLoc, 1, &m_Kd.x);
  // Draw white bunny
  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(3.0f, 0.0f, 0.0f));
  model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.5f));

  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f);
  m_modelBunny.render(-1);

  kd = {1.0f, 0.0f, 0.5f, 1.0f};
  glUniform1f(shininessLoc, m_shininess);
  glUniform4fv(KdLoc, 1, &kd.x);

  // Draw pink bunny
  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(4.0f, 0.0f, 2.0f));
  model = glm::rotate(model, glm::radians(-120.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.6f));

  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f);
  m_modelBunny.render(-1);

  kd = {1.0f, 0.5f, 0.0f, 1.0f};
  glUniform1f(shininessLoc, m_shininess);
  glUniform4fv(KdLoc, 1, &kd.x);

  // Draw orange bunny
  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.3f));
  model = glm::rotate(model, glm::radians(-150.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.4f));

  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f);
  m_modelBunny.render(-1);
}

void OpenGLWindow::paintModelsWithTexture() {
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
  GLint colorLoc{glGetUniformLocation(m_programTexture, "color")};

  // Draw red heart
  glm::mat4 model{1.0f};
  model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 1, 0));
  model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
  model = glm::scale(model, glm::vec3(0.3f));

  glUniformMatrix4fv(modelMatrixLocTexture, 1, GL_FALSE, &model[0][0]);

  glUniformMatrix4fv(viewMatrixLocTexture, 1, GL_FALSE,
                     &m_camera.m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLocTexture, 1, GL_FALSE,
                     &m_camera.m_projMatrix[0][0]);
  glUniform1i(diffuseTexLocTexture, 0);
  glUniform1i(mappingModeLocTexture, 3);  // mesh
  glUniform4f(colorLoc, 1.0f, 0.25f, 0.25f, 1.0f);

  glUniform4fv(lightDirLocTexture, 1, &m_lightDir.x);
  glUniform4fv(IaLocTexture, 1, &m_Ia.x);
  glUniform4fv(IdLocTexture, 1, &m_Id.x);
  glUniform4fv(IsLocTexture, 1, &m_Is.x);

  auto modelViewMatrixTexture{glm::mat3(m_camera.m_viewMatrix * model)};
  glm::mat3 textureMatrix{glm::inverseTranspose(modelViewMatrixTexture)};
  glUniformMatrix3fv(normalMatrixLocTexture, 1, GL_FALSE, &textureMatrix[0][0]);

  auto ka = m_modelHeart.getKa();
  auto kd = m_modelHeart.getKd();
  auto ks = m_modelHeart.getKs();

  glUniform1f(shininessLocTexture, m_modelHeart.getShininess());
  glUniform4fv(KaLocTexture, 1, &ka.x);
  glUniform4fv(KdLocTexture, 1, &kd.x);
  glUniform4fv(KsLocTexture, 1, &ks.x);

  m_modelHeart.render(-1);
  // // Draw orange t-rex
  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.0f));
  model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(1.0f));
  glUniformMatrix4fv(modelMatrixLocTexture, 1, GL_FALSE, &model[0][0]);

  glUniformMatrix4fv(viewMatrixLocTexture, 1, GL_FALSE,
                     &m_camera.m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLocTexture, 1, GL_FALSE,
                     &m_camera.m_projMatrix[0][0]);
  glUniform1i(diffuseTexLocTexture, 0);
  glUniform1i(mappingModeLocTexture, 3);  // mesh
  glUniform4f(colorLoc, 1.0f, 0.25f, 0.25f, 1.0f);

  glUniform4fv(lightDirLocTexture, 1, &m_lightDir.x);
  glUniform4fv(IaLocTexture, 1, &m_Ia.x);
  glUniform4fv(IdLocTexture, 1, &m_Id.x);
  glUniform4fv(IsLocTexture, 1, &m_Is.x);

  modelViewMatrixTexture = glm::mat3(m_camera.m_viewMatrix * model);
  textureMatrix = glm::inverseTranspose(modelViewMatrixTexture);
  glUniformMatrix3fv(normalMatrixLocTexture, 1, GL_FALSE, &textureMatrix[0][0]);

  ka = m_modelTRex.getKa();
  kd = m_modelTRex.getKd();
  ks = m_modelTRex.getKs();

  glUniform1f(shininessLocTexture, m_modelTRex.getShininess());
  glUniform4fv(KaLocTexture, 1, &ka.x);
  glUniform4fv(KdLocTexture, 1, &kd.x);
  glUniform4fv(KsLocTexture, 1, &ks.x);

  glUniformMatrix4fv(modelMatrixLocTexture, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLoc, 1.0f, 0.5f, 0.0f, 1.0f);
  m_modelTRex.render(-1);

  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(1.0f, 0.8f, -2.0f));
  model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
  // model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(1.0f));
  glUniformMatrix4fv(modelMatrixLocTexture, 1, GL_FALSE, &model[0][0]);

  glUniformMatrix4fv(viewMatrixLocTexture, 1, GL_FALSE,
                     &m_camera.m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLocTexture, 1, GL_FALSE,
                     &m_camera.m_projMatrix[0][0]);
  glUniform1i(diffuseTexLocTexture, 0);
  glUniform1i(mappingModeLocTexture, 3);  // mesh
  glUniform4f(colorLoc, 1.0f, 0.25f, 0.25f, 1.0f);

  glUniform4fv(lightDirLocTexture, 1, &m_lightDir.x);
  glUniform4fv(IaLocTexture, 1, &m_Ia.x);
  glUniform4fv(IdLocTexture, 1, &m_Id.x);
  glUniform4fv(IsLocTexture, 1, &m_Is.x);

  modelViewMatrixTexture = glm::mat3(m_camera.m_viewMatrix * model);
  textureMatrix = glm::inverseTranspose(modelViewMatrixTexture);
  glUniformMatrix3fv(normalMatrixLocTexture, 1, GL_FALSE, &textureMatrix[0][0]);

  ka = m_modelFlyingSaucer.getKa();
  kd = m_modelFlyingSaucer.getKd();
  ks = m_modelFlyingSaucer.getKs();

  glUniform1f(shininessLocTexture, m_modelFlyingSaucer.getShininess());
  glUniform4fv(KaLocTexture, 1, &ka.x);
  glUniform4fv(KdLocTexture, 1, &kd.x);
  glUniform4fv(KsLocTexture, 1, &ks.x);

  glUniformMatrix4fv(modelMatrixLocTexture, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLoc, 1.0f, 0.5f, 0.0f, 1.0f);
  m_modelFlyingSaucer.render(-1);
}

void OpenGLWindow::paintNormalModels() {
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
  glm::mat4 model{1.0f};
  model = glm::translate(model, glm::vec3(1.0f, 0.0f, 1.0f));
  // model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.25f));

  glUniformMatrix4fv(modelMatrixLocNormal, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLocNormal, 0.5f, 0.5f, 0.5f, 1.0f);
  m_modelTeapot.render(-1);

  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(2.0f, 0.0f, -2.0f));
  model = glm::rotate(model, glm::radians(-210.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.6f));
  glUniformMatrix4fv(modelMatrixLocNormal, 1, GL_FALSE, &model[0][0]);
  m_modelTree.render(-1);

  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(-2.0f, 0.0f, -1.0f));
  model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.9f));
  glUniformMatrix4fv(modelMatrixLocNormal, 1, GL_FALSE, &model[0][0]);
  m_modelTree.render(-1);

  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.7f));
  model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.6f));
  glUniformMatrix4fv(modelMatrixLocNormal, 1, GL_FALSE, &model[0][0]);
  m_modelTree.render(-1);

  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(-1.5f, 0.0f, 2.7f));
  model = glm::scale(model, glm::vec3(0.6f));
  glUniformMatrix4fv(modelMatrixLocNormal, 1, GL_FALSE, &model[0][0]);
  m_modelTree.render(-1);
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();
  {
    ImGui::SetNextWindowPos(ImVec2(m_viewportWidth - 280, 0));
    ImGui::SetNextWindowSize(ImVec2(280, 85));
    auto windowFlags{ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs};
    ImGui::Begin("score", nullptr, windowFlags);

    ImGui::Text("Movimente a câmera com as setas");
    ImGui::Text("e com as teclas q, w, e, a, s, d");

    ImGui::End();
  }
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

void OpenGLWindow::terminateGL() { glDeleteProgram(m_programPhong); }