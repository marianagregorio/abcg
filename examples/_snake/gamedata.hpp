#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class Input { Right, Left, Down, Up };
enum class State { Playing, GameOver, Win };

struct GameData {
  State m_state{State::Playing};
  // pq m_input não é do tipo Input?
  std::bitset<5> m_input;  // [fire, up, down, left, right]
};

#endif