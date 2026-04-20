#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <tuple>
#include "BrezenHemLine.h"
using namespace std;

#define GRID_WIDTH 100
#define GRID_HEIGHT 50
#define SIZE_CHANGE 8
#define MOVE_STEP 50

//
//UCHAR reson
//00 00 00 00 -> 대각 직선 / 대각방향 직선방향
// 최상위 비트 -> 목적지를 찾았는지 아닌지
// 차상위 비트 -> 부모의 위치 0 직석 1 대각
// 차상위 2비트 -> 부모에 대한 자식의 상세 위치 == 아래와 같은 규칙(자식이 좌상이면 00)
// 00 - 좌상 / 01 - 우상 / 10 - 좌하 / 11 - 우하 
// 00 - 상 / 01 - 우 / 10 좌 / 11 하
// 하위 5비트 -> 나아가야할 방향,찾아야할 방향
// 직선은 최하위 2비트만 대각은 4비트
// 직선 0001 0010 
// 대각 1000 0100 0010 0001
// 각각 시계방향으로 탐색
// 직선은 진행방향의 왼,자기방향,오
// 대각은 진행방향의 왼쪽수직,왼쪽직선,자기방향,오른쪽직선,오른쪽수직 
// 0x00은 찾지 못한 것 -> 맵 벗어난 탐색 혹은 장애물
//


//
// 다 때려치고 그냥 1~8번호 매기고 번호에 맞는 탐색할 것
// 다만 중복 체크를 피하기 위해서 진행방향의 좌우를 상위 4비트중 하위 2비트에 좌 우 순으로 0010 0001
// 1 상 2 우 3 좌 4 하 5 좌상 6 우상 7 좌하 8 우하 
//
//


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
	JpsNode(int x, int y, UCHAR reason = 0,JpsNode* p = nullptr, UINT g = 0, UINT h = 0, UINT f = 0) :curX(x), curY(y), reason(reason),parent(p),G(g),H(h),F(f) {}
	bool isEnd(int, int);
	bool isRootFound();
	void SetParent(JpsNode&);
	void SetParent();
	void DrawNode(HDC&,HBRUSH&,int,int,int);
	//void DrawRoot(HDC&, HPEN&,int,int);
	void DrawRoot(HDC&,HBRUSH& ,HPEN&, int, int);
	void replaceNode(int , int );
	void resetH();
	//void resetG();
	JpsNode* GetParent() {
		return parent;
	}
	int GetcurX() { return curX; }
	int GetcurY() { return curY; }
	void SetG(UINT g) { G = g; }
	void SetH(UINT h) { H = h; }
	UINT GetG() { return G; }
	UINT GetF() { return F; }
};

extern JpsNode* Start;
extern JpsNode* End;

struct Node {
	JpsNode* jpsNode;
	Node* next;
	Node();
	Node(JpsNode&);
	Node& operator=(Node&);
};

class JpsNodeList {
private:
	int len;
	Node head;
	Node rear;
public:
	JpsNodeList();
	bool isEmpty();
	void InsertNode(JpsNode&);
	void InsertNode(Node&);
	//bool isRear(Node&);
	//Node& GetRear();

	Node* FindNode(int, int);
	Node* PopNode();

	void ClearList();
	void Draw(HDC&,HBRUSH&,int,int,int);
	void SortList();
};

bool checkPos(int x, int y);
void SetStart(int x, int y);
void SetEnd(int x, int y);

//void SearchRoot(HWND&);
void ObstacleRender(HDC& ,HBRUSH&,HPEN&,int,int);
void GridRender(HDC&, HPEN& ,int ,int);

void Right(JpsNode&,int,int,UINT		,HDC& hdc,int xOffset, int yOffset);
void Left(JpsNode&, int, int, UINT		,HDC& hdc,int xOffset, int yOffset);
void Up(JpsNode&, int, int, UINT		,HDC& hdc,int xOffset, int yOffset);
void Down(JpsNode&, int, int, UINT		,HDC& hdc,int xOffset, int yOffset);
void RightUp(JpsNode&, int, int, UINT	,HDC& hdc,int xOffset, int yOffset);
void RightDown(JpsNode&, int, int, UINT	,HDC& hdc,int xOffset, int yOffset);
void LeftUp(JpsNode&, int, int, UINT	,HDC& hdc,int xOffset, int yOffset);
void LeftDown(JpsNode&, int, int, UINT	,HDC& hdc,int xOffset, int yOffset);

bool CheckLeft(int, int		,HDC& hdc,int xOffset, int yOffset);
bool CheckRight(int, int	,HDC& hdc,int xOffset, int yOffset);
bool CheckUp(int, int		,HDC& hdc,int xOffset, int yOffset);
bool CheckDown(int, int		,HDC& hdc,int xOffset, int yOffset);

void CreateNewNode(JpsNode&, int, int, UINT, UCHAR);

void SearchRoot(HDC& hdc, HBRUSH& hTileBrush,int xOffset, int yOffset);
void GetBrezenHemRoot(JpsNode&);

void DrawRectangles(HDC hMemDc, HBRUSH hTileBrush, int xOffset, int yOffset);

extern JpsNodeList openList;
extern JpsNodeList closeList;

extern int GRID_SIZE;

extern WCHAR Tile[GRID_HEIGHT][GRID_WIDTH];

extern std::vector<std::tuple<int, int, int>> exploredPositions;
