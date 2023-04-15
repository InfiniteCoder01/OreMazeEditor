#include "00Names.hpp"
#include <queue>
#include <nfd.h>

#pragma region Utils
void Maze::load(const std::string& filename) {
  FILE* file = fopen(filename.c_str(), "rb");
  fread(data.data(), 1, MAZE_AREA, file);
  fclose(file);
  start = vec2u(0, size.y - 1);
  finish = vec2u(size.x - 1, 0);
}

void Maze::load() {
  nfdchar_t* path = nullptr;
  const nfdresult_t result = NFD_OpenDialog("maz", nullptr, &path);

  if (result == NFD_OKAY) {
    load(path);
    free(path);
  } else if (result != NFD_CANCEL) MV_ERR("Error: %s\n", NFD_GetError());
}

void Maze::save(std::string filename) {
  if (filename.substr(filename.size() - 4) != ".maz") filename += ".maz";
  FILE* file = fopen(filename.c_str(), "wb");
  fwrite(data.data(), 1, MAZE_AREA, file);
  fclose(file);
}

void Maze::save() {
  nfdchar_t* path = nullptr;
  const nfdresult_t result = NFD_SaveDialog("maz", nullptr, &path);

  if (result == NFD_OKAY) {
    save(path);
    free(path);
  } else if (result != NFD_CANCEL) MV_ERR("Error: %s\n", NFD_GetError());
}

void Maze::exportMaze(std::string filename, MvFont& font) {
  static const uint32_t cellSize = 50, padding = 10, wallThickness = 5, colSize = 7;
  MvImage maze = MvImage((size + 1) * cellSize + padding * 2);
  maze.setFont(font);
  maze.clear(MvColor::white);

  // * Draw
  for (uint32_t x = 0; x < size.x; x++) {
    for (uint32_t y = 0; y < size.y; y++) {
      const auto cell = Rect<uint32_t>(vec2u(x, y) * cellSize + vec2u(cellSize, 0) + padding, cellSize);
      if (vec2u(x, y) == start || vec2u(x, y) == finish) maze.fillRect(cell, MvColor(170, 170, 170));

      // * Walls
      if (operator()(x, y) & NORTH) maze.fillRect(cell.x, cell.y - wallThickness / 2, cell.width, wallThickness, MvColor::black);
      if (operator()(x, y) & WEST) maze.fillRect(cell.x - wallThickness / 2, cell.y, wallThickness, cell.height, MvColor::black);
      if (y == size.y - 1 && operator()(x, y) & SOUTH) maze.fillRect(cell.x, cell.bottom() - wallThickness / 2, cell.width, wallThickness, MvColor::black);
      if (x == size.x - 1 && operator()(x, y) & EAST) maze.fillRect(cell.right() - wallThickness / 2, cell.y, wallThickness, cell.height, MvColor::black);

      // * Columns
      maze.fillRoundRect(cell.tl() - colSize / 2, colSize, MvColor::black, colSize / 2);
      if (y == size.y - 1) maze.fillRoundRect(cell.bl() - colSize / 2, colSize, MvColor::black, colSize / 2);
      if (x == size.x - 1) maze.fillRoundRect(cell.tr() - colSize / 2, colSize, MvColor::black, colSize / 2);
      if (y == size.y - 1 && x == size.x - 1) maze.fillRoundRect(cell.br() - colSize / 2, colSize, MvColor::black, colSize / 2);

      // * Distances
      if (drawDistances && distance(x, y) != MAX_DISTANCE) {
        auto number = std::to_string(distance(x, y));
        maze.drawText(cell.center() - maze.getTextSize(number) / 2 + vec2u(0, maze.getFont().ascent()), number, MvColor::black);
      }
    }
  }
  for (uint32_t y = 0; y < size.y; y++) {
    auto number = std::to_string(size.y - y - 1);
    maze.drawText(vec2u(0, y * cellSize) + cellSize / 2 + padding - maze.getTextSize(number) / 2 + vec2u(0, maze.getFont().ascent()), number, MvColor::black);
  }
  for (uint32_t x = 0; x < size.x; x++) {
    auto number = std::to_string(x);
    maze.drawText(vec2u((x + 1) * cellSize, size.y * cellSize) + cellSize / 2 + padding - maze.getTextSize(number) / 2 + vec2u(0, maze.getFont().ascent()), number, MvColor::black);
  }

  // * Export
  if (filename.substr(filename.size() - 4) != ".png") filename += ".png";
  stbi_write_png(filename.c_str(), maze.width(), maze.height(), 4, maze.data(), maze.width() * 4);
}

void Maze::exportMaze(MvFont& font) {
  nfdchar_t* path = nullptr;
  const nfdresult_t result = NFD_SaveDialog("png", nullptr, &path);

  if (result == NFD_OKAY) {
    exportMaze(path, font);
    free(path);
  } else if (result != NFD_CANCEL) MV_ERR("Error: %s\n", NFD_GetError());
}

void Maze::clear() {
  for (uint32_t x = 0; x < size.x; x++) {
    for (uint32_t y = 0; y < size.y; y++) {
      operator()(x, y) = (x == 0) * WEST | (y == 0) * NORTH | (x == size.x - 1) * EAST | (y == size.y - 1) * SOUTH;
    }
  }
}

bool Maze::neighbourBounds(vec2i cell, uint8_t dir) const {
  if (dir == NORTH) return cell.y > 0;
  if (dir == EAST) return cell.x < size.x - 1;
  if (dir == SOUTH) return cell.y < size.y - 1;
  if (dir == WEST) return cell.x > 0;
  return false;
}
#pragma endregion Utils
#pragma region Draw
void Maze::draw(MvWindow& window, vec2i tl) {
  for (uint32_t x = 0; x < size.x; x++) {
    for (uint32_t y = 0; y < size.y; y++) {
      const auto cell = Rect<int32_t>(tl + vec2u(x, y) * preferences.cellSize, preferences.cellSize);
      window.fillRect(cell, preferences.mazeBoardColor);

      // * Walls
      if (operator()(x, y) & NORTH) window.fillRect(cell.x, cell.y - preferences.wallThickness / 2, cell.width, preferences.wallThickness, getWallColor(x, y, NORTH));
      if (operator()(x, y) & WEST) window.fillRect(cell.x - preferences.wallThickness / 2, cell.y, preferences.wallThickness, cell.height, getWallColor(x, y, WEST));
      if (y == size.y - 1 && operator()(x, y) & SOUTH) window.fillRect(cell.x, cell.bottom() - preferences.wallThickness / 2, cell.width, preferences.wallThickness, getWallColor(x, y, SOUTH));
      if (x == size.x - 1 && operator()(x, y) & EAST) window.fillRect(cell.right() - preferences.wallThickness / 2, cell.y, preferences.wallThickness, cell.height, getWallColor(x, y, EAST));

      // * Columns
      window.fillRoundRect(cell.tl() - preferences.colSize / 2, preferences.colSize, MvColor::white, preferences.colSize / 2);
      if (y == size.y - 1) window.fillRoundRect(cell.bl() - preferences.colSize / 2, preferences.colSize, MvColor::white, preferences.colSize / 2);
      if (x == size.x - 1) window.fillRoundRect(cell.tr() - preferences.colSize / 2, preferences.colSize, MvColor::white, preferences.colSize / 2);
      if (y == size.y - 1 && x == size.x - 1) window.fillRoundRect(cell.br() - preferences.colSize / 2, preferences.colSize, MvColor::white, preferences.colSize / 2);

      // * Distances
      if (drawDistances && distance(x, y) != MAX_DISTANCE) {
        auto number = std::to_string(distance(x, y));
        MvGui::drawTextTL(cell.center() - window.getTextSize(number) / 2, number);
      }

      // * Interact
      if (cell.contains(window.getMousePosition())) {
        if (tool == Tool::SelectStart && Mova::isMouseButtonPressed(Mova::MouseLeft)) {
          start = vec2u(x, y);
          tool = Tool::DrawWalls;
        } else if (tool == Tool::SelectFinish && Mova::isMouseButtonPressed(Mova::MouseLeft)) {
          finish = vec2u(x, y);
          tool = Tool::DrawWalls;
        } else if (tool == Tool::DrawWalls) { // TODO: Cross
          uint8_t direction = 0;
          const vec2u mouse = window.getMousePosition() - cell.position();
          if (mouse.x < cell.width / 4) direction = WEST;
          else if (mouse.y < cell.height / 4) direction = NORTH;
          else if (mouse.x > cell.width / 4 * 3) direction = EAST;
          else if (mouse.y > cell.height / 4 * 3) direction = SOUTH;

          if (Mova::isMouseButtonHeld(Mova::MouseLeft)) set(x, y, direction);
          else if (Mova::isMouseButtonHeld(Mova::MouseRight)) clear(x, y, direction);
        }
      }
    }
  }
}

static uint8_t left(uint8_t direction) {
  direction >>= 1;
  if (direction == 0) return WEST;
  return direction;
}

static uint8_t right(uint8_t direction) {
  direction <<= 1;
  if (direction == WEST * 2) return NORTH;
  return direction;
}

void Maze::drawPath(MvWindow& window, vec2i tl) {
  if (start == MAZE_AREA || finish == MAZE_AREA) return;

  vec2u mouse = start;
  uint8_t direction = NORTH;
  while (true) {
    if (mouse == maze.finish) break;
    vec2u next = mouse;

    // * Flood fill path
    if (algorithm == Algorithm::FloodFill) {
      for (uint8_t direction = NORTH; direction <= WEST; direction <<= 1) {
        if ((operator()(mouse) & direction) == 0 && neighbourBounds(mouse, direction) && distance(neighbour(mouse, direction)) < distance(mouse)) next = neighbour(mouse, direction);
      }
      if (distance(next) >= distance(mouse)) break;
    }
    if (algorithm == Algorithm::LeftHand || algorithm == Algorithm::RightHand) {
      std::function<uint8_t(uint8_t)> first = right, second = left;
      if (algorithm == Algorithm::LeftHand) first = left, second = right;

      if ((operator()(mouse) & first(direction)) == 0 && neighbourBounds(mouse, first(direction))) direction = first(direction);
      else if ((operator()(mouse) & direction) == 0 && neighbourBounds(mouse, direction)) direction = direction;
      else if ((operator()(mouse) & second(direction)) == 0 && neighbourBounds(mouse, second(direction))) direction = second(direction);
      else if ((operator()(mouse) & flip(direction)) == 0 && neighbourBounds(mouse, flip(direction))) direction = flip(direction);
      else break;
      next = neighbour(mouse, direction);
      if (next == maze.start) break;
    }

    window.drawLine(tl + mouse * preferences.cellSize + preferences.cellSize / 2, tl + next * preferences.cellSize + preferences.cellSize / 2, preferences.pathColor);
    mouse = next;
  }
}
#pragma endregion Draw
#pragma region Stat
uint32_t Maze::handPath(std::function<uint8_t(uint8_t)> first, std::function<uint8_t(uint8_t)> second) {
  if (start == MAZE_AREA || finish == MAZE_AREA) return MAX_DISTANCE;

  vec2u mouse = start;
  uint8_t direction = NORTH;
  uint32_t count = 0;
  while (true) {
    if (mouse == maze.finish) return count;
    vec2u next = mouse;

    if ((operator()(mouse) & first(direction)) == 0 && neighbourBounds(mouse, first(direction))) direction = first(direction);
    else if ((operator()(mouse) & direction) == 0 && neighbourBounds(mouse, direction)) direction = direction;
    else if ((operator()(mouse) & second(direction)) == 0 && neighbourBounds(mouse, second(direction))) direction = second(direction);
    else if ((operator()(mouse) & flip(direction)) == 0 && neighbourBounds(mouse, flip(direction))) direction = flip(direction);
    else return MAX_DISTANCE;
    next = neighbour(mouse, direction);
    if (next == maze.start) return MAX_DISTANCE;
    mouse = next;
    count++;
  }
  return count;
}

uint32_t Maze::rightHandPath() { return handPath(right, left); }
uint32_t Maze::leftHandPath() { return handPath(left, right); }

void Maze::floodFill() {
  for (uint32_t x = 0; x < size.x; x++) {
    for (uint32_t y = 0; y < size.y; y++) distance(x, y) = MAX_DISTANCE;
  }

  if (finish == MAZE_AREA) return;
  std::queue<vec2u> queue;
  queue.push(finish);
  distance(finish) = 0;
  while (!queue.empty()) {
    const vec2u cell = queue.front();
    queue.pop();

    for (uint8_t direction = NORTH; direction <= WEST; direction <<= 1) {
      if ((operator()(cell) & direction) == 0 && neighbourBounds(cell, direction) && distance(neighbour(cell, direction)) == MAX_DISTANCE) {
        distance(neighbour(cell, direction)) = distance(cell) + 1;
        queue.push(neighbour(cell, direction));
      }
    }
  }
}
#pragma endregion Stat
