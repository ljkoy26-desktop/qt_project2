# 커밋 로그 (실제 커밋 아님 - 기록용)

- 일자: 2026-08-09
- 작업 지시: 사용자 요청 - 프로그램 이름을 "SpriteForge"로 결정, 프로젝트명까지 반영
  (작업 폴더/저장소 이름은 그대로 유지, 프로그램 자체의 이름만 변경하기로 확인)
- 대상 파일: `CMakeLists.txt`, `mainwindow.cpp`, `main.cpp`, `build_release.ps1`

## 커밋 메시지 (예시)

```
SpriteForge: 프로그램 이름을 qt_image_maker에서 SpriteForge로 변경

- CMakeLists.txt: project()/실행 파일 타겟명을 qt_image_maker -> SpriteForge로 변경
  (qt_add_executable/add_executable/add_library, target_link_libraries,
  set_target_properties, install(TARGETS), qt_finalize_executable 전부 반영)
- macOS 번들 ID를 com.example.qt_image_maker -> com.example.spriteforge로 변경
- mainwindow.cpp: 메인 창 제목을 "스프라이트시트 뷰어/편집기" -> "SpriteForge"로 변경
- main.cpp: QApplication::setApplicationName/setApplicationDisplayName("SpriteForge") 추가
- build_release.ps1: 배포용 실행 파일 이름을 qt_image_maker.exe -> SpriteForge.exe로 변경,
  스크립트 상단 설명(SYNOPSIS) 문구도 동일하게 수정
- build 디렉터리를 CMake 재구성(reconfigure)하고, 이전 타겟명(qt_image_maker)으로 남아있던
  build/qt_image_maker.exe, .ilk, .pdb, qt_image_maker_autogen 등 낡은 빌드 산출물 정리
```

## 변경 요약

1. `CMakeLists.txt`
   - `project(qt_image_maker ...)` -> `project(SpriteForge ...)`
   - 실행 파일/라이브러리 타겟명(`qt_add_executable`, `add_library`, `add_executable`,
     `target_link_libraries`, `set_target_properties`, `install(TARGETS ...)`,
     `qt_finalize_executable`)을 전부 `SpriteForge`로 변경
   - `MACOSX_BUNDLE_GUI_IDENTIFIER`를 `com.example.qt_image_maker` -> `com.example.spriteforge`로 변경

2. `mainwindow.cpp`
   - `setWindowTitle(QStringLiteral("스프라이트시트 뷰어/편집기"))` -> `setWindowTitle(QStringLiteral("SpriteForge"))`

3. `main.cpp`
   - `QApplication a(argc, argv);` 다음 줄에 `a.setApplicationName(QStringLiteral("SpriteForge"))`,
     `a.setApplicationDisplayName(QStringLiteral("SpriteForge"))` 추가

4. `build_release.ps1`
   - `$ExeName = "qt_image_maker.exe"` -> `$ExeName = "SpriteForge.exe"`
   - 스크립트 상단 `.SYNOPSIS` 설명 문구의 "qt_image_maker를 Release로 빌드" -> "SpriteForge를 Release로 빌드"

5. 그 외
   - 작업 폴더/저장소 이름(`qt_image_maker`)과 내부 C++ 클래스명(`SpriteSheetView`, `SpriteSheetModel` 등)은
     사용자 확인에 따라 **변경하지 않음** (폴더 경로 유지, 무관한 코드 재탐색 금지 원칙 준수)
   - `참고자료/` 하위의 과거 작업지시/커밋로그 md 파일들은 그 시점의 기록이므로 수정하지 않음

## 검증 방법 및 결과

- 타겟명이 바뀌어 기존 `build` 디렉터리의 Ninja 빌드 파일이 무효화되므로, `build` 디렉터리에서
  `cmake .`로 재구성 후 `cmake --build . --target SpriteForge` 실행 → 빌드/링크 성공
  (`build/SpriteForge.exe` 생성 확인).
- `build/SpriteForge.exe`를 실행해 프로세스가 크래시 없이 계속 실행 상태로 유지되는 것 확인
  (약 2.5초간 생존 확인 후 종료).
- 이전 타겟명(`qt_image_maker`)으로 남아있던 `build/qt_image_maker.exe`, `.ilk`, `.pdb`,
  `qt_image_maker_autogen` 폴더는 더 이상 사용되지 않는 낡은 빌드 산출물이라 정리(삭제)함.
- `build-release`/`dist`는 이번에 다시 빌드하지 않았습니다. 다음에 `build_release.ps1`을
  실행하면 CMake가 자동으로 재구성되며 `dist/SpriteForge.exe`로 배포 산출물이 생성됩니다.
- **한계**: 창 제목이 실제로 "SpriteForge"로 표시되는 화면은, 이전 다중선택 작업과 동일하게
  이 환경에 네이티브 GUI 스크린샷 도구가 없어 자동으로 캡처해 보여드리지 못했습니다.
  프로세스가 정상 기동되는 것은 확인했으니, 실행 후 직접 눈으로 확인해 주세요.
