#ifndef SNAKE_HPP_
#define SNAKE_HPP_

#include "abcg.hpp"
#include "gamedata.hpp"

class OpenGLWindow;

class Snake {
 public:
  void initializeGL(GLuint program);
  void paintGL(const GameData &gameData);
  void terminateGL();

  void update(const GameData &gameData, float deltaTime);
  void setDirection(glm::vec2 direction) { m_direction = direction; }

 private:
  friend OpenGLWindow;

  GLuint m_program{};
  GLint m_translationLoc{};
  GLint m_colorLoc{};
  GLint m_scaleLoc{};
//   GLint m_rotationLoc{};

  GLuint m_vao{};
  GLuint m_vbo{};
  GLuint m_ebo{};

  glm::vec4 m_color{1};
  glm::vec2 m_direction{glm::vec2(0)};
//   float m_rotation{};
  float m_scale{0.125f};
  glm::vec2 m_translation{glm::vec2(0)};
  glm::vec2 m_velocity{glm::vec2(0)};

  abcg::ElapsedTimer m_trailBlinkTimer;
  abcg::ElapsedTimer m_bulletCoolDownTimer;
};

#endif