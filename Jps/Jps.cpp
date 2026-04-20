#include "framework.h"
#include "JPS.h"
#include "JpsClass.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_JPS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_JPS));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_JPS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_JPS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; 
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd) return FALSE;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

HDC hdc;
HBITMAP hMemDcBitMap;
HBITMAP hMemDcBitMapOld;
HDC hMemDc;
RECT MemDcRect;

bool isErase = false;
bool isDrag = false;

HBRUSH hTileBrush;
HPEN hGridPen;

int xOffset = 0;
int yOffset = 0;

JpsEngine engine;

WCHAR text[] = L"왼쪽 더블 클릭: start 지정 / 오른쪽 더블 클릭 : end 지정 / 마우스 휠 버튼: 경로 순차 탐색 / S : 즉시 탐색 / R: 초기화 / 엔터: 맵 전체 지우기";

void GridRender(HDC& hdc, HPEN& hGridPen, int xOffset, int yOffset) {
    int x = xOffset;
    int y = yOffset;
    HPEN oldPen = (HPEN)SelectObject(hdc, hGridPen);
    for (int i = 0; i <= GRID_WIDTH; i++) {
        MoveToEx(hdc, x, yOffset, NULL);
        LineTo(hdc, x, GRID_HEIGHT * engine.GRID_SIZE + yOffset);
        x += engine.GRID_SIZE;
    }
    for (int j = 0; j <= GRID_HEIGHT; j++) {
        MoveToEx(hdc, xOffset, y, NULL);
        LineTo(hdc, GRID_WIDTH * engine.GRID_SIZE + xOffset, y);
        y += engine.GRID_SIZE;
    }
    SelectObject(hdc, oldPen);
}

#define TIMER_ID 1
#define TIMER_INTERVAL 100

bool searchInProgress = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE: {
        int x = 15;
        int y = 20;
        engine.SetStart(x, y);
        engine.SetEnd(x + 30, y);
        
        hdc = GetDC(hWnd);
        GetClientRect(hWnd, &MemDcRect);
        hMemDcBitMap = CreateCompatibleBitmap(hdc, MemDcRect.right, MemDcRect.bottom);
        hMemDc = CreateCompatibleDC(hdc);
        ReleaseDC(hWnd, hdc);
        hMemDcBitMapOld = (HBITMAP)SelectObject(hMemDc, hMemDcBitMap);

        hGridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        hTileBrush = CreateSolidBrush(RGB(100, 100, 100));
        break;
    }
    case WM_LBUTTONDOWN: {
        isDrag = true;
        int x = GET_X_LPARAM(lParam) - xOffset;
        int y = GET_Y_LPARAM(lParam) - yOffset;
        int tileX = x / engine.GRID_SIZE;
        int tileY = y / engine.GRID_SIZE;
        if (tileX < 0 || tileY < 0 || tileX >= GRID_WIDTH || tileY >= GRID_HEIGHT) break;
        
        if (engine.Tile[tileY][tileX] == 1) isErase = true;
        else isErase = false;
        break;
    }
    case WM_LBUTTONUP: {
        isDrag = false;
        break;
    }
    case WM_MOUSEMOVE: {
        if (isDrag) {
            int x = GET_X_LPARAM(lParam) - xOffset;
            int y = GET_Y_LPARAM(lParam) - yOffset;
            int tileX = x / engine.GRID_SIZE;
            int tileY = y / engine.GRID_SIZE;
            if (tileX < 0 || tileY < 0 || tileX >= GRID_WIDTH || tileY >= GRID_HEIGHT) break;
            
            if (engine.Tile[tileY][tileX] < 2) {
                engine.Tile[tileY][tileX] = !isErase ? 1 : 0;
            }
            InvalidateRect(hWnd, NULL, false);
        }
        break;
    }
    case WM_LBUTTONDBLCLK: {
        int x = GET_X_LPARAM(lParam) - xOffset;
        int y = GET_Y_LPARAM(lParam) - yOffset;
        int tileX = x / engine.GRID_SIZE;
        int tileY = y / engine.GRID_SIZE;
        if (tileX < 0 || tileY < 0 || tileX >= GRID_WIDTH || tileY >= GRID_HEIGHT) break;
        
        engine.ClearLists();
        engine.SetStart(tileX, tileY);
        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_RBUTTONDBLCLK: {
        int x = GET_X_LPARAM(lParam) - xOffset;
        int y = GET_Y_LPARAM(lParam) - yOffset;
        int tileX = x / engine.GRID_SIZE;
        int tileY = y / engine.GRID_SIZE;
        if (tileX < 0 || tileY < 0 || tileX >= GRID_WIDTH || tileY >= GRID_HEIGHT) break;
        
        engine.ClearLists();
        engine.SetEnd(tileX, tileY);
        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_MBUTTONDOWN: { // MMB for starting animation
        engine.PrepareSearch();
        searchInProgress = true;
        SetTimer(hWnd, TIMER_ID, TIMER_INTERVAL, NULL);
        break;
    }
    case WM_TIMER: {
        if (wParam == TIMER_ID) {
            if (!engine.isSearchFinished()) {
                engine.ExpandNextNodeFromQueue();
            } else {
                KillTimer(hWnd, TIMER_ID);
                searchInProgress = false;
            }
            InvalidateRect(hWnd, NULL, false);
        }
        break;
    }
    case WM_KEYDOWN: {
        switch (wParam) {
        case VK_RETURN: {
            engine.InitializeGridMap();
            engine.SetStart(engine.StartNode ? engine.StartNode->GetcurX() : 15, engine.StartNode ? engine.StartNode->GetcurY() : 20);
            engine.SetEnd(engine.EndNode ? engine.EndNode->GetcurX() : 45, engine.EndNode ? engine.EndNode->GetcurY() : 20);
            engine.ClearLists();
            break;
        }
        case VK_LEFT:   xOffset += MOVE_STEP; break;
        case VK_RIGHT:  xOffset -= MOVE_STEP; break;
        case VK_UP:     yOffset += MOVE_STEP; break;
        case VK_DOWN:   yOffset -= MOVE_STEP; break;
        case 0x52: { // R key
            engine.ClearLists();
            break;
        }
        case 0x53: { // S key (Search Instantly)
            engine.PrepareSearch();
            while (!engine.isSearchFinished()) {
                engine.ExpandNextNodeFromQueue();
            }
            break;
        }
        }
        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_MOUSEWHEEL: {
        if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) engine.GRID_SIZE += SIZE_CHANGE;
        else engine.GRID_SIZE -= SIZE_CHANGE;
        if (engine.GRID_SIZE < 8) engine.GRID_SIZE = 8; // Prevent too small
        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        PatBlt(hMemDc, 0, 0, MemDcRect.right, MemDcRect.bottom, WHITENESS);
        
        // Draw everything
        engine.RenderAll(hMemDc, hTileBrush, hGridPen, xOffset, yOffset);
        GridRender(hMemDc, hGridPen, xOffset, yOffset);

        hdc = BeginPaint(hWnd, &ps);
        BitBlt(hdc, 0, 0, MemDcRect.right, MemDcRect.bottom, hMemDc, 0, 0, SRCCOPY);
        TextOutW(hdc, 10, 10, text, (int)wcslen(text));
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_SIZE: {
        SelectObject(hMemDc, hMemDcBitMapOld);
        DeleteObject(hMemDc);
        DeleteObject(hMemDcBitMap);

        hdc = GetDC(hWnd);
        GetClientRect(hWnd, &MemDcRect);
        hMemDcBitMap = CreateCompatibleBitmap(hdc, MemDcRect.right, MemDcRect.bottom);
        hMemDc = CreateCompatibleDC(hdc);
        ReleaseDC(hWnd, hdc);

        hMemDcBitMapOld = (HBITMAP)SelectObject(hMemDc, hMemDcBitMap);
        break;
    }
    case WM_DESTROY: {
        SelectObject(hMemDc, hMemDcBitMapOld);
        DeleteObject(hMemDc);
        DeleteObject(hMemDcBitMap);
        DeleteObject(hTileBrush);
        DeleteObject(hGridPen);
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
