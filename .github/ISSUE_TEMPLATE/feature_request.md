---
name: Feature request
about: 일감 생성 템플릿
title: "[Feature]"
labels: "✨ feature"
assignees: ''

---

## 🎯 작업 목적 (Description)

<!-- 이 기능이 왜 필요한지, 어떤 역할을 하는지 간략하게 적어주세요. -->

- 전투의 중심이 되는 컨트롤 타워인 `UHACF\_CombatComponent`의 기본 헤더 구조를 설계하고 GAS 초기화 브릿지를 캡슐화합니다.



## 📋 세부 작업 내역 (Tasks)

<!-- 이번 이슈에서 실제로 타이핑할 코드 단위의 체크리스트입니다. -->

- [ ] `EHACF\_CombatInputType` Enum 정의

- [ ] `FCombatInputBuffer` 구조체 정의

- [ ] `UHACF\_CombatComponent` 클래스 생성 및 전방 선언 적용

- [ ] `InitializeWithASC` 함수 구현



## ✅ 완료 조건 (Definition of Done)

<!-- 이 이슈를 닫기(Close) 위해 반드시 만족해야 하는 조건입니다. (TDD와 연동) -->

- [ ] 에디터에서 컴파일 에러가 발생하지 않아야 함.

- [ ] 블루프린트에서 `RequestAttack` 함수 노드가 정상적으로 노출되어야 함.

- [ ] 연관된 CQTest 단위 테스트 코드가 모두 `Passed` 상태여야 함.
