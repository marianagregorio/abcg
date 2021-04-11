#include "openglwindow.hpp"

#include <gsl/gsl>
#include <imgui.h>

#include <cppitertools/itertools.hpp>

void OpenGLWindow::handleEvent(SDL_Event& event) {
  glm::ivec2 mousePosition;
  SDL_GetMouseState(&mousePosition.x, &mousePosition.y);

  if (event.type == SDL_MOUSEMOTION) {
    m_trackBall.mouseMove(mousePosition);
  }
  if (event.type == SDL_MOUSEBUTTONDOWN &&
      event.button.button == SDL_BUTTON_LEFT) {
    m_trackBall.mousePress(mousePosition);
  }
  if (event.type == SDL_MOUSEBUTTONUP &&
      event.button.button == SDL_BUTTON_LEFT) {
    m_trackBall.mouseRelease(mousePosition);
  }
  if (event.type == SDL_MOUSEWHEEL) {
    m_zoom += (event.wheel.y > 0 ? 1.0f : -1.0f) / 5.0f;
    m_zoom = glm::clamp(m_zoom, -1.5f, 1.0f);
  }
}

void OpenGLWindow::initializeGL() {
  ImGuiIO& io{ImGui::GetIO()};
  auto filename{getAssetsPath() + "Inconsolata-Medium.ttf"};
  m_font = io.Fonts->AddFontFromFileTTF(filename.c_str(), 36.0f);
  if (m_font == nullptr) {
    throw abcg::Exception{abcg::Exception::Runtime("Cannot load font file")};
  }

  glClearColor(0, 80, 0, 1);

  // Enable depth buffering
  glEnable(GL_DEPTH_TEST);

  // Create program
  m_program = createProgramFromFile(getAssetsPath() + "depth.vert",
                                    getAssetsPath() + "depth.frag");

  // Load model
  m_model.loadFromFile(getAssetsPath() + m_fileName);
  // m_model.loadFromFile(getAssetsPath() + "bunny.obj");

  m_model.setupVAO(m_program);

  m_trianglesToDraw = m_model.getNumTriangles();
}

void OpenGLWindow::paintGL() {
  update();

  // Clear color buffer and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  glUseProgram(m_program);

  // Get location of uniform variables (could be precomputed)
  GLint viewMatrixLoc{glGetUniformLocation(m_program, "viewMatrix")};
  GLint projMatrixLoc{glGetUniformLocation(m_program, "projMatrix")};
  GLint modelMatrixLoc{glGetUniformLocation(m_program, "modelMatrix")};
  GLint colorLoc{glGetUniformLocation(m_program, "color")};

  // Set uniform variables used by every scene object
  glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);

  // Set uniform variables of the current object
  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_modelMatrix[0][0]);
  glUniform4f(colorLoc, gsl::at(m_clearColor, 0), gsl::at(m_clearColor, 1),
               gsl::at(m_clearColor, 2), gsl::at(m_clearColor, 3));

  m_model.render(m_trianglesToDraw);

  glUseProgram(0);
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();

  // Create window for slider
  {
    ImGui::SetNextWindowPos(ImVec2(5, m_viewportHeight - 94));
    ImGui::SetNextWindowSize(ImVec2(m_viewportWidth - 10, -1));
    ImGui::Begin("Slider window", nullptr, ImGuiWindowFlags_NoDecoration);

    // Create a slider to control the number of rendered triangles
    {
      // Slider will fill the space of the window
      ImGui::PushItemWidth(m_viewportWidth - 25);

      ImGui::SliderInt("", &m_trianglesToDraw, 0, m_model.getNumTriangles(),
                       "%d triangles");

      ImGui::PopItemWidth();
    }

    ImGui::End();
  }

  // Combo para selecionar modelo
  {
    auto widgetSize{ImVec2(340, 40)};
    ImGui::SetNextWindowPos(ImVec2(m_viewportWidth - widgetSize.x - 5, 5));
    ImGui::SetNextWindowSize(widgetSize);
    ImGui::Begin("Widget window", nullptr, ImGuiWindowFlags_NoDecoration);
    {
      // Edit background color
      static std::size_t currentComboIndex{};
      bool indexChanged = false;
      std::vector<std::string> comboObjectItemsLabel{
          "Airplane",  "Bunny",  "Skull", "Box",   "Chamferbox", "Container",
          "Geosphere", "Teapot", "Hand",  "T-Rex", "Sphere"};
      std::vector<std::string> comboObjectItems{
          "airplane.obj",   "bunny.obj",     "skull.obj",     "box.obj",
          "chamferbox.obj", "container.obj", "geosphere.obj", "teapot.obj",
          "hand.obj",       "trex.obj",      "sphere.obj"};

      ImGui::PushItemWidth(120);
      if (ImGui::BeginCombo(
              "Selecionar Modelo",
              comboObjectItemsLabel.at(currentComboIndex).c_str())) {
        for (auto index : iter::range(comboObjectItemsLabel.size())) {
          const bool isSelected{currentComboIndex == index};
          if (ImGui::Selectable(comboObjectItemsLabel.at(index).c_str(),
                                isSelected)) {
            indexChanged = index != currentComboIndex;
            currentComboIndex = index;
            m_fileName = comboObjectItems.at(index);
          }
          if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      if (indexChanged) {
        m_model.loadFromFile(getAssetsPath() + m_fileName);

        m_model.setupVAO(m_program);

        m_trianglesToDraw = m_model.getNumTriangles();
        m_model.render(m_trianglesToDraw);
      }
      m_projMatrix = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 5.0f);
    }

    ImGui::End();

    // Seleção da cor do modelo
    ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - 345), 50));
    ImGui::SetNextWindowSize(ImVec2(340, 40));
    ImGui::Begin("Color edit", nullptr, {ImGuiWindowFlags_NoDecoration});
    ImGui::ColorEdit3("Cor do objeto", m_clearColor.data());
    ImGui::End();

    // Aviso sobre triângulos zerados
    ImGui::SetNextWindowPos(
        ImVec2((m_viewportWidth - 400) / 2, (m_viewportHeight - 85) / 2));
    ImGui::SetNextWindowSize(ImVec2(400, 85));
    auto windowFlags{ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs};
    ImGui::Begin("triangles", nullptr, windowFlags);
    ImGui::PushFont(m_font);

    if (m_trianglesToDraw == 0) {
      ImGui::Text("No more triangles!");
    }
    ImGui::PopFont();
    ImGui::End();
  }
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  m_trackBall.resizeViewport(width, height);
}

void OpenGLWindow::terminateGL() { glDeleteProgram(m_program); }

void OpenGLWindow::update() {
  m_modelMatrix = m_trackBall.getRotation();

  m_viewMatrix =
      glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f + m_zoom),
                  glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}