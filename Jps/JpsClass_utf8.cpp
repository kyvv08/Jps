#include "JpsClass.h"

WCHAR numF[64];
WCHAR numG[64];
WCHAR numH[64];

COLORREF colors[] = { RGB(200, 150, 150), RGB(150, 255, 150), RGB(150, 150, 255), RGB(255, 255, 150), RGB(150, 255, 255), RGB(255, 150, 255),RGB(50, 150, 255),RGB(100, 150, 50),RGB(150, 150, 50) };
int colorIndex = 0;

std::vector<std::tuple<int, int, int>> exploredPositions;

//JpsNodeList openList;
//JpsNodeList closeList;

int GRID_SIZE = 16;

Node::Node() :jpsNode(NULL), next(NULL) {}

Node::Node(JpsNode& node) :jpsNode(&node), next(nullptr) {}

Node& Node::operator=(Node& ref) {
    jpsNode = ref.jpsNode;
    next = ref.next;
    return *this;
}

bool JpsNode::isEnd(int x, int y) {
	if (!End) {
		return false;
	}
	if (End->curX == x && End->curY == y) { return true; }
	return false;
}

bool JpsNode::isRootFound() {//End용 함수
    return this->parent != NULL;
}

void JpsNode::SetParent(JpsNode& node) {
    parent = &node;
}

void JpsNode::SetParent() {
    parent = NULL;
}

void JpsNode::DrawNode(HDC& hdc, HBRUSH& hTileBrush, int xOffset, int yOffset,int color) {
    int x, y;
    HPEN oldBrush = (HPEN)SelectObject(hdc, hTileBrush);
    SelectObject(hdc, GetStockObject(DC_BRUSH));
    if (color == 1) {
        SetDCBrushColor(hdc, RGB(0, 0, 255));
    }
    else if (color == 2)
    {
        SetDCBrushColor(hdc, RGB(255, 100, 0));
    }
    x = curX * GRID_SIZE + xOffset;
    y = curY * GRID_SIZE + yOffset;
    Rectangle(hdc, x, y, x + GRID_SIZE + 2, y + GRID_SIZE + 2);
    if (GRID_SIZE >= 56) {
        memset(numG, L' ', 64);
        int j = swprintf_s(numG, 64, L" G: %d", G);
        TextOutW(hdc, x, y, numG, j);
        memset(numH, L' ', 64);
        j = swprintf_s(numH, 64, L" H: %d", H);
        TextOutW(hdc, x, y + 20, numH, j);
        memset(numF, L' ', 64);
        j = swprintf_s(numF, 64, L" F: %d", F);
        TextOutW(hdc, x, y + 40, numF, j);
    }
    SetDCBrushColor(hdc, RGB(100, 100, 100));
    SelectObject(hdc, oldBrush);
}

void JpsNode::DrawRoot(HDC& hdc, HBRUSH& hTileBrush,HPEN& hGridPen,int xOffset,int yOffset) {
    JpsNode* r = End;
    int x = ((r->curX * GRID_SIZE) + GRID_SIZE / 2) + xOffset;
    int y = ((r->curY * GRID_SIZE) + GRID_SIZE / 2) + yOffset;

    HPEN oldPen = (HPEN)SelectObject(hdc, hGridPen);
    HPEN curPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
    SelectObject(hdc, curPen);
    while (r->parent != NULL) {
        //r->DrawNode(hdc, hTileBrush, xOffset, yOffset,2);
        MoveToEx(hdc, x, y, NULL);
        r = r->parent;
        x = ((r->curX * GRID_SIZE) + GRID_SIZE / 2) + xOffset;
        y = ((r->curY * GRID_SIZE) + GRID_SIZE / 2) + yOffset;
        LineTo(hdc, x, y);
    }
    DeleteObject(curPen);
    SelectObject(hdc, oldPen);
}

void JpsNode::replaceNode(int x, int y) {
    Tile[curY][curX] = 0;
}

void JpsNode::resetH() {
    H = abs(End->GetcurX() - curX) + abs(End->GetcurY() - curY);
}

bool checkPos(int x, int y) {
    if (x < 0 || y< 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT || Tile[y][x] == 1) {
        return false;
    }
	return true;
}

void SetStart(int x, int y) {
    if (!Start) {
        Start = new JpsNode(x, y);
        return;
    }
    Start->replaceNode(x, y);

    delete Start;

    Start = new JpsNode((UINT)x, (UINT)y);
    Tile[y][x] = 2;
    Start->resetH();
}

void SetEnd(int x, int y) {
    if (!End) {
        End = new JpsNode(x, y);
        return;
    }
    End->replaceNode(x, y);

    delete End;

    End = new JpsNode((UINT)x, (UINT)y);
    Tile[y][x] = 3;
    Start->resetH();
}


JpsNodeList::JpsNodeList() :len(0) {
    head.next = &rear;
    rear.next = &head;
}

bool JpsNodeList::isEmpty() {
    return len == 0;
}

void JpsNodeList::InsertNode(JpsNode& node) {
    Node* temp = new Node(node);
    if (isEmpty()) {
        temp->next = head.next;
        head.next = temp;
        rear.next = temp;
    }
    else {
        rear.next = rear.next->next = temp;
        temp->next = &rear;
    }
    ++len;
}

void JpsNodeList::InsertNode(Node& node) {
    if (isEmpty()) {
        node.next = head.next;
        head.next = rear.next = &node;
    }
    else {
        rear.next = rear.next->next = &node;
        node.next = &rear;
    }
    ++len;
}

Node* JpsNodeList::FindNode(int x, int y) {
    Node* temp = head.next;
    for (int i = 0; i < len; i++) {
        if ((temp->jpsNode->GetcurX() == x) && (temp->jpsNode->GetcurY() == y)) {
            return temp;
        }
        temp = temp->next;
    }
    if (!temp->jpsNode) { temp = NULL; }
    return temp;
}

Node* JpsNodeList::PopNode() {
    Node* temp = head.next;
    head.next = temp->next;
    if (len == 1) {
        rear.next = &head;
    }
    --len;
    return temp;
}

void JpsNodeList::SortList() {
    Node* temp;
    JpsNode* swap;
    temp = head.next;
    for (int i = 0; i < len-1; i++) {
        if (temp->jpsNode->GetF() > temp->next->jpsNode->GetF()) {
            swap = temp->jpsNode;
            temp->jpsNode = temp->next->jpsNode;
            temp->next->jpsNode = swap;
        }
        temp = temp->next;
    }
}

void JpsNodeList::ClearList() {
    Node* temp = head.next;
    for (int i = 0; i < len; i++) {
        if (temp->jpsNode == Start) {
            head.next = temp->next;
            temp = head.next;
            continue;
        }
        head.next = temp->next;
        delete temp->jpsNode;
        delete temp;
        temp = head.next;
    }
    len = 0;
    rear.next = &head;
}

void JpsNodeList::Draw(HDC& hdc,HBRUSH& hTileBrush, int xOffset, int yOffset,int color) {
    Node* temp = head.next;
    for (int i = 0; i < len; i++) {
        temp->jpsNode->DrawNode(hdc,hTileBrush,xOffset,yOffset,color);
        temp = temp->next;
    }
}


// 새 노드 생성
void CreateNewNode(JpsNode& node, int x, int y, UINT g, UCHAR result) {
    Node* found = closeList.FindNode(x, y);
    if (found) { return; }
    found = openList.FindNode(x, y);
    UINT h = abs(End->GetcurX() - x) + abs(End->GetcurY() - y);
    if (!found) {
        JpsNode* temp = new JpsNode(x, y, result, &node, g, h, g + h);
        openList.InsertNode(*temp);
        openList.SortList();
        return;
    }
    if (found->jpsNode->GetG() > g) {
        found->jpsNode->SetG(g);
        found->jpsNode->SetH(g + h);
        found->jpsNode->SetParent(node);
    }
}

//---------Right-----------
void Right(JpsNode& node, int x, int y,UINT g, HDC& hdc, int xOffset, int yOffset) {

    UCHAR result = 0x02;
    g += 5;
    if (!checkPos(x, y)) {
        return;
    }
    if (End->isEnd(x, y)) {
        End->SetParent(node);
        return;
    }

    if (!checkPos(x, y - 1) && checkPos(x + 1, y - 1)) {
        result |= 0x20;
    }
    if (!checkPos(x, y + 1) && checkPos(x + 1, y + 1)) {
        result |= 0x10;
    }
    if (result != 0x02) {
        CreateNewNode(node, x, y, g, result);
        return;
    }
    //int tempX = x * GRID_SIZE + xOffset;
    //int tempY = y * GRID_SIZE + yOffset;
    //Rectangle(hdc, tempX, tempY, tempX + GRID_SIZE + 2, tempY + GRID_SIZE + 2);
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    Right(node, x + 1, y,g,hdc,xOffset,yOffset);
}
//-----------Left--------------
void Left(JpsNode& node, int x, int y, UINT g, HDC& hdc, int xOffset, int yOffset) {
    UCHAR result = 0x03;
    g += 5;
    if (!checkPos(x, y)) {
        return;
    }
    if (End->isEnd(x, y)) {
        End->SetParent(node);
        return;
    }
    if (!checkPos(x, y + 1) && checkPos(x - 1, y + 1)) {
        result |= 0x20;
    }
    if (!checkPos(x, y - 1) && checkPos(x - 1, y - 1)) {
        result |= 0x10;
    }
    if (result != 0x03) {
        CreateNewNode(node, x, y, g, result);
        return;
    }    
    /*int tempX = x * GRID_SIZE + xOffset;
    int tempY = y * GRID_SIZE + yOffset;
    Rectangle(hdc, tempX, tempY, tempX + GRID_SIZE + 2, tempY + GRID_SIZE + 2);*/
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    Left(node, x - 1, y, g, hdc, xOffset, yOffset);
}

//----------Up----------------
void Up(JpsNode& node, int x, int y, UINT g, HDC& hdc, int xOffset, int yOffset) {
    UCHAR result = 0x01;
    g += 5;
    if (!checkPos(x, y)) {
        return;
    }
    if (End->isEnd(x, y)) {
        End->SetParent(node);
        return;
    }
    if (!checkPos(x - 1, y) && checkPos(x - 1, y - 1)) {
        result |= 0x20;
    }
    if (!checkPos(x + 1, y) && checkPos(x + 1, y - 1)) {
        result |= 0x10;
    }
    if (result != 0x01) {
        CreateNewNode(node, x, y, g, result);
        return;
    }
    /*int tempX = x * GRID_SIZE + xOffset;
    int tempY = y * GRID_SIZE + yOffset;
    Rectangle(hdc, tempX, tempY, tempX + GRID_SIZE + 2, tempY + GRID_SIZE + 2);*/
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    Up(node, x , y - 1, g, hdc, xOffset, yOffset);
}
//------------Down---------------
void Down(JpsNode& node, int x, int y, UINT g, HDC& hdc, int xOffset, int yOffset) {
    UCHAR result = 0x04;
    g += 5;
    if (!checkPos(x, y)) {
        return;
    }
    if (End->isEnd(x, y)) {
        End->SetParent(node);
        return;
    }
    if (!checkPos(x + 1, y) && checkPos(x + 1, y + 1)) {
        result |= 0x20;
    }
    if (!checkPos(x - 1, y) && checkPos(x - 1, y + 1)) {
        result |= 0x10;
    }
    if (result != 0x04) {
        CreateNewNode(node, x, y, g, result);
        return;
    }
    /*int tempX = x * GRID_SIZE + xOffset;
    int tempY = y * GRID_SIZE + yOffset;
    Rectangle(hdc, tempX, tempY, tempX + GRID_SIZE + 2, tempY + GRID_SIZE + 2);*/
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    Down(node, x, y + 1, g, hdc, xOffset, yOffset);
}

// ----------Left Up---------------
void LeftUp(JpsNode& node, int x, int y, UINT g, HDC& hdc, int xOffset, int yOffset) {
    UCHAR result = 0x05;
    g+= 7;
    if (!checkPos(x, y)) {
        return;
    }
    if (End->isEnd(x, y)) {
        End->SetParent(node);
        return;
    }
    if (!checkPos(x, y + 1) && checkPos(x - 1, y + 1)) {
        result |= 0x20;
    }
    if (!checkPos(x + 1, y) && checkPos(x + 1, y - 1)) {
        result |= 0x10;
    }
    if ((result != 0x05) || CheckLeft(x - 1, y, hdc, xOffset, yOffset) || CheckUp(x, y - 1, hdc, xOffset, yOffset)) {
        CreateNewNode(node, x, y, g, result);
        return;
    }
    /*int tempX = x * GRID_SIZE + xOffset;
    int tempY = y * GRID_SIZE + yOffset;
    Rectangle(hdc, tempX, tempY, tempX + GRID_SIZE + 2, tempY + GRID_SIZE + 2);*/
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    LeftUp(node, x - 1, y - 1, g, hdc, xOffset, yOffset);
}

// --------------- Right Up ----------------
void RightUp(JpsNode& node, int x, int y, UINT g, HDC& hdc, int xOffset, int yOffset) {
    UCHAR result = 0x06;
    g += 7;
    if (!checkPos(x, y)) {
        return;
    }
    if (End->isEnd(x, y)) {
        End->SetParent(node);
        return;
    }
    if (!checkPos(x - 1, y) && checkPos(x - 1, y - 1)) {
        result |= 0x20;
    }
    if (!checkPos(x, y + 1) && checkPos(x + 1, y + 1)) {
        result |= 0x10;
    }
    if ((result != 0x06) || CheckUp(x, y - 1, hdc, xOffset, yOffset) || CheckRight(x + 1, y, hdc, xOffset, yOffset)) {
        CreateNewNode(node, x, y, g, result);
        return;
    }
    /*int tempX = x * GRID_SIZE + xOffset;
    int tempY = y * GRID_SIZE + yOffset;
    Rectangle(hdc, tempX, tempY, tempX + GRID_SIZE + 2, tempY + GRID_SIZE + 2);*/
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    RightUp(node, x + 1, y - 1, g, hdc, xOffset, yOffset);
}

// ----------------- Left Down -----------------
void LeftDown(JpsNode& node, int x, int y, UINT g, HDC& hdc, int xOffset, int yOffset) {
    UCHAR result = 0x07;
    g += 7;
    if (!checkPos(x, y)) {
        return;
    }
    if (End->isEnd(x, y)) {
        End->SetParent(node);
        return;
    }
    if (!checkPos(x + 1, y) && checkPos(x + 1, y + 1)) {
        result |= 0x20;
    }
    if (!checkPos(x, y - 1) && checkPos(x - 1, y - 1)) {
        result |= 0x10;
    }
    if ((result != 0x07) || CheckDown(x, y + 1, hdc, xOffset, yOffset) || CheckLeft(x - 1, y, hdc, xOffset, yOffset)) {
        CreateNewNode(node, x, y, g, result);
        return;
    }
    /*int tempX = x * GRID_SIZE + xOffset;
    int tempY = y * GRID_SIZE + yOffset;
    Rectangle(hdc, tempX, tempY, tempX + GRID_SIZE + 2, tempY + GRID_SIZE + 2);*/
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    LeftDown(node, x - 1, y + 1, g, hdc, xOffset, yOffset);
}

// ----------------- Left Down -----------------
void RightDown(JpsNode& node, int x, int y, UINT g, HDC& hdc, int xOffset, int yOffset) {
    UCHAR result = 0x08;
    g += 7;
    if (!checkPos(x, y)) {
        return;
    }
    if (End->isEnd(x, y)) {
        End->SetParent(node);
        return;
    }
    if (!checkPos(x, y - 1) && checkPos(x + 1, y - 1)) {
        result |= 0x20;
    }
    if (!checkPos(x - 1, y) && checkPos(x - 1, y + 1)) {
        result |= 0x10;
    }
    if ((result != 0x08) || CheckRight(x + 1, y, hdc, xOffset, yOffset) || CheckDown(x, y + 1, hdc, xOffset, yOffset)) {
        CreateNewNode(node, x, y, g, result);
        return;
    }
    /*int tempX = x * GRID_SIZE + xOffset;
    int tempY = y * GRID_SIZE + yOffset;
    Rectangle(hdc, tempX, tempY, tempX + GRID_SIZE + 2, tempY + GRID_SIZE + 2);*/
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    RightDown(node, x + 1, y + 1, g, hdc, xOffset, yOffset);
}
//----------직선 체크
bool CheckLeft(int x, int y, HDC& hdc, int xOffset, int yOffset) {
    if (!checkPos(x, y)) {
        return false;
    }
    if ((!checkPos(x, y - 1) && checkPos(x - 1, y - 1)) || (!checkPos(x, y + 1) && checkPos(x - 1, y + 1)) || End->isEnd(x, y)) {
        return true;
    }
    /*int tempX = x * GRID_SIZE + xOffset;
    int tempY = y * GRID_SIZE + yOffset;
    Rectangle(hdc, tempX, tempY, tempX + GRID_SIZE + 2, tempY + GRID_SIZE + 2);*/
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    return CheckLeft(x - 1, y, hdc, xOffset, yOffset);
}

bool CheckRight(int x, int y, HDC& hdc, int xOffset, int yOffset) {
    if (!checkPos(x, y)) {
        return false;
    }
    if ((!checkPos(x, y - 1) && checkPos(x + 1, y - 1)) || (!checkPos(x, y + 1) && checkPos(x + 1, y + 1)) || End->isEnd(x, y)) {
        return true;
    }
    /*int tempX = x * GRID_SIZE + xOffset;
    int tempY = y * GRID_SIZE + yOffset;
    Rectangle(hdc, tempX, tempY, tempX + GRID_SIZE + 2, tempY + GRID_SIZE + 2);*/
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    return CheckRight(x + 1, y, hdc, xOffset, yOffset);
}

bool CheckUp(int x, int y, HDC& hdc, int xOffset, int yOffset) {
    if (!checkPos(x, y)) {
        return false;
    }
    if ((!checkPos(x - 1, y) && checkPos(x - 1, y - 1)) || (!checkPos(x + 1, y) && checkPos(x + 1, y - 1)) || End->isEnd(x, y)) {
        return true;
    }
    /*int tempX = x * GRID_SIZE + xOffset;
    int tempY = y * GRID_SIZE + yOffset;
    Rectangle(hdc, tempX, tempY, tempX + GRID_SIZE + 2, tempY + GRID_SIZE + 2);*/
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    return CheckUp(x, y -1, hdc, xOffset, yOffset);
}

bool CheckDown(int x, int y, HDC& hdc, int xOffset, int yOffset) {
    if (!checkPos(x, y)) {
        return false;
    }
    if ((!checkPos(x - 1, y) && checkPos(x - 1, y + 1)) || (!checkPos(x + 1, y) && checkPos(x + 1, y + 1)) || End->isEnd(x, y)) {
        return true;
    }
    /*
    int tempX = x * GRID_SIZE + xOffset;
    int tempY = y * GRID_SIZE + yOffset;
    Rectangle(hdc, tempX, tempY, tempX + GRID_SIZE + 2, tempY + GRID_SIZE + 2);*/
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    return CheckDown(x, y+1, hdc, xOffset, yOffset);
}

void SearchRoot(HDC& hdc, HBRUSH& hTileBrush,int xOffset, int yOffset) {
    JpsNode* temp = openList.PopNode()->jpsNode;
    closeList.InsertNode(*temp);
    int x = temp->GetcurX();
    int y = temp->GetcurY();
    UINT g = temp->GetG();

    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hTileBrush);
    SetDCBrushColor(hdc, colors[colorIndex]);
    SelectObject(hdc, GetStockObject(DC_BRUSH));
    switch (temp->reason & 0x0f) {
    case 0x00: {
        Right(*temp, x + 1, y, g,hdc,xOffset,yOffset);
        RightUp(*temp, x + 1, y - 1, g, hdc, xOffset, yOffset);
        Up(*temp, x, y - 1, g, hdc, xOffset, yOffset);
        LeftUp(*temp, x - 1, y - 1, g, hdc, xOffset, yOffset);
        Left(*temp, x - 1, y, g, hdc, xOffset, yOffset);
        LeftDown(*temp, x - 1, y + 1, g, hdc, xOffset, yOffset);
        Down(*temp, x, y + 1, g, hdc, xOffset, yOffset);
        RightDown(*temp, x + 1, y + 1, g, hdc, xOffset, yOffset);
        break;
    }
    case 0x01:
    {
        Up(*temp, x, y - 1, g, hdc, xOffset, yOffset);
        if (temp->reason & 0x20) {
            LeftUp(*temp, x - 1, y - 1, g, hdc, xOffset, yOffset);
        }
        if (temp->reason & 0x10) {
            RightUp(*temp, x + 1, y - 1, g, hdc, xOffset, yOffset);
        }
        break;
    }
    case 0x02:
    {
        Right(*temp, x + 1, y, g, hdc, xOffset, yOffset);
        if (temp->reason & 0x20)
        {
            RightUp(*temp, x + 1, y - 1, g, hdc, xOffset, yOffset);
        }
        if (temp->reason & 0x10) {
            RightDown(*temp, x + 1, y + 1, g, hdc, xOffset, yOffset);
        }
        break;
    }
    case 0x03:
    {
        Left(*temp, x - 1, y, g, hdc, xOffset, yOffset);
        if (temp->reason & 0x20) {
            LeftDown(*temp, x - 1, y + 1, g, hdc, xOffset, yOffset);
        }
        if (temp->reason & 0x10) {
            LeftUp(*temp, x - 1, y - 1, g, hdc, xOffset, yOffset);
        }
        break;
    }
    case 0x04:
    {
        Down(*temp, x, y + 1, g, hdc, xOffset, yOffset);
        if (temp->reason & 0x20) {
            RightDown(*temp, x + 1, y + 1, g, hdc, xOffset, yOffset);
        }
        if (temp->reason & 0x10) {
            LeftDown(*temp, x - 1, y + 1, g, hdc, xOffset, yOffset);
        }
        break;
    }
    case 0x05:
    {
        LeftUp(*temp, x - 1, y - 1, g, hdc, xOffset, yOffset);
        Left(*temp, x - 1, y, g, hdc, xOffset, yOffset);
        Up(*temp, x , y - 1, g, hdc, xOffset, yOffset);
        if (temp->reason & 0x20) {
            LeftDown(*temp, x - 1, y + 1, g, hdc, xOffset, yOffset);
        }
        if (temp->reason & 0x10) {
            RightUp(*temp, x + 1, y - 1, g, hdc, xOffset, yOffset);
        }
        break;
    }
    case 0x06:
    {
        RightUp(*temp, x + 1, y - 1, g, hdc, xOffset, yOffset);
        Up(*temp, x, y - 1, g, hdc, xOffset, yOffset);
        Right(*temp, x + 1, y, g, hdc, xOffset, yOffset);
        if (temp->reason & 0x20) {
            LeftUp(*temp, x - 1, y - 1, g, hdc, xOffset, yOffset);
        }
        if (temp->reason & 0x10) {
            RightDown(*temp, x + 1, y + 1, g, hdc, xOffset, yOffset);
        }
        break;
    }
    case 0x07:
    {
        LeftDown(*temp, x - 1, y + 1, g, hdc, xOffset, yOffset);
        Left(*temp, x - 1, y, g, hdc, xOffset, yOffset);
        Down(*temp, x, y + 1, g, hdc, xOffset, yOffset);
        if (temp->reason & 0x20) {
            RightDown(*temp, x + 1, y + 1, g, hdc, xOffset, yOffset);
        }
        if (temp->reason & 0x10) {
            LeftUp(*temp, x - 1, y - 1, g, hdc, xOffset, yOffset);
        }
        break;
    }
    case 0x08: {
        RightDown(*temp, x + 1, y + 1, g, hdc, xOffset, yOffset);
        Right(*temp, x + 1, y, g, hdc, xOffset, yOffset);
        Down(*temp, x, y + 1, g, hdc, xOffset, yOffset);
        if (temp->reason & 0x20) {
            RightUp(*temp, x + 1, y - 1, g, hdc, xOffset, yOffset);
        }
        if (temp->reason & 0x10) {
            LeftDown(*temp, x - 1, y + 1, g, hdc, xOffset, yOffset);
        }
        break;
    }
    }
    colorIndex = (colorIndex + 1) % (sizeof(colors) / sizeof(colors[0]));

    SelectObject(hdc, hOldBrush);
}

void GetBrezenHemRoot(JpsNode& node) {
    list<Point*> lt;
    list<Point*>::iterator it;
    if (!node.GetParent()) { return; }
    JpsNode* temp = node.GetParent()->GetParent();
    if (!temp) { return; }
    int x = node.GetcurX();
    int y = node.GetcurY();
    BrezenHemLine::BrezenHem(node.GetcurX(), node.GetcurY(), temp->GetcurX(), temp->GetcurY(), lt);
    if (lt.size() == 0) {
        GetBrezenHemRoot(*node.GetParent());
        return;
    }
    for (it = lt.begin(); it != lt.end(); it++) {
        if (!checkPos((*it)->x, (*it)->y)) {
            break;
        }
    }
    if (it == lt.end()) {
        node.SetParent(*temp);
        GetBrezenHemRoot(node);
    }
    else {
        GetBrezenHemRoot(*node.GetParent());
    }
}

void DrawRectangles(HDC hdc, HBRUSH hTileBrush, int xOffset, int yOffset) {
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hTileBrush);
    SelectObject(hdc, GetStockObject(DC_BRUSH));
    for (const auto& pos : exploredPositions) {
        int x = std::get<0>(pos);
        int y = std::get<1>(pos);
        int colorIndex = std::get<2>(pos);
        x= x* GRID_SIZE + xOffset;
        y= y* GRID_SIZE + yOffset;
        SetDCBrushColor(hdc, colors[colorIndex]);
        Rectangle(hdc, x, y, x + GRID_SIZE+2, y + GRID_SIZE+2);
    }
    SelectObject(hdc, oldBrush);
}
