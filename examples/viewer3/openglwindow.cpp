#include "openglwindow.hpp"

#include <imgui.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "imfilebrowser.h"

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
  glClearColor(0, 0, 0, 1);
  glEnable(GL_DEPTH_TEST);

  // Create programs
  for (const auto& name : m_shaderNames) {
    auto program{createProgramFromFile(getAssetsPath() + name + ".vert",
                                       getAssetsPath() + name + ".frag")};
    m_programs.push_back(program);
  }

  // Load model
  m_model.loadFromFile(getAssetsPath() + "bunny.obj");
  m_trianglesToDraw = m_model.getNumTriangles();
}

void OpenGLWindow::paintGL() {
  update();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  // Use currently selected program
  const auto program{m_programs.at(m_currentProgramIndex)};
  glUseProgram(program);

  // Get location of uniform variables
  GLint viewMatrixLoc{glGetUniformLocation(program, "viewMatrix")};
  GLint projMatrixLoc{glGetUniformLocation(program, "projMatrix")};
  GLint modelMatrixLoc{glGetUniformLocation(program, "modelMatrix")};
  GLint normalMatrixLoc{glGetUniformLocation(program, "normalMatrix")};

  // Set uniform variables used by every scene object
  glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);

  // Set uniform variables of the current object
  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_modelMatrix[0][0]);

  auto modelViewMatrix{glm::mat3(m_viewMatrix * m_modelMatrix)};
  glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
  glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);

  m_model.render(m_trianglesToDraw);

  glUseProgram(0);
}

void OpenGLWindow::paintUI() {
  static ImGui::FileBrowser fileDialog;
  fileDialog.SetTitle("Load 3D Model");
  fileDialog.SetTypeFilters({".obj"});
  fileDialog.SetWindowSize(m_viewportWidth * 0.8f, m_viewportHeight * 0.8f);
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

  // Create a window for the other widgets
  {
    auto widgetSize{ImVec2(222, 90)};
    ImGui::SetNextWindowPos(ImVec2(m_viewportWidth - widgetSize.x - 5, 5));
    ImGui::SetNextWindowSize(widgetSize);
    ImGui::Begin("Widget window", nullptr, ImGuiWindowFlags_NoDecoration);

    static bool faceCulling{};
    ImGui::Checkbox("Back-face culling", &faceCulling);

    if (faceCulling) {
      glEnable(GL_CULL_FACE);
    } else {
      glDisable(GL_CULL_FACE);
    }

    // CW/CCW combo box
    {
      static std::size_t currentIndex{};
      std::vector<std::string> comboItems{"CCW", "CW"};

      ImGui::PushItemWidth(120);
      if (ImGui::BeginCombo("Front face",
                            comboItems.at(currentIndex).c_str())) {
        for (auto index : iter::range(comboItems.size())) {
          const bool isSelected{currentIndex == index};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected))
            currentIndex = index;
          if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      if (currentIndex == 0) {
        glFrontFace(GL_CCW);
      } else {
        glFrontFace(GL_CW);
      }
    }

    // Projection combo box
    {
      static std::size_t currentIndex{};
      std::vector<std::string> comboItems{"Perspective", "Orthographic"};

      ImGui::PushItemWidth(120);
      if (ImGui::BeginCombo("Projection",
                            comboItems.at(currentIndex).c_str())) {
        for (auto index : iter::range(comboItems.size())) {
          const bool isSelected{currentIndex == index};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected))
            currentIndex = index;
          if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      if (currentIndex == 0) {
        auto aspect{static_cast<float>(m_viewportWidth) /
                    static_cast<float>(m_viewportHeight)};
        m_projMatrix =
            glm::perspective(glm::radians(45.0f), aspect, 0.1f, 5.0f);

      } else {
        m_projMatrix = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 5.0f);
      }
    }

    ImGui::End();
  }

  // Shader combo box
  {
    static std::size_t currentIndex{};

    ImGui::PushItemWidth(120);
    if (ImGui::BeginCombo("Shader", m_shaderNames.at(currentIndex))) {
      for (auto index : iter::range(m_shaderNames.size())) {
        const bool isSelected{currentIndex == index};
        if (ImGui::Selectable(m_shaderNames.at(index), isSelected))
          currentIndex = index;
        if (isSelected) ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    // Set up VAO if shader program has changed
    if (static_cast<int>(currentIndex) != m_currentProgramIndex) {
      m_currentProgramIndex = currentIndex;
      m_model.setupVAO(m_programs.at(m_currentProgramIndex));
    }
  }

  if (ImGui::Button("Load 3D Model...", ImVec2(-1, -1))) {
    fileDialog.Open();
  }

  fileDialog.Display();

  if (fileDialog.HasSelected()) {
    // Load model
    m_model.loadFromFile(fileDialog.GetSelected().string());
    m_model.setupVAO(m_programs.at(m_currentProgramIndex));
    m_trianglesToDraw = m_model.getNumTriangles();
    fileDialog.ClearSelected();
  }
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  m_trackBall.resizeViewport(width, height);
}

void OpenGLWindow::terminateGL() {
  for (const auto& program : m_programs) {
    glDeleteProgram(program);
  }
}

void OpenGLWindow::update() {
  m_modelMatrix = m_trackBall.getRotation();

  m_viewMatrix =
      glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f + m_zoom),
                  glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}