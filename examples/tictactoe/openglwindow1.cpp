#include <fmt/core.h>
#include <imgui.h>

#include "openglwindow.hpp"

void OpenGLWindow::initializeGL() {
  auto windowSettings{getWindowSettings()};
  fmt::print("Initial window size: {}x{}\n", windowSettings.width,
             windowSettings.height);
}

char checkWinner(char board[]) {
  const int winningPositions[][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8},
                                     {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
                                     {0, 4, 8}, {2, 4, 6}};
  for (int i = 0; i < 8; i++) {
    if (board[winningPositions[i][0]] == board[winningPositions[i][1]] &&
        board[winningPositions[i][1]] == board[winningPositions[i][2]]) {
      return board[winningPositions[i][0]];
    }
  }
  return ' ';
}

void OpenGLWindow::paintGL() {
  // Set the clear color
  glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2],
               m_clearColor[3]);
  // Clear the color buffer
  glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLWindow::paintUI() {
  // Parent class will show fullscreen button and FPS meter
  abcg::OpenGLWindow::paintUI();

  // Our own ImGui widgets go below
  {
    // Window begin
    ImGui::Begin("Tic Tac Toe");

    // Static text
    ImGui::SetNextWindowSize(ImVec2(330, 500));

    const int boardSize = 9;

    static bool xTurn = true;
    static char board[boardSize] = {' ', ' ', ' ', ' ', ' ',
                                    ' ', ' ', ' ', ' '};

    static char winner = ' ';
    if (winner == ' ') {
      ImGui::Text("%c turn", xTurn ? 'X' : 'O');
    } else {
      ImGui::Text("%c won!", winner);
    }

    for (int i = 0; i < boardSize; i++) {
      ImGui::Button(std::string(1, board[i]).c_str(), ImVec2(100, 100));
      if (ImGui::IsItemClicked() && board[i] == ' ' && winner == ' ') {
        board[i] = xTurn ? 'X' : 'O';
        xTurn = !xTurn;
        winner = checkWinner(board);
      }
      if (i != 2 && i != 5 && i != 8) {
        ImGui::SameLine(0, 5.0);
      }
    }

    ImGui::Button("Reset", ImVec2(-1, 50));
    if (ImGui::IsItemClicked()) {
      for (int i = 0; i < boardSize; i++) {
        board[i] = ' ';
      }
      xTurn = true;
      winner = ' ';
    }

    // Window end
    ImGui::End();
  }
}