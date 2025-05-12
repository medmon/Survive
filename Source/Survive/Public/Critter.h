// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"
#include "Critter.generated.h"

class UInputMappingContext;
class USkeletalMeshComponent;
class UCameraComponent;
class UCapsuleComponent;
class USpringArmComponent;
class UInputAction;

UCLASS()
class SURVIVE_API ACritter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACritter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Input")
	UInputMappingContext* CritterMappingContext;
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, category = "Pawn Movement")
	float MaxSpeed;

	bool bSteering = false;
	
	UPROPERTY(EditAnywhere, category = "Pawn Movement")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, category = "Pawn Movement")
	UInputAction* StrafeAction;
	UPROPERTY(EditAnywhere, category = "Pawn Movement")
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, category = "Pawn Movement")
	UInputAction* TurnAction;
	UPROPERTY(EditAnywhere, category = "Pawn Movement")
	UInputAction* MoveForwardAction;
	UPROPERTY(EditAnywhere, category = "Pawn Movement")
	UInputAction* MouseLookAction;
	UPROPERTY(EditAnywhere, category = "Pawn Movement")
	UInputAction* CameraHomeAction;
	UPROPERTY(EditAnywhere, category = "Pawn Movement")
	UInputAction* SteerAction;
	UPROPERTY(EditAnywhere, category = "Pawn Movement")
	UInputAction* ZoomAction;
	UPROPERTY(EditAnywhere, category = "Pawn Movement")
	UInputAction* ZoomOutAction;
	UPROPERTY(EditAnywhere, category = "Pawn Movement")
	UInputAction* R90Action;

	
	//old input method
	//void MoveForward(float Value);
	//void MoveRight( float Value);
	//void Turn( float Value);
	//void LookUp(float Value);

	//Enhanced Input System
	void Move(const FInputActionValue& Value);
	void Strafe(const FInputActionValue& Value);
	void MoveForward(const FInputActionValue& Value);
	void Turn( const FInputActionValue& Value);
	void MouseLook( const FInputActionValue& Value);
	void Steering (const FInputActionValue& Value);
	void Zoom (const FInputActionValue& Value);
	void ZoomOut (const FInputActionValue& Value);
	void CameraHome ();
	void Right90 ();

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(EditAnywhere, category = "Mesh")
	TObjectPtr<USkeletalMeshComponent> CritterMesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCameraComponent> Camera;
	
	
	
};



