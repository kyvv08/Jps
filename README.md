# Jump Point Search (JPS) Algorithm Visualizer

## 📖 프로젝트 소개
격자 기반(Grid-based) 타일맵 환경에서 최단 경로를 고속으로 탐색하는 **Jump Point Search (JPS) 알고리즘**을 구현하고 시각화한 프로젝트입니다. 기존 A* 알고리즘의 불필요한 노드 탐색을 줄여 성능을 최적화하는 과정을 직관적으로 확인할 수 있으며, 단계별 탐색 애니메이션 및 즉시 탐색 기능을 지원합니다.

<img width="1600" height="846" alt="image" src="https://github.com/user-attachments/assets/89b58d43-5316-4b80-b1f9-1b17272bbbc0" />
<img width="1589" height="795" alt="image" src="https://github.com/user-attachments/assets/f3555967-8d93-4a13-8f3f-f3b36cb902e5" />

## 🛠 기술 스택
- **Language**: C++ 
- **Graphics/UI**: Win32 API (GDI)
- **Data Structures**: C++ STL (`std::priority_queue`, `std::vector`, `std::tuple`)

## 🎮 주요 기능 및 조작법
| 입력 | 동작 |
|---|---|
| **더블 클릭 (좌/우)** | 시작점(Start) / 도착점(End) 지정 |
| **마우스 드래그 (좌클릭)** | 장애물(벽) 생성 및 지우기 |
| **마우스 휠** | 맵 확대 / 축소 (Zoom In/Out) |
| **방향키 (↑↓←→)** | 화면 이동 (Panning) |
| **마우스 휠 버튼 (클릭)** | 경로 탐색 과정 순차적(단계별) 시각화 |
| **S 키** | 즉시 경로 탐색 |
| **R 키 / Enter 키** | 탐색 초기화 / 맵 전체 초기화 |

## 🧠 아키텍처 및 디자인 패턴
- **데이터 기반 상태 관리 (Data-Driven State Management)**: 타일맵의 상태를 2차원 배열과 클래스 포인터(`JpsNode*`)로 매핑하여 관리.
- **비트 연산(Bitwise Operation) 플래그**: 이웃 노드 검사 시 빠른 상태 검증을 위해 비트 플래그(`FLAG_FORCED_RIGHT`, `FLAG_FORCED_LEFT`)와 열거형(`JumpDir`)을 혼합 사용.
- **커스텀 비교 연산자 (Custom Functor)**: `NodeCompare` 구조체를 구현하여 `std::priority_queue`가 A* 휴리스틱 기반 최소 힙(Min-Heap)으로 동작하도록 설계.

## 📚 학습 및 적용된 주요 개념
1. **Jump Point Search (JPS) & A* Algorithm**
   - 직선 및 대각선 방향으로 장애물이나 '강제 이웃(Forced Neighbor)'을 만날 때까지 노드 확장을 건너뛰는(Jump) 최적화 기법 적용.
   - F = G + H 비용 함수 계산 및 맨해튼 거리(Manhattan Distance) 기반의 휴리스틱(H) 구현.
2. **브레슨햄 선 그리기 알고리즘 (Bresenham's Line Algorithm)**
   - `VerifyDirectLineOfSight` 함수에 적용하여, 두 노드 간의 직접적인 시야(Line of Sight)가 확보되는지 검증할 때 부동소수점 연산 없이 정수 연산만으로 경로의 유효성을 체크.
3. **Win32 API 더블 버퍼링 (Double Buffering)**
   - `HMemDc`와 메모리 비트맵을 생성하여 백 버퍼에 렌더링한 후, `BitBlt`으로 화면에 한 번에 출력함으로써 렌더링 깜빡임(Flickering) 현상 제거.
4. **이벤트 드리븐 시스템 (Event-Driven System)**
   - `WM_TIMER`를 활용한 탐색 과정 애니메이션 루프 구현 및 마우스/키보드 메세지 큐 비동기 처리.
