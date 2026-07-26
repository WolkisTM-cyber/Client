#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>

struct PathNode {
    int x, y, z;
    float gCost;
    float hCost;
    int parentIndex;

    float FCost() const { return gCost + hCost; }
};

class PathFinder : public Module {
public:
    PathFinder() : Module("PathFinder", "Path Finder", Category::World, 0) {}

    std::vector<PathNode> FindPath(int startX, int startY, int startZ, int targetX, int targetY, int targetZ) {
        std::vector<PathNode> openList;
        std::vector<PathNode> closedList;

        PathNode startNode{ startX, startY, startZ, 0, Heuristic(startX, startY, startZ, targetX, targetY, targetZ), -1 };
        openList.push_back(startNode);

        while (!openList.empty() && closedList.size() < 300) {
            size_t bestIdx = 0;
            for (size_t i = 1; i < openList.size(); ++i) {
                if (openList[i].FCost() < openList[bestIdx].FCost()) bestIdx = i;
            }

            PathNode current = openList[bestIdx];
            openList.erase(openList.begin() + bestIdx);
            closedList.push_back(current);

            if (abs(current.x - targetX) <= 1 && abs(current.y - targetY) <= 1 && abs(current.z - targetZ) <= 1) {
                return closedList;
            }

            static const int dx[] = { 1, -1, 0, 0, 0, 0 };
            static const int dy[] = { 0, 0, 1, -1, 0, 0 };
            static const int dz[] = { 0, 0, 0, 0, 1, -1 };

            for (int i = 0; i < 6; ++i) {
                int nx = current.x + dx[i];
                int ny = current.y + dy[i];
                int nz = current.z + dz[i];

                bool inClosed = false;
                for (auto& n : closedList) {
                    if (n.x == nx && n.y == ny && n.z == nz) { inClosed = true; break; }
                }
                if (inClosed) continue;

                float newG = current.gCost + 1.0f;
                PathNode neighbor{ nx, ny, nz, newG, Heuristic(nx, ny, nz, targetX, targetY, targetZ), (int)closedList.size() - 1 };

                bool inOpen = false;
                for (auto& n : openList) {
                    if (n.x == nx && n.y == ny && n.z == nz) {
                        if (newG < n.gCost) { n.gCost = newG; n.parentIndex = neighbor.parentIndex; }
                        inOpen = true;
                        break;
                    }
                }
                if (!inOpen) openList.push_back(neighbor);
            }
        }

        return closedList;
    }

private:
    float Heuristic(int x1, int y1, int z1, int x2, int y2, int z2) {
        return (float)(abs(x1 - x2) + abs(y1 - y2) + abs(z1 - z2));
    }
};
