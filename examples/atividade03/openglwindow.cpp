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

  // Create program
  m_programTeapot = createProgramFromFile(getAssetsPath() + "lookat.vert",
                                    getAssetsPath() + "lookat.frag");

  // Load model
  loadTeapotModelFromFile(getAssetsPath() + "teapot.obj");

  m_model.loadFromFile(getAssetsPath() + "bunny.obj");
  m_model.setupVAO(m_programBunny);

  glGenBuffers(1, &m_VBOTeapot);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBOTeapot);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_verticesTeapot[0]) * m_verticesTeapot.size(),
               m_verticesTeapot.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenBuffers(1, &m_EBOTeapot);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBOTeapot);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indicesTeapot[0]) * m_indicesTeapot.size(),
               m_indicesTeapot.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Create VAO
  glGenVertexArrays(1, &m_VAOTeapot);

    // Bind vertex attributes to current VAO
  glBindVertexArray(m_VAOTeapot);

  glBindBuffer(GL_ARRAY_BUFFER, m_VBOTeapot);
  GLint positionAttributeTeapot{glGetAttribLocation(m_programTeapot, "inPosition")};
  glEnableVertexAttribArray(positionAttributeTeapot);
  glVertexAttribPointer(positionAttributeTeapot, 3, GL_FLOAT, GL_FALSE,
                        sizeof(Vertex), nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBOTeapot);

  // End of binding to current VAO
  glBindVertexArray(0);

  resizeGL(getWindowSettings().width, getWindowSettings().height);
}

void OpenGLWindow::loadTeapotModelFromFile(std::string_view path) {
  tinyobj::ObjReaderConfig readerConfig;
  readerConfig.mtl_search_path =
      getAssetsPath() + "mtl/";  // Path to material files

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data(), readerConfig)) {
    if (!reader.Error().empty()) {
      throw abcg::Exception{abcg::Exception::Runtime(
          fmt::format("Failed to load model {} ({})", path, reader.Error()))};
    }
    throw abcg::Exception{
        abcg::Exception::Runtime(fmt::format("Failed to load model {}", path))};
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  const auto& attrib{reader.GetAttrib()};
  const auto& shapes{reader.GetShapes()};

  m_verticesTeapot.clear();
  m_indicesTeapot.clear();

  // A key:value map with key=Vertex and value=index
  std::unordered_map<Vertex, GLuint> hash{};

  // Loop over shapes
  for (const auto& shape : shapes) {
    // Loop over faces(polygon)
    size_t indexOffset{0};
    for (const auto faceNumber :
         iter::range(shape.mesh.num_face_vertices.size())) {
      // Number of vertices composing face f
      std::size_t numFaceVertices{shape.mesh.num_face_vertices[faceNumber]};
      // Loop over vertices in the face
      std::size_t startIndex{};
      for (const auto vertexNumber : iter::range(numFaceVertices)) {
        // Access to vertex
        tinyobj::index_t index{shape.mesh.indices[indexOffset + vertexNumber]};

        // Vertex coordinates
        startIndex = 3 * index.vertex_index;
        tinyobj::real_t vx = attrib.vertices[startIndex + 0];
        tinyobj::real_t vy = attrib.vertices[startIndex + 1];
        tinyobj::real_t vz = attrib.vertices[startIndex + 2];

        Vertex vertex{};
        vertex.position = {vx, vy, vz};

        // If hash doesn't contain this vertex
        if (hash.count(vertex) == 0) {
          // Add this index (size of vertices)
          hash[vertex] = m_verticesTeapot.size();
          // Add this vertex
          m_verticesTeapot.push_back(vertex);
        }

        m_indicesTeapot.push_back(hash[vertex]);
      }
      indexOffset += numFaceVertices;
    }
  }
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

  // // Draw yellow bunny
  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.0f));
  model = glm::scale(model, glm::vec3(1.0f));

  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLoc, 1.0f, 0.5f, 0.0f, 1.0f);
  m_model.render(-1);

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

  glUseProgram(0);

  // changes to teapot program

  glUseProgram(m_programTeapot);
  glBindVertexArray(m_VAOTeapot);

  // Get location of uniform variables (could be precomputed)
  GLint viewMatrixLocTeapot{glGetUniformLocation(m_programTeapot, "viewMatrix")};
  GLint projMatrixLocTeapot{glGetUniformLocation(m_programTeapot, "projMatrix")};
  GLint modelMatrixLocTeapot{glGetUniformLocation(m_programTeapot, "modelMatrix")};
  GLint colorLocTeapot{glGetUniformLocation(m_programTeapot, "color")};

  // Set uniform variables for viewMatrix and projMatrix
  // These matrices are used for every scene object
  glUniformMatrix4fv(viewMatrixLocTeapot, 1, GL_FALSE, &m_camera.m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLocTeapot, 1, GL_FALSE, &m_camera.m_projMatrix[0][0]);

  // Draw white Teapot
  // glm::mat4 model{1.0f};
  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(1.0f, 0.0f, 1.0f));
  // model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.005f));

  glUniformMatrix4fv(modelMatrixLocTeapot, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLocTeapot, 1.0f, 1.0f, 1.0f, 1.0f);
  glDrawElements(GL_TRIANGLES, m_indicesTeapot.size(), GL_UNSIGNED_INT, nullptr);

  glUniformMatrix4fv(modelMatrixLocTeapot, 1, GL_FALSE, &model[0][0]);
  glUniform4f(colorLocTeapot, 1.0f, 0.25f, 0.25f, 1.0f);
  glDrawElements(GL_TRIANGLES, m_indicesTeapot.size(), GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);
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