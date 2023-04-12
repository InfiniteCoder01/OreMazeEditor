#include "stb_image_write.h"
#include <movaGUI.hpp>
#include <stdio.h>

using namespace Math;
using namespace VectorMath;

#pragma region Directions
enum Direction : uint8_t {
  NORTH = 0x01,
  EAST = 0x02,
  SOUTH = 0x04,
  WEST = 0x08,
};

const uint32_t MAZE_WIDTH = 16;
const uint32_t MAZE_HEIGHT = 16;
const uint32_t MAZE_AREA = MAZE_WIDTH * MAZE_HEIGHT, MAX_DISTANCE = MAZE_AREA + 1;

inline vec2i neighbour(vec2i cell, uint8_t dir) {
  if (dir == NORTH) return cell - vec2i(0, 1);
  if (dir == EAST) return cell + vec2i(1, 0);
  if (dir == SOUTH) return cell + vec2i(0, 1);
  if (dir == WEST) return cell - vec2i(1, 0);
  MV_FATALERR("Wrong direction");
  return 0;
}

inline uint8_t flip(uint8_t dir) {
  if (dir == NORTH) return SOUTH;
  if (dir == EAST) return WEST;
  if (dir == SOUTH) return NORTH;
  if (dir == WEST) return EAST;
  MV_FATALERR("Wrong direction");
  return 0;
}
#pragma endregion Directions
#pragma region Preferences
struct Preferences {
  uint32_t cellSize = 100, colSize = 10, wallThickness = 6;
  MvColor mazeBoardColor = MvColor(50, 50, 50);
  MvColor columnColor = MvColor::white;

  MvColor wallColor = MvColor::red;
  MvColor startColor = MvColor(255, 250, 246);
  MvColor finishColor = MvColor(255, 253, 208);

  MvColor pathColor = MvColor::blue;
};

extern Preferences preferences;
#pragma endregion Preferences
#pragma region Maze
enum class Algorithm {
  FloodFill,
  LeftHand,
  RightHand,
};

extern Algorithm algorithm;

struct Maze {
  std::array<uint8_t, MAZE_AREA> data;
  std::array<uint32_t, MAZE_AREA> distances;
  vec2u size = 9;
  vec2u start = vec2u(0, size.y - 1), finish = vec2u(size.x - 1, 0);

  // * Utils
  void load(const std::string& filename);
  void load();
  void save(const std::string& filename);
  void save();
  void exportMaze(const std::string& filename, MvFont& font);
  void exportMaze(MvFont& font);
  void clear();
  bool neighbourBounds(vec2i cell, uint8_t dir) const;
  void draw(MvWindow& window, vec2i tl);
  void drawPath(MvWindow& window, vec2i tl);

  MvColor getWallColor(int32_t x, int32_t y, uint8_t direcion) const {
    if (vec2u(x, y) == start || neighbour(vec2u(x, y), direcion) == start) return preferences.startColor;
    if (vec2u(x, y) == finish || neighbour(vec2u(x, y), direcion) == finish) return preferences.finishColor;
    return preferences.wallColor;
  }

  // * Cell interactions
  uint8_t& operator()(vec2i cell) { return operator()(cell.x, cell.y); }
  uint8_t& operator()(int32_t x, int32_t y) {
    MV_ASSERT(Math::inRange(x, 0, size.x) && Math::inRange(y, 0, size.y), "Coordinates are outside of the maze");
    return data[(size.y - y - 1) + x * 16];
  }

  void set(int32_t x, int32_t y, uint8_t direction) { set(vec2i(x, y), direction); }
  void set(vec2i cell, uint8_t direction) {
    operator()(cell) |= direction;
    if (neighbourBounds(cell, direction)) operator()(neighbour(cell, direction)) |= flip(direction);
  }

  void clear(int32_t x, int32_t y, uint8_t direction) { clear(vec2i(x, y), direction); }
  void clear(vec2i cell, uint8_t direction) {
    operator()(cell) &= ~direction;
    if (neighbourBounds(cell, direction)) operator()(neighbour(cell, direction)) &= ~flip(direction);
  }

  uint32_t& distance(vec2i cell) { return distance(cell.x, cell.y); }
  uint32_t& distance(int32_t x, int32_t y) { return distances[(size.y - y - 1) + x * 16]; }

  // * Stats
  uint32_t countWalls() {
    uint32_t count = 0;
    for (uint32_t x = 0; x < size.x; x++) {
      for (uint32_t y = 0; y < size.y; y++) {
        if (x == 0 && operator()(x, y) & WEST) count++;
        if (y == 0 && operator()(x, y) & NORTH) count++;
        if (operator()(x, y) & SOUTH) count++;
        if (operator()(x, y) & EAST) count++;
      }
    }
    return count;
  }

  uint32_t handPath(std::function<uint8_t(uint8_t)> first, std::function<uint8_t(uint8_t)> second);
  uint32_t rightHandPath();
  uint32_t leftHandPath();
  void floodFill();
};

extern Maze maze;
#pragma endregion Maze

enum class Tool {
  SelectStart,
  SelectFinish,
  DrawWalls,
};

extern Tool tool;
extern bool drawDistances;
extern bool drawPath;
