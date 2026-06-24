---
name: "♻️ 리팩토링 및 최적화 (Refactoring)"
about: 기존 코드의 구조 개선, 성능 최적화, 기술 부채 해결을 진행할 때 사용합니다.
title: "[Refactor] "
labels: "♻️ refactor"
assignees: ''

---

## 🛠️ 현재 구조의 문제점 (Current Problems)

> 예시) CombatComponent 내부에서 타겟팅 로직을 직접 처리하고 있어 클래스의 책임이 비대해졌습니다. (SRP 위배)

## ✨ 개선 방향 (Proposed Changes)

타겟팅 연산 로직을 별도의 `UHACF_TargetingComponent`로 분리하고 델리게이트로 통신합니다.

## 📋 세부 작업 내역 (Tasks)

- [ ] UHACF_TargetingComponent 클래스 생성
- [ ] CombatComponent의 기존 타겟팅 로직 이관
- [ ] 타겟 갱신 델리게이트 연결

## ✅ 완료 조건 (Definition of Done)

- [ ] 기존 CQTest가 모두 Passed 상태여야 함.
- [ ] 타겟팅 Tick 비용이 기존 대비 저하되지 않아야 함.
