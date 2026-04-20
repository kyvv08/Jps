#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <tuple>
#include <queue>
#include "BrezenHemLine.h"
using namespace std;

#define GRID_WIDTH 100
#define GRID_HEIGHT 50
#define SIZE_CHANGE 8
#define MOVE_STEP 50

enum class JumpDir : UCHAR {
    NONE = 0x00,
    UP = 0x01,
    RIGHT = 0x02,
    LEFT = 0x03,
    DOWN = 0x04,
    LEFT_UP = 0x05,
    RIGHT_UP = 0x06,
    LEFT_DOWN = 0x07,
    RIGHT_DOWN = 0x08
};

// Flags for forced neighbors
#define FLAG_FORCED_RIGHT 0x10
#define FLAG_FORCED_LEFT  0x20

class JpsNode {
private:
	JpsNode* parent;
	int curX;
	int curY;
	UINT G;
	UINT H;
	UINT F;
public:
	UCHAR reason;
    bool closed; // true if it has been evaluated (in closeList)
    
	JpsNode(int x, int y, UCHAR reason = 0, JpsNode* p = nullptr, UINT g = 0, UINT h = 0, UINT f = 0) 
        : curX(x), curY(y), reason(reason), parent(p), G(g), H(h), F(f), closed(false) {}

	bool isEnd(const int ex, const int ey) const { return (curX == ex && curY == ey); }
	bool isRootFound() const { return parent != nullptr; }
	void SetParent(JpsNode* const node) { parent = node; }
	void SetParent() { parent = nullptr; }

	JpsNode* GetParent() const { return parent; }
	int GetcurX() const { return curX; }
	int GetcurY() const { return curY; }
	void SetG(const UINT g) { G = g; }
	void SetH(const UINT h) { H = h; }
    void SetF(const UINT f) { F = f; }
	UINT GetG() const { return G; }
	UINT GetH() const { return H; }
	UINT GetF() const { return F; }
};

struct NodeCompare {
    bool operator()(const JpsNode* a, const JpsNode* b) const {
        return a->GetF() > b->GetF(); // Min-heap
    }
};

class JpsEngine {
public:
    int GRID_SIZE = 16;
    WCHAR Tile[GRID_HEIGHT][GRID_WIDTH];
    std::vector<std::tuple<int, int, int>> exploredPositions;
    int colorIndex = 0;

    JpsNode* StartNode;
    JpsNode* EndNode;

    std::priority_queue<JpsNode*, std::vector<JpsNode*>, NodeCompare> openList;
    JpsNode* gridNodes[GRID_HEIGHT][GRID_WIDTH];

    JpsEngine();
    ~JpsEngine();

    void InitializeGridMap();
    void ClearLists();
    
    bool IsValidPathPosition(const int x, const int y) const;
    void SetStart(const int x, const int y);
    void SetEnd(const int x, const int y);

    void RegisterJumpPoint(JpsNode* const parent, const int x, const int y, const UINT g, const UCHAR result);
    
    void ExploreStraight(JpsNode* const node, const int x, const int y, const UINT g, const JumpDir dir = JumpDir::NONE);
    void ExploreDiagonal(JpsNode* const node, const int x, const int y, const UINT g, const JumpDir dir = JumpDir::NONE);
    bool HasForcedNeighborStraight(const int x, const int y, const JumpDir dir = JumpDir::NONE);

    void ExpandNextNodeFromQueue();
    void VerifyDirectLineOfSight(JpsNode* const node);

    void DrawNode(HDC& hdc, HBRUSH& hTileBrush, const JpsNode* const node, const int xOffset, const int yOffset, const int color) const;
    void DrawRoot(HDC& hdc, HBRUSH& hTileBrush, HPEN& hGridPen, const int xOffset, const int yOffset) const;
    void DrawRectangles(HDC hdc, const int xOffset, const int yOffset) const;
    void RenderAll(HDC& hdc, HBRUSH& hTileBrush, HPEN& hGridPen, const int xOffset, const int yOffset) const;

    void resetH(JpsNode* const node) const;
    bool isSearchFinished() const;
    void PrepareSearch();
};