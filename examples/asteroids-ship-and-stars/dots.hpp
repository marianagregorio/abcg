#ifndef DOTS_HPP_
#define DOTS_HPP_

#include <array>
#include <random>

#include "abcg.hpp"
#include "gamedata.hpp"

class OpenGLWindow;

class Dots {
 public:
  void initializeGL(GLuint program, int quantity);
  void paintGL();
  void terminateGL();

  void update(const GameData &gameData, float deltaTime);

 private:
  friend OpenGLWindow;

  GLuint m_program{};
  GLint m_pointSizeLoc{};
  GLint m_translationLoc{};

  struct Dot {
    GLuint m_vao{};
    GLuint m_vbo{};

    float m_pointSize{};
    int m_quantity{};
    std::array<glm::vec2, 10> m_positions{};
    std::array<bool, 10> m_hit{};
    std::array<glm::vec2, 10> m_timer{};
    glm::vec2 m_translation{glm::vec2(0)};
  };

  std::array<Dot, 1> m_starLayers;

  std::default_random_engine m_randomEngine;
};

#endif