#include "player.hpp"

#include <cppitertools/itertools.hpp>
#include <fmt/core.h>

void PlayerLayer::initializeGL(GLuint program, int quantity) {
  terminateGL();

  // Start pseudo-random number generator
  auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);

  m_program = program;
  m_pointSizeLoc = glGetUniformLocation(m_program, "pointSize");
  m_translationLoc = glGetUniformLocation(m_program, "translation");

  auto &re{m_randomEngine};
  std::uniform_real_distribution<float> distPos(-1.0f, 1.0f);
  std::uniform_real_distribution<float> distIntensity(0.5f, 1.0f);

  // for (auto &&[index, layer] : iter::enumerate(m_starLayers)) {
  m_player.m_pointSize = 30.0f / (1.0f);
  m_player.m_quantity = quantity * (static_cast<int>(1));
  float d1 = 0; //distPos(re);
  float d2 = 0; //distPos(re);
  m_player.m_translation = glm::vec2{d1, d2};

  std::vector<glm::vec3> data(0);

  data.emplace_back(d1, d2, 0);
  data.push_back(glm::vec3(1));

  // TODO: isso aqui pra gerar os outros gomos da cobra (é nessa função mesmo?)
  for ([[maybe_unused]] auto i : iter::range(1, m_player.m_quantity)) {
    data.emplace_back(d1 + 0.05f, d2 + 0.0f, 0);
    data.push_back(glm::vec3(1) * distIntensity(re));
  }

  // Generate VBO
  glGenBuffers(1, &m_player.m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_player.m_vbo);
  glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec3), data.data(),
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{glGetAttribLocation(m_program, "inPosition")};
  GLint colorAttribute{glGetAttribLocation(m_program, "inColor")};

  // Create VAO
  glGenVertexArrays(1, &m_player.m_vao);

  // Bind vertex attributes to current VAO
  glBindVertexArray(m_player.m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, m_player.m_vbo);
  glEnableVertexAttribArray(positionAttribute);
  glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE,
                        sizeof(glm::vec3) * 2, nullptr);
  glEnableVertexAttribArray(colorAttribute);
  glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE,
                        sizeof(glm::vec3) * 2,
                        reinterpret_cast<void *>(sizeof(glm::vec3)));
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  glBindVertexArray(0);
  // }
}

void PlayerLayer::paintGL() {
  glUseProgram(m_program);

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);

  // for (auto &layer : m_starLayers) {
  glBindVertexArray(m_player.m_vao);
  glUniform1f(m_pointSizeLoc, m_player.m_pointSize);

  for (auto i : {-2, 0, 2}) {
    for (auto j : {-2, 0, 2}) {
      glUniform2f(m_translationLoc, m_player.m_translation.x + j,
                  m_player.m_translation.y + i);

      glDrawArrays(GL_POINTS, 0, m_player.m_quantity);
    }
  }

  glBindVertexArray(0);
  // }

  glDisable(GL_BLEND);

  glUseProgram(0);
}

void PlayerLayer::terminateGL() {
  // for (auto &layer : m_starLayers) {
  glDeleteBuffers(1, &m_player.m_vbo);
  glDeleteVertexArrays(1, &m_player.m_vao);
  // }
}

void PlayerLayer::update(const GameData &gameData, float deltaTime) {
  glm::vec2 direction;
  if (gameData.m_input[static_cast<size_t>(Input::Left)])
    direction = glm::vec2{-1.0f, 0.0f};

  if (gameData.m_input[static_cast<size_t>(Input::Right)])
    direction = glm::vec2{1.0f, 0.0f};

  if (gameData.m_input[static_cast<size_t>(Input::Up)])
    direction = glm::vec2{0.0f, 1.0f};

  if (gameData.m_input[static_cast<size_t>(Input::Down)])
    direction = glm::vec2{0.0f, -1.0f};
  auto layerSpeedScale{1.0f / (0 + 2.0f)};
  m_player.m_translation += direction * deltaTime * layerSpeedScale;

  // fmt::print(m_player.m_translation);
  // Wrap-around
  if (m_player.m_translation.x < -1.0f) m_player.m_translation.x += 2.0f;
  if (m_player.m_translation.x > +1.0f) m_player.m_translation.x -= 2.0f;
  if (m_player.m_translation.y < -1.0f) m_player.m_translation.y += 2.0f;
  if (m_player.m_translation.y > +1.0f) m_player.m_translation.y -= 2.0f;
  // }
}