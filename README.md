# 3D Maze — OpenGL 미로 게임 & 자체 회전체 모델러

2023년 중앙대학교 컴퓨터그래픽스 기말 프로젝트 (개인 작업)

C++ / OpenGL 고정 파이프라인으로 구현한 3D 미로 탐험 게임과, 게임에 사용할 3D 모델을
만들기 위해 직접 제작한 회전체(Surface of Revolution) 모델러입니다.
모델 데이터 생성부터 런타임 렌더링까지의 파이프라인을 전부 자체 구현했습니다.

```
SOR.exe (모델러)  ──►  *.dat (정점 + 면 인덱스)  ──►  Final.exe (런타임 로드)
```

## 스크린샷

_추가 예정_

## 실행

| 파일 | 설명 |
| --- | --- |
| `bin/Final.exe` | 미로 게임 본편 |
| `bin/SOR.exe` | 회전체 모델러 |

`freeglut.dll`이 함께 들어 있어 별도 설치 없이 실행됩니다. (Windows)

### 조작 — Final.exe

| 입력 | 동작 |
| --- | --- |
| `W` `A` `S` `D` | 전진 / 좌 / 후진 / 우 |
| 마우스 좌클릭 드래그 | 시점 회전 |
| `R` | 시작 지점으로 복귀 |
| `ESC` | 종료 |

### 목표

중앙 구역에서 이어지는 좌·우·후방 3개 구역에 배터리가 하나씩 숨겨져 있습니다.
3개를 모두 모으면 중앙 정면의 잠금 장치가 해제되고, 그 너머의 트로피에 도달하면 클리어입니다.

### 조작 — SOR.exe

화면을 클릭해 정점을 찍고, 우클릭 메뉴로 기능을 실행합니다.

| 메뉴 | 동작 |
| --- | --- |
| `rotate` | 360의 약수를 입력받아 해당 각도씩 y축 회전, 회전체 정점군 생성 |
| `make face` | 정점 인덱스 3개 단위의 면(Face) 구조체 배열 생성 |
| `connect` | 생성된 면 순서대로 정점을 연결해 결과 폴리곤 미리보기 |
| `scale` | 정점 좌표 배율 조정 |
| `save` | 정점과 면을 `.dat`로 저장 |
| `clear` / `exit` | 초기화 / 종료 |

## 구조

### 클래스 계층

```
MapObject                        기반 클래스
├─ position, direction, radius
├─ polygonName (string)          .dat 로드 키
├─ readModel()                   .dat 파서
├─ render()                      폴리곤 렌더링
└─ checkCollisionWithCamera()    거리 기반 획득 판정
     └ 사용처: 배터리 3, 트로피, 장식 행성 3, 행성 링 2

PlaneObject : MapObject          벽 / 바닥 / 천장
├─ direction_height, width, height
├─ texture (int)                 값에 따라 이미지와 반복 패턴 결정
├─ render()                      텍스처 매핑된 사각형
└─ collisionNavigation()         벽면 슬라이딩 충돌 처리
     └ 사용처: 133개 (Main 59 / Right 22 / Left 24 / Back 27 / Lock 1)
```

### 벽 충돌 — 슬라이딩

단순 차단이 아니라 벽을 따라 미끄러지도록 처리했습니다.

1. 벽의 `direction`과 `direction_height`를 외적해 법선을 구함
2. 이동 예정 지점과 벽 평면의 거리, 그리고 벽의 너비·높이 범위 안에 드는지 검사
3. 충돌 시 이동 벡터를 벽의 `direction`에 투영 — 벽과 평행한 성분만 남김

### 구역 스트리밍 (히스테리시스)

맵 전체(133개 벽)를 항상 유지하면 렌더링과 충돌 검사 비용이 커지므로,
카메라 좌표에 따라 구역 단위로 객체를 생성·해제합니다.

이때 경계에서 생성과 해제가 반복되는 스래싱을 막기 위해 **생성 임계값과 해제 임계값을
다르게** 두었습니다.

```cpp
if (cameraZ >  40.0f && !isRightGenerated) { generateWallsRight(); isRightGenerated = true;  }
if (cameraZ <  30.0f &&  isRightGenerated) { releaseWallsRight();  isRightGenerated = false; }
//         ^^ 10 단위의 히스테리시스 밴드
```

### `.dat` 포맷

텍스트 기반이며 `SOR.exe`가 출력하고 `MapObject::readModel()`이 파싱합니다.

```
VERTEX = <정점 수>
<x> <y> <z>          × 정점 수
FACE = <면 수>
<i0> <i1> <i2>       × 면 수
```

| 파일 | 정점 | 면 | 용도 |
| --- | ---: | ---: | --- |
| `trophy.dat` | 576 | 1152 | 트로피 (최종 목표) |
| `battery.dat` | 270 | 540 | 배터리 아이템 |
| `planet3.dat` | 192 | 384 | 장식 행성 |
| `polygon.dat` | 162 | 324 | 테스트 모델 |
| `planet1.dat` `planet2.dat` | 180 | 360 | 장식 행성 |
| `planet_ring.dat` `planet_ring2.dat` | 160 | 320 | 행성 고리 |

모든 모델이 `면 = 정점 × 2`입니다. 회전체의 사각형 세그먼트를 삼각형 2개로 분할하는
구조에서 나오는 결과입니다.

## 이후 리팩터링

당시 개발 중 "벽면 텍스처가 무작위로 다른 이미지로 바뀌는" 현상이 있었고, 그때는
원인을 특정하지 못한 채 거리 컬링으로 증상만 완화했습니다. 이후 코드를 다시 분석해
근본 원인을 규명하고 정리한 내역입니다.

| 커밋 | 내용 |
| --- | --- |
| `fix` | `PlaneObject::textureID` 미초기화 수정 — 생성자에서 초기화되지 않은 값을 `glDeleteTextures`에 넘겨 **다른 객체의 텍스처를 삭제**하고 있었음. 텍스처 오염의 근본 원인 |
| `perf` | 텍스처 캐싱 도입 — 벽마다 새 ID를 발급하던 것을 경로 기준으로 공유. 텍스처 생성 **133개 → 7개** |
| `perf` | 면 단위 `glBegin`/`glEnd`/`glPolygonMode` 호출을 루프 밖으로. `trophy.dat` 기준 프레임당 **1152쌍 → 1쌍** |
| `fix` | 모델 로드 실패 시 미초기화 `pnum`/`fnum`으로 순회하던 문제, `delete[]` 후 댕글링 포인터 방지 |
| `fix` | 잠금 해제 후 `lockExists` 플래그를 내리지 않아 매 입력마다 해제 함수가 반복 호출되던 문제 |
| `refactor` | `glm::radians(glm::degrees(x))` 무의미한 단위 왕복 제거 |

`bin/` 안의 실행 파일은 **2023년 제출 당시 빌드본**이며 위 수정은 반영되어 있지 않습니다.

## 빌드

- Visual Studio 2022 (MSVC v14.37)
- 의존성: FreeGLUT, GLM, SOIL

`src/Final.cpp` 단일 파일입니다. 프로젝트 파일(`.sln`/`.vcxproj`)은 포함하지 않았으므로
새로 생성한 뒤 위 라이브러리를 링크해야 합니다. 실행 파일은 `bin/`의 상위 디렉터리에서
구동되는 것을 전제로 리소스 경로가 잡혀 있습니다.

> `SOR.exe`의 소스는 유실되어 포함되어 있지 않습니다. 실행 파일만 제공합니다.

## 문서

- [`docs/report.pdf`](docs/report.pdf) — 당시 제출한 9페이지 설계 보고서
- [`docs/original_README.txt`](docs/original_README.txt) — 제출 당시 작성한 안내문

## 라이선스

개인 학습 목적의 프로젝트입니다. 코드는 자유롭게 참고하셔도 됩니다.
