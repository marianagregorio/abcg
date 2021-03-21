#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class Input { Right, Left, Down, Up, Fire };
enum class State { Playing, GameOver, Win, NewGame };

struct GameData {
  State m_state{State::NewGame};
  long unsigned int m_score{0};
  std::bitset<5> m_input;
};

#endif