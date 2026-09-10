// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpecHandle.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "CombatInput/HACF_InputBuffer.h"
#include "Combat/Combo/HACF_ComboWindowRoute.h"
#include "CombatInput/HACF_ActionWindowTypes.h"
#include "HACF_CombatComponent.generated.h"

struct FGameplayAbilitySpec;
class UAbilitySystemComponent;

/*
 *
 */
UCLASS( ClassGroup=(HyperActionCombatFramework), meta=(BlueprintSpawnableComponent) )
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_CombatComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHACF_CombatComponent(const FObjectInitializer& ObjectInitializer);
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	//~Begin IGameFrameworkInitStateInterface
	static const FName NAME_ActorFeatureName;
	
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,FGameplayTag DesiredState) override;
	virtual void CheckDefaultInitialization() override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	
	void HandlePlayerStateChanged();
	//~End
	
	//~Begin AbilitySystem
	void InitializeWithASC(UAbilitySystemComponent* InASC, AActor* InOwnerActor);
	void UninitializeFromASC();
	
	UFUNCTION(BlueprintCallable, Category="HACF")
	UAbilitySystemComponent* GetAbilitySystemComponent() const { return ASC; }
	//~End

	//~Begin Ability Input
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	// 입력 처리
	// PlayerController의 PostProcessInput에서 호출
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	//~End
	
	//~Begin Buffered Input / Action Windows
	// 입력 버퍼 윈도우 처리
	void RecordBufferedInput(const FGameplayTag& InputTag);
	
	// ActionWindow를 생성 후 저장하고 Handle을 반환
	FHACF_ActionWindowHandle OpenActionWindow(const FHACF_ActionWindowDefinition& Definition, const UObject* Source, FHACF_ActionWindowInputHandler InputHandler);
	bool CloseActionWindow(FHACF_ActionWindowHandle Handle);
	int32 CloseAllActionWindowsFromSource(const UObject* Source);
	void ClearActionWindows();
	
	bool IsActionWindowActive(FHACF_ActionWindowHandle Handle) const;
	
	// 호출자가 핸들을 저장한 뒤 실행합니다.
	bool ProcessBufferedInput();
	//~End
	
	
	FHACF_ComboWindowRouteHandle BindComboWindowRoute(FHACF_ComboWindowBeginHandler BeginHandler, FHACF_ComboWindowEndHandler EndHandler);
	bool UnbindComboWindowRoute(FHACF_ComboWindowRouteHandle Handle);
	void RouteComboWindowBegin(const FHACF_ComboWindowKey& NotifyKey, const FGameplayTag& WindowTag);
	void RouteComboWindowEnd(const FHACF_ComboWindowKey& NotifyKey);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void OnRegister() override;
	
	virtual bool ResolveAbilitySystem(UAbilitySystemComponent*& OutASC, AActor*& OutOwnerActor) const;
	
	// 입력 버퍼 소비를 위한 메서드
	bool TryConsumeBufferedInput();
	
	// 입력 버퍼 핸들러 처리
	bool TryDispatchActionWindowInput(const FHACF_ActionWindowDispatchTarget& DispatchTarget);
	
	// ASC AbilityInput 이벤트 전달
	void DispatchAbilityInputPressed(FGameplayAbilitySpec& AbilitySpec);
	void DispatchAbilityInputReleased(FGameplayAbilitySpec& AbilitySpec);
	
	// 물리적/논리적으로 현재 눌린 입력 태그
	TSet<FGameplayTag> PressedInputTags;
	
	// 입력 처리용 버퍼
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// 이번 입력 처리에서 어빌리티 활성화 여부를 판정한 뒤 실제 버퍼에 기록할 Press 입력.
	TArray<FHACF_BufferedInput> PendingBufferedInputs;

	// 입력 버퍼 시스템
	FHACF_InputBuffer InputBuffer;
	
	FHACF_ActionWindowContainer ActionWindows;
	
	UPROPERTY(EditDefaultsOnly, Category = "HACF|Input")
	float InputBufferDuration = 0.2f;
	
private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
	
	FHACF_ComboWindowBeginHandler ComboWindowBeginHandler;
	FHACF_ComboWindowEndHandler ComboWindowEndHandler;
	uint32 NextComboWindowRouteId = 0;
	uint32 ActiveComboWindowRouteId = 0;
	
	bool bIsDispatchingBufferedInput = false;
};
