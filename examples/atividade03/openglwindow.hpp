#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include "abcg.hpp"
#include "model.hpp"
#include "camera.hpp"

// struct Vertex {
//   glm::vec3 position;

//   bool operator==(const Vertex& other) const {
//     return position == other.position;
//   }
// };

class OpenGLWindow : public abcg::OpenGLWindow {
 protected:
  void handleEvent(SDL_Event& ev) override;
  void initializeGL() override;
  void paintGL() override;
  void paintUI() override;
  void resizeGL(int width, int height) override;
  void terminateGL() override;

 private:
  GLuint m_programBunny{};

  int m_viewportWidth{};
  int m_viewportHeight{};

  Model m_model;
  Model m_modelTeapot;
  Model m_modelTRex;

  Camera m_camera;
  float m_dollySpeed{0.0f};
  float m_truckSpeed{0.0f};
  float m_panSpeed{0.0f};

  void update();
};

#endif