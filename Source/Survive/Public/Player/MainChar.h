// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "Windows.h"

#include "MainChar.generated.h"

class UItemBase;
class UInventoryComponent;
class ASurviveHUD;
class IInteractionInterface;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;

USTRUCT()
struct FInteractionData
{
	GENERATED_BODY()

	FInteractionData() : CurrentInteractable(nullptr), LastInteractionCheckTime(0.0f)
	{
		
	};
	
	UPROPERTY()
	AActor* CurrentInteractable;

	UPROPERTY()
	float LastInteractionCheckTime;
};

UCLASS()
class SURVIVE_API AMainChar : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainChar();

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = Camera)
	float BaseMoveRate;
	

	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* LMBAction;
	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* LMBTapAction;
	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* RMBAction;
	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* MouseTurnAction;
	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* TurnAction;
	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* MouseMoveAction;
	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* AutoRunAction;
	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* StrafeAction;
	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* ToggleMenuAction;
	
	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* OrbitCameraAction;
	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* CameraHomeAction;
	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* ZoomAction;
	UPROPERTY(EditAnywhere, category = "Input")
	UInputAction* ZoomOutAction;


	bool bSteering = false;
	bool bDriving = false;
	bool bLooking = false;
	bool LMBDown = false;
	bool RMBDown = false;
	bool bBoomTurned = false;
	bool bAutoRun = false;
	bool bInReach = false;
	
	FInputActionValue AutoRunStruct = FInputActionValue(EInputActionValueType::Boolean, FVector(1.f,1.f,1.f));
	
//	tagPOINT Point = {0,0};

	FORCEINLINE bool IsInteracting() { return GetWorldTimerManager().IsTimerActive(TimerHandle_Interaction); }

	FORCEINLINE UInventoryComponent* GetInventory() const { return PlayerInventory; };

	void UpdateInteractionWidget() const;

	void DropItem(UItemBase* ItemToDrop, const int32 QuantityToDrop);
	
	
protected:
//==============================================================================================
//		Properties and Variables
//==============================================================================================	

	UPROPERTY()
	ASurviveHUD* HUD;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Input")
	UInputMappingContext* MainMappingContext;


	UPROPERTY(VisibleAnywhere, Category = "Character | Interaction")
	TScriptInterface<IInteractionInterface> TargetInteractable;

	UPROPERTY(VisibleAnywhere, Category = "Character | Inventory")
	UInventoryComponent* PlayerInventory;
	
	float InteractionCheckFreq = 0.5f;			//in seconds

	float InteractionCheckDistance = 6000.0f;     //length of line trace

	float Reach = 225.f;
	
	FTimerHandle TimerHandle_Interaction;

	FInteractionData InteractionData;

	//==============================================================================================
	//		Functions
	//==============================================================================================

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void PerformInteractionCheck();
	void FoundInteractable(AActor* NewInteractable);
	void NoInteractableFound();
	void BeginInteracting();
	void EndInteracting();
	void Interacting();
	void ToggleMenu();
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void LMB (const FInputActionValue& Value);
	void RMB (const FInputActionValue& Value);
	void Turn (const FInputActionValue& Value);
	void Move (const FInputActionValue& Value);
	void AutoRun (const FInputActionValue& Value);
	void Strafe(const FInputActionValue& Value);
	void OrbitCamera(const FInputActionValue& Value);

	
	FORCEINLINE class USpringArmComponent* GetCameraBoom( ) const { return CameraBoom; };
	FORCEINLINE class UCameraComponent* GetFollowCamera( ) const { return FollowCamera; };

	void CameraHome();
	void Zoom(const FInputActionValue& Value);
	void ZoomOut(const FInputActionValue& Value);
	void FaceFront();
	void LMBTap(const FInputActionValue& Value);

	void ShowMousePointer( bool ShouldShow) const;


//	void TriggerLMBDown();
	
};
