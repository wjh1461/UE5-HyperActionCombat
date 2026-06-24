---
name: "\U0001F41B 버그 수정 (Bug Report)"
about: 게임 플레이나 로직 중 발생하는 오류를 제보하고 수정할 때 사용합니다.
title: "[Bug] "
labels: "\U0001F41B bug"
assignees: ''

---

## 🐛 버그 설명 (Describe the bug)

> 예시) Mover의 대시 모드(ActionDashMode) 사용 중 벽에 비스듬히 부딪히면 캐릭터가 무한히 체공하는 현상이 발생합니다.

## 🔄 재현 방법 (Steps to reproduce)

1. '샌드박스 맵'을 실행한다.
2. 벽을 45도 각도로 바라본다.
3. 회피(Dodge) 버튼을 누른다.
4. 캐릭터가 벽에 끼어 바닥으로 떨어지지 않는 현상을 확인한다.

## 🎯 예상되는 정상 동작 (Expected behavior)

벽에 부딪히면 대시가 강제로 종료되고 바닥으로 정상 착지해야 합니다.

## 💻 환경 (Environment)

- OS: Windows 11
- Unreal Engine Version: 5.4+
- 발생 환경: PIE / Standalone / Dedicated Server

## 📸 스크린샷 및 로그 (Screenshots / Logs)

첨부
