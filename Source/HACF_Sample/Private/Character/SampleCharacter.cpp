// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SampleCharacter.h"
#include "EnhancedInputComponent.h"
#include "Character/HACF_HeroComponent.h"

// Sets default values
ASampleCharacter::ASampleCharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASampleCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASampleCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	HeroComponent = FindComponentByClass<UHACF_HeroComponent>();

	ensureMsgf(HeroComponent, TEXT("%s requires a HACF HeroComponent."), *GetNameSafe(this));
}

// Called every frame
void ASampleCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASampleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	
	if (HeroComponent)
	{
		const UInputAction* MoveAction = HeroComponent->FindNativeInputAction(InputMoveTag);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ThisClass::Input_MoveCompleted);
		
		const UInputAction* LookAction = HeroComponent->FindNativeInputAction(InputLookTag);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
	}
}

void ASampleCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	if (HeroComponent)
	{
		HeroComponent->SubmitMoveInput(InputActionValue.Get<FVector2D>());
	}
}

void ASampleCharacter::Input_MoveCompleted(const FInputActionValue& InputActionValue)
{
	if (HeroComponent)
	{
		HeroComponent->SubmitMoveInput(FVector2D::ZeroVector);
	}
}

void ASampleCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
	const FVector2D LookInput = InputActionValue.Get<FVector2D>();

	if (LookInput.X != 0.f)
	{
		AddControllerYawInput(LookInput.X);
	}

	if (LookInput.Y != 0.f)
	{
		AddControllerPitchInput(LookInput.Y);	
	}
}

