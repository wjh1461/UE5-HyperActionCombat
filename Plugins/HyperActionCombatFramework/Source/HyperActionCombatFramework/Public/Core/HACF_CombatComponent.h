// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CombatInput/HACF_InputBuffer.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "CombatInput/HACF_ActionWindowTypes.h"
#include "HACF_CombatComponent.generated.h"

struct FGameplayAbilitySpec;
class UAbilitySystemComponent;
struct FGameplayAbilitySpecHandle;

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
	//~End
	
	void HandlePlayerStateChanged();
	
	void InitializeWithASC(UAbilitySystemComponent* InASC, AActor* InOwnerActor);
	void UninitializeFromASC();
	
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	// 입력 처리
	// PlayerController의 PostProcessInput에서 호출
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	
	void RecordBufferedInput(const FGameplayTag& InputTag);
	
	// 입력 버퍼 윈도우 처리
	FHACF_ActionWindowHandle OpenActionWindow(const FHACF_ActionWindowDefinition& Definition, const UObject* Source = nullptr);
	bool CloseActionWindow(FHACF_ActionWindowHandle Handle);
	int32 CloseAllActionWindowsFromSource(const UObject* Source);
	bool IsActionWindowActive(FHACF_ActionWindowHandle Handle) const;
	void ClearActionWindows();
	
	UFUNCTION(BlueprintCallable, Category="HACF")
	UAbilitySystemComponent* GetAbilitySystemComponent() const { return ASC; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void OnRegister() override;
	
	virtual bool ResolveAbilitySystem(UAbilitySystemComponent*& OutASC, AActor*& OutOwnerActor) const;
	
	// 입력 버퍼 소비를 위한 메서드
	bool TryConsumeBufferedInput();
	
	bool TryDispatchActionWindowInput(const FHACF_ActionWindow& ActionWindow, const FHACF_BufferedInput& BufferedInput);
	
	// ASC AbilityInput 이벤트 전달
	void DispatchAbilityInputPressed(FGameplayAbilitySpec& AbilitySpec);
	void DispatchAbilityInputReleased(FGameplayAbilitySpec& AbilitySpec);
	
	// 입력 처리용 버퍼
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
	
	FHACF_InputBuffer InputBuffer;
	
	FHACF_ActionWindowContainer ActionWindows;
	
	// Source가 소유한 Window 목록
	TMap<FObjectKey, TSet<FHACF_ActionWindowHandle>> SourceWindowHandles;
	// Handle로 닫을 때 Source 인덱스도 정리하기 위한 역방향 조회
	TMap<FHACF_ActionWindowHandle, FObjectKey> ActionWindowSources;
	
	TMap<FObjectKey, FHACF_ActionWindowInputHandler> ActionWindowInputHandlers;
	
	
	UPROPERTY(EditDefaultsOnly)
	float InputBufferDuration = 0.2f;
	
private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
};
