
#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include "abcg.hpp"
#include "model.hpp"
#include "trackball.hpp"

class OpenGLWindow : public abcg::OpenGLWindow {
 protected:
  void handleEvent(SDL_Event& ev) override;
  void initializeGL() override;
  void paintGL() override;
  void paintUI() override;
  void resizeGL(int width, int height) override;
  void terminateGL() override;

 private:
  std::vector<const char*> m_shaderNames{"normal", "depth"};
  std::vector<GLuint> m_programs;
  int m_currentProgramIndex{-1};

  int m_viewportWidth{};
  int m_viewportHeight{};

  Model m_model;
  int m_trianglesToDraw{};

  TrackBall m_trackBall;
  float m_zoom{};

  glm::mat4 m_modelMatrix{1.0f};
  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};

  void update();
};

#endif