// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Pawn.h"
#include "SampleCharacter.generated.h"

struct FInputActionValue;
class UHACF_HeroComponent;

UCLASS()
class HACF_SAMPLE_API ASampleCharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASampleCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void PostInitializeComponents() override;

	// 이동 및 카메라 처리
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_MoveCompleted(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	
	//TODO: 점프는 나중에 구현
	// void Input_JumpStarted(const FInputActionValue& InputActionValue);
	// void Input_JumpCompleted(const FInputActionValue& InputActionValue);
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(EditAnywhere, Category="Input Tag")
	FGameplayTag InputMoveTag;
	UPROPERTY(EditAnywhere, Category="Input Tag")
	FGameplayTag InputLookTag;
	
	UPROPERTY(Transient)
	TObjectPtr<UHACF_HeroComponent> HeroComponent;
};
