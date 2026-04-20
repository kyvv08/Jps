#include "JpsClass.h"

WCHAR numF[64];
WCHAR numG[64];
WCHAR numH[64];

COLORREF colors[] = { RGB(200, 150, 150), RGB(150, 255, 150), RGB(150, 150, 255), RGB(255, 255, 150), 
                      RGB(150, 255, 255), RGB(255, 150, 255), RGB(50, 150, 255), RGB(100, 150, 50), RGB(150, 150, 50) };

JpsEngine::JpsEngine() {
    StartNode = nullptr;
    EndNode = nullptr;
    InitializeGridMap();
}

JpsEngine::~JpsEngine() {
    ClearLists();
    if (StartNode) delete StartNode;
    if (EndNode) delete EndNode;
}

void JpsEngine::InitializeGridMap() {
    for (int i = 0; i < GRID_HEIGHT; i++) {
        wmemset(Tile[i], 0, GRID_WIDTH);
        for (int j = 0; j < GRID_WIDTH; j++) {
            gridNodes[i][j] = nullptr;
        }
    }
}

void JpsEngine::ClearLists() {
    exploredPositions.clear();
    colorIndex = 0;
    while (!openList.empty()) {
        openList.pop();
    }
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            if (gridNodes[i][j]) {
                if (gridNodes[i][j] != StartNode && gridNodes[i][j] != EndNode) {
                    delete gridNodes[i][j];
                }
                gridNodes[i][j] = nullptr;
            }
        }
    }
    
    if (StartNode) {
        StartNode->closed = false;
        gridNodes[StartNode->GetcurY()][StartNode->GetcurX()] = StartNode;
    }
    if (EndNode) {
        EndNode->closed = false;
        gridNodes[EndNode->GetcurY()][EndNode->GetcurX()] = EndNode;
    }
}

void JpsEngine::PrepareSearch() {
    ClearLists();
    if (StartNode) {
        openList.push(StartNode);
        StartNode->SetParent();
    }
    if (EndNode) {
        EndNode->SetParent();
    }
}

bool JpsEngine::isSearchFinished() const {
    if (openList.empty()) return true;
    if (EndNode && EndNode->isRootFound()) return true;
    return false;
}

bool JpsEngine::IsValidPathPosition(const int x, const int y) const {
    if (x < 0 || y < 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT || Tile[y][x] == 1) {
        return false;
    }
	return true;
}

void JpsEngine::resetH(JpsNode* const node) const {
    if (EndNode) {
        node->SetH(abs(EndNode->GetcurX() - node->GetcurX()) + abs(EndNode->GetcurY() - node->GetcurY()));
    }
}

void JpsEngine::SetStart(const int x, const int y) {
    if (StartNode) {
        Tile[StartNode->GetcurY()][StartNode->GetcurX()] = 0;
        gridNodes[StartNode->GetcurY()][StartNode->GetcurX()] = nullptr;
        delete StartNode;
    }
    StartNode = new JpsNode(x, y);
    Tile[y][x] = 2;
    gridNodes[y][x] = StartNode;
    resetH(StartNode);
}

void JpsEngine::SetEnd(const int x, const int y) {
    if (EndNode) {
        Tile[EndNode->GetcurY()][EndNode->GetcurX()] = 0;
        gridNodes[EndNode->GetcurY()][EndNode->GetcurX()] = nullptr;
        delete EndNode;
    }
    EndNode = new JpsNode(x, y);
    Tile[y][x] = 3;
    gridNodes[y][x] = EndNode;
    if (StartNode) resetH(StartNode);
}

void JpsEngine::RegisterJumpPoint(JpsNode* const parent, const int x, const int y, const UINT g, const UCHAR result) {
    JpsNode* existing = gridNodes[y][x];
    if (existing && existing->closed) return;

    UINT h = abs(EndNode->GetcurX() - x) + abs(EndNode->GetcurY() - y);
    
    if (!existing) {
        JpsNode* newNode = new JpsNode(x, y, result, parent, g, h, g + h);
        gridNodes[y][x] = newNode;
        openList.push(newNode);
    } else {
        if (existing->GetG() > g) {
            existing->SetG(g);
            existing->SetF(g + h);
            existing->SetParent(parent);
            existing->reason = result;
            openList.push(existing); 
        }
    }
}

// ---------------- Merged Directional Search Operations ---------------- //

void JpsEngine::ExploreStraight(JpsNode* const node, const int x, const int y, const UINT g, const JumpDir dir) {
    if (!IsValidPathPosition(x, y)) return;
    if (EndNode->isEnd(x, y)) {
        EndNode->SetParent(node);
        return;
    }
    
    UCHAR result = (UCHAR)dir;
    UINT nextG = g + 5;
    int nextX = x;
    int nextY = y;
    
    switch (dir) {
        case JumpDir::RIGHT:
            if (!IsValidPathPosition(x, y - 1) && IsValidPathPosition(x + 1, y - 1)) result |= FLAG_FORCED_LEFT;
            if (!IsValidPathPosition(x, y + 1) && IsValidPathPosition(x + 1, y + 1)) result |= FLAG_FORCED_RIGHT;
            nextX++;
            break;
        case JumpDir::LEFT:
            if (!IsValidPathPosition(x, y + 1) && IsValidPathPosition(x - 1, y + 1)) result |= FLAG_FORCED_LEFT;
            if (!IsValidPathPosition(x, y - 1) && IsValidPathPosition(x - 1, y - 1)) result |= FLAG_FORCED_RIGHT;
            nextX--;
            break;
        case JumpDir::UP:
            if (!IsValidPathPosition(x - 1, y) && IsValidPathPosition(x - 1, y - 1)) result |= FLAG_FORCED_LEFT;
            if (!IsValidPathPosition(x + 1, y) && IsValidPathPosition(x + 1, y - 1)) result |= FLAG_FORCED_RIGHT;
            nextY--;
            break;
        case JumpDir::DOWN:
            if (!IsValidPathPosition(x + 1, y) && IsValidPathPosition(x + 1, y + 1)) result |= FLAG_FORCED_LEFT;
            if (!IsValidPathPosition(x - 1, y) && IsValidPathPosition(x - 1, y + 1)) result |= FLAG_FORCED_RIGHT;
            nextY++;
            break;
        default: return; // Should not happen
    }
    
    if (result != (UCHAR)dir) {
        RegisterJumpPoint(node, x, y, g, result);
        return;
    }
    
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    ExploreStraight(node, nextX, nextY, nextG, dir);
}

bool JpsEngine::HasForcedNeighborStraight(const int x, const int y, const JumpDir dir) {
    if (!IsValidPathPosition(x, y)) return false;
    
    int nextX = x;
    int nextY = y;
    
    switch (dir) {
        case JumpDir::RIGHT:
            if ((!IsValidPathPosition(x, y - 1) && IsValidPathPosition(x + 1, y - 1)) || 
                (!IsValidPathPosition(x, y + 1) && IsValidPathPosition(x + 1, y + 1)) || EndNode->isEnd(x, y)) return true;
            nextX++;
            break;
        case JumpDir::LEFT:
            if ((!IsValidPathPosition(x, y - 1) && IsValidPathPosition(x - 1, y - 1)) || 
                (!IsValidPathPosition(x, y + 1) && IsValidPathPosition(x - 1, y + 1)) || EndNode->isEnd(x, y)) return true;
            nextX--;
            break;
        case JumpDir::UP:
            if ((!IsValidPathPosition(x - 1, y) && IsValidPathPosition(x - 1, y - 1)) || 
                (!IsValidPathPosition(x + 1, y) && IsValidPathPosition(x + 1, y - 1)) || EndNode->isEnd(x, y)) return true;
            nextY--;
            break;
        case JumpDir::DOWN:
            if ((!IsValidPathPosition(x - 1, y) && IsValidPathPosition(x - 1, y + 1)) || 
                (!IsValidPathPosition(x + 1, y) && IsValidPathPosition(x + 1, y + 1)) || EndNode->isEnd(x, y)) return true;
            nextY++;
            break;
        default: return false;
    }
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    return HasForcedNeighborStraight(nextX, nextY, dir);
}

void JpsEngine::ExploreDiagonal(JpsNode* const node, const int x, const int y, const UINT g, const JumpDir dir) {
    if (!IsValidPathPosition(x, y)) return;
    if (EndNode->isEnd(x, y)) {
        EndNode->SetParent(node);
        return;
    }
    
    UCHAR result = (UCHAR)dir;
    UINT nextG = g + 7;
    int nextX = x;
    int nextY = y;
    bool hasStraightIntersection = false;
    
    switch (dir) {
        case JumpDir::RIGHT_UP:
            if (!IsValidPathPosition(x - 1, y) && IsValidPathPosition(x - 1, y - 1)) result |= FLAG_FORCED_LEFT;
            if (!IsValidPathPosition(x, y + 1) && IsValidPathPosition(x + 1, y + 1)) result |= FLAG_FORCED_RIGHT;
            if ((result != (UCHAR)dir) || HasForcedNeighborStraight(x, y - 1, JumpDir::UP) || HasForcedNeighborStraight(x + 1, y, JumpDir::RIGHT)) {
                hasStraightIntersection = true;
            }
            nextX++; nextY--;
            break;
        case JumpDir::LEFT_UP:
            if (!IsValidPathPosition(x, y + 1) && IsValidPathPosition(x - 1, y + 1)) result |= FLAG_FORCED_LEFT;
            if (!IsValidPathPosition(x + 1, y) && IsValidPathPosition(x + 1, y - 1)) result |= FLAG_FORCED_RIGHT;
            if ((result != (UCHAR)dir) || HasForcedNeighborStraight(x - 1, y, JumpDir::LEFT) || HasForcedNeighborStraight(x, y - 1, JumpDir::UP)) {
                hasStraightIntersection = true;
            }
            nextX--; nextY--;
            break;
        case JumpDir::RIGHT_DOWN:
            if (!IsValidPathPosition(x, y - 1) && IsValidPathPosition(x + 1, y - 1)) result |= FLAG_FORCED_LEFT;
            if (!IsValidPathPosition(x - 1, y) && IsValidPathPosition(x - 1, y + 1)) result |= FLAG_FORCED_RIGHT;
             if ((result != (UCHAR)dir) || HasForcedNeighborStraight(x + 1, y, JumpDir::RIGHT) || HasForcedNeighborStraight(x, y + 1, JumpDir::DOWN)) {
                hasStraightIntersection = true;
            }
            nextX++; nextY++;
            break;
        case JumpDir::LEFT_DOWN:
             if (!IsValidPathPosition(x + 1, y) && IsValidPathPosition(x + 1, y + 1)) result |= FLAG_FORCED_LEFT;
            if (!IsValidPathPosition(x, y - 1) && IsValidPathPosition(x - 1, y - 1)) result |= FLAG_FORCED_RIGHT;
             if ((result != (UCHAR)dir) || HasForcedNeighborStraight(x, y + 1, JumpDir::DOWN) || HasForcedNeighborStraight(x - 1, y, JumpDir::LEFT)) {
                hasStraightIntersection = true;
            }
            nextX--; nextY++;
            break;
        default: return; // Should not happen
    }
    
    if (hasStraightIntersection) {
        RegisterJumpPoint(node, x, y, g, result);
        return;
    }
    
    exploredPositions.push_back(std::make_tuple(x, y, colorIndex));
    ExploreDiagonal(node, nextX, nextY, nextG, dir);
}

void JpsEngine::ExpandNextNodeFromQueue() {
    if (openList.empty()) return;
    
    JpsNode* temp = openList.top();
    openList.pop();
    
    if (temp->closed) {
        ExpandNextNodeFromQueue();
        return;
    }
    
    temp->closed = true;

    const int x = temp->GetcurX();
    const int y = temp->GetcurY();
    const UINT g = temp->GetG();

    UCHAR direction = temp->reason & 0x0f;
    UCHAR flag = temp->reason & 0xf0;

    switch (direction) {
    case (UCHAR)JumpDir::NONE: {
        ExploreStraight(temp, x + 1, y, g, JumpDir::RIGHT); if (EndNode->isRootFound()) return;
        ExploreDiagonal(temp, x + 1, y - 1, g, JumpDir::RIGHT_UP); if (EndNode->isRootFound()) return;
        ExploreStraight(temp, x, y - 1, g, JumpDir::UP); if (EndNode->isRootFound()) return;
        ExploreDiagonal(temp, x - 1, y - 1, g, JumpDir::LEFT_UP); if (EndNode->isRootFound()) return;
        ExploreStraight(temp, x - 1, y, g, JumpDir::LEFT); if (EndNode->isRootFound()) return;
        ExploreDiagonal(temp, x - 1, y + 1, g, JumpDir::LEFT_DOWN); if (EndNode->isRootFound()) return;
        ExploreStraight(temp, x, y + 1, g, JumpDir::DOWN); if (EndNode->isRootFound()) return;
        ExploreDiagonal(temp, x + 1, y + 1, g, JumpDir::RIGHT_DOWN);
        break;
    }
    case (UCHAR)JumpDir::UP: {
        ExploreStraight(temp, x, y - 1, g, JumpDir::UP);
        if (EndNode->isRootFound()) return;
        if (flag & FLAG_FORCED_LEFT) { ExploreDiagonal(temp, x - 1, y - 1, g, JumpDir::LEFT_UP); if (EndNode->isRootFound()) return; }
        if (flag & FLAG_FORCED_RIGHT){ ExploreDiagonal(temp, x + 1, y - 1, g, JumpDir::RIGHT_UP); }
        break;
    }
    case (UCHAR)JumpDir::RIGHT: {
        ExploreStraight(temp, x + 1, y, g, JumpDir::RIGHT);
        if (EndNode->isRootFound()) return;
        if (flag & FLAG_FORCED_LEFT) { ExploreDiagonal(temp, x + 1, y - 1, g, JumpDir::RIGHT_UP); if (EndNode->isRootFound()) return; }
        if (flag & FLAG_FORCED_RIGHT){ ExploreDiagonal(temp, x + 1, y + 1, g, JumpDir::RIGHT_DOWN); }
        break;
    }
    case (UCHAR)JumpDir::LEFT: {
        ExploreStraight(temp, x - 1, y, g, JumpDir::LEFT);
        if (EndNode->isRootFound()) return;
        if (flag & FLAG_FORCED_LEFT) { ExploreDiagonal(temp, x - 1, y + 1, g, JumpDir::LEFT_DOWN); if (EndNode->isRootFound()) return; }
        if (flag & FLAG_FORCED_RIGHT){ ExploreDiagonal(temp, x - 1, y - 1, g, JumpDir::LEFT_UP); }
        break;
    }
    case (UCHAR)JumpDir::DOWN: {
        ExploreStraight(temp, x, y + 1, g, JumpDir::DOWN);
        if (EndNode->isRootFound()) return;
        if (flag & FLAG_FORCED_LEFT) { ExploreDiagonal(temp, x + 1, y + 1, g, JumpDir::RIGHT_DOWN); if (EndNode->isRootFound()) return; }
        if (flag & FLAG_FORCED_RIGHT){ ExploreDiagonal(temp, x - 1, y + 1, g, JumpDir::LEFT_DOWN); }
        break;
    }
    case (UCHAR)JumpDir::LEFT_UP: {
        ExploreDiagonal(temp, x - 1, y - 1, g, JumpDir::LEFT_UP);
        if (EndNode->isRootFound()) return;
        ExploreStraight(temp, x - 1, y, g, JumpDir::LEFT);
        if (EndNode->isRootFound()) return;
        ExploreStraight(temp, x , y - 1, g, JumpDir::UP);
        if (EndNode->isRootFound()) return;
        if (flag & FLAG_FORCED_LEFT) { ExploreDiagonal(temp, x - 1, y + 1, g, JumpDir::LEFT_DOWN); if (EndNode->isRootFound()) return; }
        if (flag & FLAG_FORCED_RIGHT){ ExploreDiagonal(temp, x + 1, y - 1, g, JumpDir::RIGHT_UP); }
        break;
    }
    case (UCHAR)JumpDir::RIGHT_UP: {
        ExploreDiagonal(temp, x + 1, y - 1, g, JumpDir::RIGHT_UP);
        if (EndNode->isRootFound()) return;
        ExploreStraight(temp, x, y - 1, g, JumpDir::UP);
        if (EndNode->isRootFound()) return;
        ExploreStraight(temp, x + 1, y, g, JumpDir::RIGHT);
        if (EndNode->isRootFound()) return;
        if (flag & FLAG_FORCED_LEFT) { ExploreDiagonal(temp, x - 1, y - 1, g, JumpDir::LEFT_UP); if (EndNode->isRootFound()) return; }
        if (flag & FLAG_FORCED_RIGHT){ ExploreDiagonal(temp, x + 1, y + 1, g, JumpDir::RIGHT_DOWN); }
        break;
    }
    case (UCHAR)JumpDir::LEFT_DOWN: {
        ExploreDiagonal(temp, x - 1, y + 1, g, JumpDir::LEFT_DOWN);
        if (EndNode->isRootFound()) return;
        ExploreStraight(temp, x - 1, y, g, JumpDir::LEFT);
        if (EndNode->isRootFound()) return;
        ExploreStraight(temp, x, y + 1, g, JumpDir::DOWN);
        if (EndNode->isRootFound()) return;
        if (flag & FLAG_FORCED_LEFT) { ExploreDiagonal(temp, x + 1, y + 1, g, JumpDir::RIGHT_DOWN); if (EndNode->isRootFound()) return; }
        if (flag & FLAG_FORCED_RIGHT){ ExploreDiagonal(temp, x - 1, y - 1, g, JumpDir::LEFT_UP); }
        break;
    }
    case (UCHAR)JumpDir::RIGHT_DOWN: {
        ExploreDiagonal(temp, x + 1, y + 1, g, JumpDir::RIGHT_DOWN);
        if (EndNode->isRootFound()) return;
        ExploreStraight(temp, x + 1, y, g, JumpDir::RIGHT);
        if (EndNode->isRootFound()) return;
        ExploreStraight(temp, x, y + 1, g, JumpDir::DOWN);
        if (EndNode->isRootFound()) return;
        if (flag & FLAG_FORCED_LEFT) { ExploreDiagonal(temp, x + 1, y - 1, g, JumpDir::RIGHT_UP); if (EndNode->isRootFound()) return; }
        if (flag & FLAG_FORCED_RIGHT){ ExploreDiagonal(temp, x - 1, y + 1, g, JumpDir::LEFT_DOWN); }
        break;
    }
    }
    colorIndex = (colorIndex + 1) % (sizeof(colors) / sizeof(colors[0]));
}

void JpsEngine::VerifyDirectLineOfSight(JpsNode* const node) {
    if (!node->GetParent()) return;
    JpsNode* temp = node->GetParent()->GetParent();
    if (!temp) return;
    
    std::vector<Point> lt;
    BrezenHemLine::BrezenHem(node->GetcurX(), node->GetcurY(), temp->GetcurX(), temp->GetcurY(), lt);
    
    if (lt.empty()) {
        VerifyDirectLineOfSight(node->GetParent());
        return;
    }
    
    bool directLineOfSight = true;
    for (const auto& pt : lt) {
        if (!IsValidPathPosition(pt.x, pt.y)) {
            directLineOfSight = false;
            break;
        }
    }
    
    if (directLineOfSight) {
        node->SetParent(temp);
        VerifyDirectLineOfSight(node);
    } else {
        VerifyDirectLineOfSight(node->GetParent());
    }
}

// ---------------- UI Rendering Operations ---------------- //

void JpsEngine::DrawNode(HDC& hdc, HBRUSH& hTileBrush, const JpsNode* const node, const int xOffset, const int yOffset, const int color) const {
    const int x = node->GetcurX() * GRID_SIZE + xOffset;
    const int y = node->GetcurY() * GRID_SIZE + yOffset;
    
    HPEN oldBrush = (HPEN)SelectObject(hdc, hTileBrush);
    SelectObject(hdc, GetStockObject(DC_BRUSH));
    if (color == 1) SetDCBrushColor(hdc, RGB(0, 0, 255));
    else if (color == 2) SetDCBrushColor(hdc, RGB(255, 100, 0));
    
    Rectangle(hdc, x, y, x + GRID_SIZE + 2, y + GRID_SIZE + 2);
    
    if (GRID_SIZE >= 56) {
        memset(numG, L' ', 64);
        int j = swprintf_s(numG, 64, L" G: %d", node->GetG());
        TextOutW(hdc, x, y, numG, j);
        memset(numH, L' ', 64);
        j = swprintf_s(numH, 64, L" H: %d", node->GetH());
        TextOutW(hdc, x, y + 20, numH, j);
        memset(numF, L' ', 64);
        j = swprintf_s(numF, 64, L" F: %d", node->GetF());
        TextOutW(hdc, x, y + 40, numF, j);
    }
    SetDCBrushColor(hdc, RGB(100, 100, 100));
    SelectObject(hdc, oldBrush);
}

void JpsEngine::DrawRoot(HDC& hdc, HBRUSH& hTileBrush, HPEN& hGridPen, const int xOffset, const int yOffset) const {
    if (!EndNode) return;
    const JpsNode* r = EndNode;
    int x = ((r->GetcurX() * GRID_SIZE) + GRID_SIZE / 2) + xOffset;
    int y = ((r->GetcurY() * GRID_SIZE) + GRID_SIZE / 2) + yOffset;

    HPEN oldPen = (HPEN)SelectObject(hdc, hGridPen);
    HPEN curPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
    SelectObject(hdc, curPen);
    while (r->GetParent() != nullptr) {
        MoveToEx(hdc, x, y, NULL);
        r = r->GetParent();
        x = ((r->GetcurX() * GRID_SIZE) + GRID_SIZE / 2) + xOffset;
        y = ((r->GetcurY() * GRID_SIZE) + GRID_SIZE / 2) + yOffset;
        LineTo(hdc, x, y);
    }
    DeleteObject(curPen);
    SelectObject(hdc, oldPen);
}

void JpsEngine::DrawRectangles(HDC hdc, const int xOffset, const int yOffset) const {
    SelectObject(hdc, GetStockObject(DC_BRUSH));
    for (const auto& pos : exploredPositions) {
        int x = std::get<0>(pos);
        int y = std::get<1>(pos);
        int cIdx = std::get<2>(pos);
        x = x * GRID_SIZE + xOffset;
        y = y * GRID_SIZE + yOffset;
        SetDCBrushColor(hdc, colors[cIdx]);
        Rectangle(hdc, x, y, x + GRID_SIZE + 2, y + GRID_SIZE + 2);
    }
}

void JpsEngine::RenderAll(HDC& hdc, HBRUSH& hTileBrush, HPEN& hGridPen, const int xOffset, const int yOffset) const {
    DrawRectangles(hdc, xOffset, yOffset);
    
    HPEN oldBrush = (HPEN)SelectObject(hdc, hTileBrush);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            if (gridNodes[i][j]) {
                if (gridNodes[i][j]->closed) DrawNode(hdc, hTileBrush, gridNodes[i][j], xOffset, yOffset, 2);
                else DrawNode(hdc, hTileBrush, gridNodes[i][j], xOffset, yOffset, 1);
            }
        }
    }
    
    if (EndNode && EndNode->isRootFound()) {
        DrawRoot(hdc, hTileBrush, hGridPen, xOffset, yOffset);
    }
    
    int x, y;
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            if (!Tile[i][j]) continue;
            x = j * GRID_SIZE + xOffset;
            y = i * GRID_SIZE + yOffset;
            if (Tile[i][j] == 1) {
                Rectangle(hdc, x, y, x + GRID_SIZE + 2, y + GRID_SIZE + 2);
            }
            if (Tile[i][j] == 2) {
                SetDCBrushColor(hdc, RGB(0, 255, 0));
                SelectObject(hdc, GetStockObject(DC_BRUSH));
                Rectangle(hdc, x, y, x + GRID_SIZE + 2, y + GRID_SIZE + 2);
                SetDCBrushColor(hdc, RGB(100, 100, 100));
            }
            if (Tile[i][j] == 3) {
                SetDCBrushColor(hdc, RGB(255, 0, 0));
                SelectObject(hdc, GetStockObject(DC_BRUSH));
                Rectangle(hdc, x, y, x + GRID_SIZE + 2, y + GRID_SIZE + 2);
                SetDCBrushColor(hdc, RGB(100, 100, 100));
            }
        }
    }
    
    SelectObject(hdc, oldBrush);
}