// JPS.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "JPS.h"
#include "JpsClass.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_JPS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_JPS));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
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

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//


HDC hdc;
HBITMAP hMemDcBitMap;
HBITMAP hMemDcBitMapOld;
HDC hMemDc;

HBITMAP hRectMemDcBitMap;
HBITMAP hRectMemDcBitMapOld;
HDC hRectMemDc;
RECT MemDcRect;

bool isErase = false;
bool isDrag = false;

HBRUSH hTileBrush;
HPEN hGridPen;

int xOffset;
int yOffset;

JpsNode* Start;
JpsNode* End;

JpsNodeList openList;
JpsNodeList closeList;

WCHAR Tile[GRID_HEIGHT][GRID_WIDTH];
WCHAR text[] = L"왼쪽 더블 클릭: start 지정 / 오른쪽 더블 클릭 : end 지정 / 마우스 휠 버튼: 경로 자동 순차 탐색 / R: open,close list clear / S : 경로 탐색 / 스페이스: 경로 순차 탐색 / 엔터: start,end 제외 모든 노드 clear";
//int GRID_SIZE = 16;

void ObstacleRender(HDC& hdc, HBRUSH& hTileBrush, HPEN& hGridPen, int xOffset, int yOffset) {
    int x = 0;
    int y = 0;
    HPEN oldBrush = (HPEN)SelectObject(hdc, hTileBrush);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    openList.Draw(hdc, hTileBrush, xOffset, yOffset,1);
    closeList.Draw(hdc, hTileBrush, xOffset, yOffset,2);
    if (End->isRootFound()) {
        //GetBrezenHemRoot(*End);
        End->DrawRoot(hdc,hTileBrush, hGridPen, xOffset, yOffset);
    }
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            if (!Tile[i][j]) {
                continue;
            }
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

void GridRender(HDC& hdc, HPEN& hGridPen, int xOffset, int yOffset) {
    int x = xOffset;
    int y = yOffset;
    HPEN oldPen = (HPEN)SelectObject(hdc, hGridPen);
    for (int i = 0; i <= GRID_WIDTH; i++) {
        MoveToEx(hdc, x, yOffset, NULL);
        LineTo(hdc, x, GRID_HEIGHT * GRID_SIZE + yOffset);
        x += GRID_SIZE;
    }
    for (int j = 0; j <= GRID_HEIGHT; j++) {
        MoveToEx(hdc, xOffset, y, NULL);
        LineTo(hdc, GRID_WIDTH * GRID_SIZE + xOffset, y);
        y += GRID_SIZE;
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
            Tile[y][x] = 2;
            Tile[y][x + 30] = 3;
            SetStart(x, y);
            SetEnd(x + 30, y);
            openList.InsertNode(*Start);
            hdc = GetDC(hWnd);
            GetClientRect(hWnd, &MemDcRect);
            hMemDcBitMap = CreateCompatibleBitmap(hdc, MemDcRect.right, MemDcRect.bottom);
            hMemDc = CreateCompatibleDC(hdc);
            ReleaseDC(hWnd, hdc);
            hMemDcBitMapOld = (HBITMAP)SelectObject(hMemDc, hMemDcBitMap);

            hGridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
            hTileBrush = CreateSolidBrush(RGB(100, 100, 100));

            //PatBlt(hMemDc, 0, 0, MemDcRect.right, MemDcRect.bottom, WHITENESS);

            break;
        }
        case WM_LBUTTONDOWN: {
            isDrag = true;
            int x = GET_X_LPARAM(lParam) - xOffset;
            int y = GET_Y_LPARAM(lParam) - yOffset;
            int tileX = x / GRID_SIZE;
            int tileY = y / GRID_SIZE;
            if (tileX < 0 || tileY < 0 || tileX >= GRID_WIDTH || tileY >= GRID_HEIGHT) {
                break;
            }
            if (Tile[tileY][tileX] == 1) {
                isErase = true;
            }
            else {
                isErase = false;
            }
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
                int tileX = x / GRID_SIZE;
                int tileY = y / GRID_SIZE;
                if (tileX < 0 || tileY < 0 || tileX>=GRID_WIDTH || tileY >=GRID_HEIGHT) {
                    break;
                }
                if (Tile[tileY][tileX] < 2) {
                    Tile[tileY][tileX] = !isErase;
                }
                InvalidateRect(hWnd, NULL, false);
            }
            break;
        }
        case WM_LBUTTONDBLCLK: {
            int x = GET_X_LPARAM(lParam)- xOffset;
            int y = GET_Y_LPARAM(lParam)- yOffset;
            int tileX = x / GRID_SIZE;
            int tileY = y / GRID_SIZE;
            if (tileX < 0 || tileY < 0 || tileX >= GRID_WIDTH || tileY >= GRID_HEIGHT) {
                break;
            }
            openList.ClearList();
            closeList.ClearList();
            SetStart(tileX, tileY);
            openList.InsertNode(*Start);
            End->SetParent();
            InvalidateRect(hWnd, NULL, false);
            break;
        }
        case WM_RBUTTONDBLCLK: {
            int x = GET_X_LPARAM(lParam) - xOffset;
            int y = GET_Y_LPARAM(lParam) - yOffset;
            int tileX = x / GRID_SIZE;
            int tileY = y / GRID_SIZE;
            if (tileX < 0 || tileY < 0 || tileX >= GRID_WIDTH || tileY >= GRID_HEIGHT) {
                break;
            }
            openList.ClearList();
            closeList.ClearList();
            openList.InsertNode(*Start);
            SetEnd(tileX, tileY);
            End->SetParent();
            InvalidateRect(hWnd, NULL, false);
            break;
        }
        case WM_MBUTTONDOWN: {
            openList.ClearList();
            closeList.ClearList();
            openList.InsertNode(*Start);
            End->SetParent();
            exploredPositions.clear();
            searchInProgress = true;
            SetTimer(hWnd, TIMER_ID, TIMER_INTERVAL, NULL);
            
            //while (!End->isRootFound() && !openList.isEmpty()) {
            //    SearchRoot();
            //    InvalidateRect(hWnd, NULL, false);
            //}
            //InvalidateRect(hWnd, NULL, false);
            break;
        }
        case WM_TIMER: {
            if (wParam == TIMER_ID) {
                if (!End->isRootFound() && !openList.isEmpty()) {
                    SearchRoot(hRectMemDc,hTileBrush,xOffset,yOffset);
                }
                else {
                    KillTimer(hWnd, TIMER_ID);
                    searchInProgress = false;
                }
                InvalidateRect(hWnd, NULL, false);
            }
            break;
        }

                           //------키, 휠 관련
        case WM_KEYDOWN: {
            switch (wParam) {
            case VK_RETURN: {
                for (int i = 0; i < GRID_HEIGHT; i++) {
                    wmemset(Tile[i], 0, GRID_WIDTH);
                }
                openList.ClearList();
                closeList.ClearList();
                SetStart(Start->GetcurX(), Start->GetcurY());
                SetEnd(End->GetcurX(), End->GetcurY());
                openList.InsertNode(*Start);
                End->SetParent();
                exploredPositions.clear();
                break;
            }
            case VK_SPACE: {
                if (!End->isRootFound() && !openList.isEmpty()) {
                    SearchRoot(hRectMemDc, hTileBrush, xOffset, yOffset);
                }
                break;
            }
            case VK_LEFT: {
                xOffset += MOVE_STEP;
                //InvalidateRect(hWnd, NULL, false);
                break;
            }
            case VK_RIGHT: {
                xOffset -= MOVE_STEP;
                //InvalidateRect(hWnd, NULL, false);
                break;
            }
            case VK_UP: {
                yOffset += MOVE_STEP;
                //InvalidateRect(hWnd, NULL, false);
                break;
            }
            case VK_DOWN: {
                yOffset -= MOVE_STEP;
                //InvalidateRect(hWnd, NULL, false);
                break;
            }
            case 0x52: {
                openList.ClearList();
                closeList.ClearList();
                SetStart(Start->GetcurX(), Start->GetcurY());
                SetEnd(End->GetcurX(), End->GetcurY());
                openList.InsertNode(*Start);
                End->SetParent();

                exploredPositions.clear();
                break;
            }
            case 0x53: {
                openList.ClearList();
                closeList.ClearList();
                openList.InsertNode(*Start);
                End->SetParent();
                exploredPositions.clear();
                while (!End->isRootFound() && !openList.isEmpty()) {
                    SearchRoot(hRectMemDc, hTileBrush, xOffset, yOffset);
                }
                break;
            }
            }
            InvalidateRect(hWnd, NULL, false);
            break;
        }
                       //grid 사이즈 변경
        case WM_MOUSEWHEEL: {
            if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) {
                GRID_SIZE += SIZE_CHANGE;
            }
            else {
                GRID_SIZE -= SIZE_CHANGE;
            }
            InvalidateRect(hWnd, NULL, false);
            break;
        }
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            PatBlt(hMemDc, 0, 0, MemDcRect.right, MemDcRect.bottom, WHITENESS);
            DrawRectangles(hMemDc,hTileBrush, xOffset, yOffset);
            ObstacleRender(hMemDc,hTileBrush,hGridPen,xOffset,yOffset);
            GridRender(hMemDc,hGridPen,xOffset,yOffset);

            hdc = BeginPaint(hWnd, &ps);
            BitBlt(hdc, 0, 0, MemDcRect.right, MemDcRect.bottom, hMemDc, 0, 0, SRCCOPY);
            TextOutW(hdc, 10, 10, text, wcslen(text));
            EndPaint(hWnd, &ps);
        }
        break;
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
        case WM_DESTROY:
            SelectObject(hMemDc, hMemDcBitMapOld);

            DeleteObject(hMemDc);
            DeleteObject(hMemDcBitMap);

            DeleteObject(hTileBrush);
            DeleteObject(hGridPen);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
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
