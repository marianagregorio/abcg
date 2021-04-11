
#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include <imgui.h>

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
  GLuint m_program{};

  int m_viewportWidth{};
  int m_viewportHeight{};

  ImFont* m_font{};

  Model m_model;
  std::string m_fileName = "airplane.obj";
  int m_trianglesToDraw{};

  TrackBall m_trackBall;
  float m_zoom{};

  glm::mat4 m_modelMatrix{1.0f};
  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};

  std::array<float, 4> m_clearColor{0.906f, 0.0f, 0.918f, 1.00f};

  void update();
};

#endif