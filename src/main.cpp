#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "00Names.hpp"
#ifdef __WINDOWS__
#include <windows.h>
#endif

Maze maze;
Preferences preferences;
Tool tool = Tool::DrawWalls;
Algorithm algorithm = Algorithm::FloodFill;

bool drawDistances = true;
bool drawPath = false;

int main() {
  MvWindow window("OreMaze Editor");

#ifdef __WINDOWS__
  MvFont font(R"(C:\Windows\Fonts\Arial.ttf)", window.height() / 25);
#else
  MvFont font("Assets/OpenSans-Regular.ttf", 30);
#endif

  window.setFont(font);
  MvGui::setWindow(window);

  maze.clear();

  while (window.isOpen()) {
    preferences.cellSize = min((window.height() - 100) / maze.size.y, (window.width() - 500) / maze.size.y);
    maze.floodFill();

    window.clear(MvColor(115, 140, 152));
    MvGui::newFrame();

    const vec2i tl = MvGui::getEmptyDockspace().center() - maze.size * preferences.cellSize / 2;

    // * Draw maze
    maze.draw(window, tl);
    if (drawPath) maze.drawPath(window, tl);

    // * Save & Load
    if (MvGui::Button("Open") || (Mova::isKeyHeld(MvKey::Ctrl) && Mova::isKeyPressed(MvKey::O))) maze.load();
    MvGui::sameLine();
    if (MvGui::Button("Save") || (Mova::isKeyHeld(MvKey::Ctrl) && Mova::isKeyPressed(MvKey::S))) maze.save();
    if (MvGui::Button("Export") || (Mova::isKeyHeld(MvKey::Ctrl) && Mova::isKeyPressed(MvKey::E))) maze.exportMaze(window.getFont());
    MvGui::sameLine();
    if (MvGui::Button("Clear") || (Mova::isKeyHeld(MvKey::Ctrl) && Mova::isKeyPressed(MvKey::N))) maze.clear();
    MvGui::newLine();

    // * Size
    { // Width
      MvGui::Text("Maze Width: %zu", maze.size.x);
      MvGui::sameLine();
      if (MvGui::Button("-")) {
        maze.size.x = max(maze.size.x - 1, 1);
        maze.start.x = min(maze.start.x, maze.size.x - 1);
        maze.finish.x = min(maze.finish.x, maze.size.x - 1);
      }
      MvGui::sameLine();
      if (MvGui::Button("+")) maze.size.x = min(maze.size.x + 1, MAZE_WIDTH);
    }
    { // Height
      MvGui::Text("Maze Height: %zu", maze.size.y);
      MvGui::sameLine();
      if (MvGui::Button("-")) {
        maze.size.y = max(maze.size.y - 1, 1);
        maze.start.y = min(maze.start.y, maze.size.y - 1);
        maze.finish.y = min(maze.finish.y, maze.size.y - 1);
      }
      MvGui::sameLine();
      if (MvGui::Button("+")) maze.size.y = min(maze.size.y + 1, MAZE_HEIGHT);
    }
    MvGui::newLine();

    // * Start & Finish
    if (maze.start == MAZE_AREA) MvGui::TextUnformatted("Start: (Not specified)");
    else MvGui::Text("Start: (%zu, %zu)", maze.start.x, maze.size.y - maze.start.y - 1);
    if (tool == Tool::DrawWalls) {
      MvGui::sameLine();
      if (MvGui::Button("Select")) tool = Tool::SelectStart;
    }

    if (maze.finish == MAZE_AREA) MvGui::TextUnformatted("Finish: (Not specified)");
    else MvGui::Text("Finish: (%zu, %zu)", maze.finish.x, maze.size.y - maze.finish.y - 1);
    if (tool == Tool::DrawWalls) {
      MvGui::sameLine();
      if (MvGui::Button("Select")) tool = Tool::SelectFinish;
    }
    MvGui::newLine();

    // * View enables
    MvGui::TextUnformatted("Distances: ");
    MvGui::sameLine();
    MvGui::Switch(drawDistances);

    MvGui::TextUnformatted("Path: ");
    MvGui::sameLine();
    MvGui::Switch(drawPath);
    MvGui::newLine();

    if (MvGui::Button("Flood fill")) algorithm = Algorithm::FloodFill;
    if (MvGui::Button("Left hand")) algorithm = Algorithm::LeftHand;
    if (MvGui::Button("Right hand")) algorithm = Algorithm::RightHand;
    MvGui::newLine();

    // * Stats
    MvGui::Text("Walls: %zu", maze.countWalls());
    MvGui::Text("Columns: %zu", (maze.size.x + 1) * (maze.size.y + 1));
    if (maze.start != MAZE_AREA && maze.distance(maze.start) != MAX_DISTANCE) MvGui::Text("Shortest path: %zu cells", maze.distance(maze.start));
    const uint32_t leftHand = maze.leftHandPath();
    const uint32_t rightHand = maze.rightHandPath();
    if (leftHand != MAX_DISTANCE) MvGui::Text("Left hand path: %zu cells", leftHand);
    if (rightHand != MAX_DISTANCE) MvGui::Text("Right hand path: %zu cells", rightHand);
    if (MvGui::Button("Made by InfiniteCoder (Dima Lobach) (Copy Link)")) Mova::copyToClipboard("https://www.youtube.com/@InfiniteCoder01/about");
    if (MvGui::Button("Powered by pinmode.by (Copy Link)")) Mova::copyToClipboard("http://pinmode.by/");

    MvGui::endFrame();
    Mova::nextFrame();
  }
  return 0;
}
