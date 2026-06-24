---
name: "🧪 테스트 코드 작성 (TDD / CQTest)"
about: 새로운 단위/통합 테스트를 작성하거나 로직 검증을 수행할 때 사용합니다.
title: "[Test] "
labels: "🧪 test-tdd"
assignees: ""
---

## 🎯 테스트 목적 (Test Objective)

> 예시) UHACF_AttributeSet_Combat의 데미지 차감 공식과 Poise(경직도) 갱신 로직이 의도대로 작동하는지 검증합니다.

## 🧪 테스트 시나리오 (Test Scenarios)

### 케이스 1 : 기본 데미지 차감

**Given**
- HP 100, Poise 50인 더미 액터 스폰

**When**
- 30의 데미지(Damage Attribute) 적용

**Then**
- HP가 70으로 감소하고 누적 데미지는 0으로 초기화되어야 함

---

### 케이스 2 : 슈퍼아머(Poise) 판정

**Given**
- HP 100, Poise 50인 더미 액터 스폰

**When**
- 30의 데미지 적용

**Then**
- Poise 수치가 20으로 감소
- HitStun 상태로 전이되지 않아야 함

## ✅ 완료 조건 (Definition of Done)

- [ ] CQTest 프레임워크 기반으로 C++ 테스트 코드가 작성되어야 함.
- [ ] Session Frontend에서 테스트가 Passed 상태여야 함.
- [ ] (선택) GitHub Actions CI에서 테스트 통과가 확인되어야 함.