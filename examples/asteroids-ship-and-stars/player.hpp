#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include <array>
#include <random>

#include "abcg.hpp"
#include "gamedata.hpp"

class OpenGLWindow;

class PlayerLayer {
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

  struct Player {
    GLuint m_vao{};
    GLuint m_vbo{};

    float m_pointSize{};
    int m_quantity{};
    glm::vec2 m_translation{glm::vec2(0)};
  };

  Player m_player;

  std::default_random_engine m_randomEngine;
};

#endif