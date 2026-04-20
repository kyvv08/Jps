#pragma once
#include <vector>
#include <cmath>

using namespace std;

class Point {
public:
	int x;
	int y;
    Point(int x, int y):x(x),y(y){}
};

class BrezenHemLine
{
public:
    static void BrezenHem(int x,int y, int ex, int ey, vector<Point>& lt) {
        if (x == ex || y == ey) { return; }
        int dx = x - ex;
        int dy = y - ey;
        int xOffset = dx > 0 ? -1 : 1;
        int yOffset = dy > 0 ? -1 : 1;
        int p;
        dx = abs(dx);
        dy = abs(dy);

        if (dx == dy) {
            for (x += xOffset; xOffset > 0 ? x <= ex: x >= ex; x += xOffset) {
                y += yOffset;
                lt.push_back(Point(x, y));
            }
            return;
        }
        else if (dx > dy) {
            p = 2 * dy - dx;
            for (x += xOffset; (xOffset>0 ? x <= ex : x >= ex); x += xOffset) {
                if (p < 0) {
                    p += 2 * dy;
                }
                else {
                    p += 2 * (dy - dx);
                    y += yOffset;
                }
                lt.push_back(Point(x, y));
            }
        }
        else {
            p = 2 * dx - dy;
            for (y += yOffset; yOffset > 0 ? y <= ey : y >= ey; y += yOffset) {
                if (p < 0) {
                    p += 2 * dx;
                }
                else {
                    p += 2 * (dx - dy);
                    x += xOffset;
                }
                lt.push_back(Point(x, y));
            }
        }
    }
};
