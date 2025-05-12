// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MainChar.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Windows.h"
#include "Components/InventoryComponent.h"
#include "Interfaces/InteractionInterface.h"
#include "UserInterface/SurviveHUD.h"
#include "World/Pickup.h"

#pragma push_macro("GetObject")
#undef GetObject


// Sets default values
AMainChar::AMainChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->TargetOffset = FVector(0.f,0.f,80.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>("FollowCamera");
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	PlayerInventory = CreateDefaultSubobject<UInventoryComponent>("PlayerInventory");
	PlayerInventory->SetSlotsCapacity(20);
	PlayerInventory->SetWeightCapacity(50.0f);
	
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;
	BaseMoveRate = 650.f;
	
}


// Called when the game starts or when spawned
void AMainChar::BeginPlay()
{
	Super::BeginPlay();

	CameraHome();
	
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MainMappingContext,0);
		}
 
		PlayerController->SetShowMouseCursor(true);
	}
	
	HUD = Cast<ASurviveHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
	
}

void AMainChar::PerformInteractionCheck()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	FHitResult Hit;
	
	PlayerController->GetHitResultUnderCursor(ECC_GameTraceChannel1, false, Hit);

	if(Hit.GetActor())
	{

		if (Hit.GetActor()->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
		{

			const float Distance = (GetActorLocation() - Hit.ImpactPoint).Size();

			if (PlayerController->bShowMouseCursor && Hit.GetActor() != InteractionData.CurrentInteractable && Distance <= InteractionCheckDistance)
			{
				FoundInteractable(Hit.GetActor());
				if (Distance <= Reach)
				{
					bInReach = true;
				}
				else
				{
					bInReach = false;
				}
				
				return;
			}

			if (PlayerController->bShowMouseCursor && Hit.GetActor() == InteractionData.CurrentInteractable)
			{
				return;
			}
		}
	}

	NoInteractableFound();
	
}

void AMainChar::FoundInteractable(AActor* NewInteractable)
{
	UE_LOG(LogTemp, Warning, TEXT("Interactable Found : %s" ), *NewInteractable->GetName());

	if (IsInteracting())
	{
		EndInteracting();
	}

	if (InteractionData.CurrentInteractable)
	{
		TargetInteractable = InteractionData.CurrentInteractable;
		TargetInteractable->EndFocus();
	}

	InteractionData.CurrentInteractable = NewInteractable;
	TargetInteractable = NewInteractable;

	HUD->UpdateInteractionWidget(&TargetInteractable->InteractableData);
	
	TargetInteractable->BeginFocus();
	
}

void AMainChar::NoInteractableFound()
{
	if (IsInteracting())
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);
	}
		
	if (InteractionData.CurrentInteractable)
	{
		if (IsValid(TargetInteractable.GetObject()))
		{
			TargetInteractable->EndFocus();
		}

		HUD->HideInteractionWidget();
		
		InteractionData.CurrentInteractable = nullptr;
//		TargetInteractable = nullptr;
		
	}

	
}

void AMainChar::BeginInteracting()
{
	PerformInteractionCheck();

	if (InteractionData.CurrentInteractable)
	{
		if (IsValid(TargetInteractable.GetObject()))
		{
			TargetInteractable->BeginInteract();

			if (FMath::IsNearlyZero(TargetInteractable->InteractableData.InteractionDuration, 0.1f))
			{
				Interacting();
			}
			else
			{
				GetWorldTimerManager().SetTimer(TimerHandle_Interaction,
					this,
					&AMainChar::Interacting,
					TargetInteractable->InteractableData.InteractionDuration,
					false);
				
			}
		}
	}
	
}

void AMainChar::EndInteracting()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);

	if (IsValid(TargetInteractable.GetObject()))
	{
		TargetInteractable->EndInteract();
	}
	

}

void AMainChar::Interacting()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);

	
	if (IsValid(TargetInteractable.GetObject()) && bInReach)
	{
		
		TargetInteractable->Interact(this);
		
	}
}

void AMainChar::ToggleMenu()
{
	HUD->ToggleMenu();
}

void AMainChar::UpdateInteractionWidget() const
{
	if (IsValid(TargetInteractable.GetObject()))
	{
		HUD->UpdateInteractionWidget(&TargetInteractable->InteractableData);
	}

}

void AMainChar::DropItem(UItemBase* ItemToDrop, const int32 QuantityToDrop)
{
	if(PlayerInventory->FindMatchingItem(ItemToDrop))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride =  ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		const FVector SpawnLocation{GetActorLocation() + (GetActorForwardVector() * 50.f)};
		const FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

		const int32 RemovedQuantity =  PlayerInventory->RemoveAmountOfItem(ItemToDrop, QuantityToDrop);

		APickup* Pickup = GetWorld()->SpawnActor<APickup>(APickup::StaticClass(), SpawnTransform, SpawnParams);

		Pickup->InitializeDrop(ItemToDrop, RemovedQuantity);
	}
}


// Called every frame
void AMainChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFreq)
	{
		PerformInteractionCheck();
	}

	if (bAutoRun)
	{
		bool InputValue = bAutoRun;
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	
		AddMovementInput(Forward, InputValue * BaseMoveRate * GetWorld()->GetDeltaSeconds());
	}
	
}

// Called to bind functionality to input
void AMainChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(LMBAction, ETriggerEvent::Triggered, this,  &AMainChar::LMB);
		EnhancedInputComponent->BindAction(LMBAction, ETriggerEvent::Completed, this,  &AMainChar::LMB);
		EnhancedInputComponent->BindAction(LMBTapAction, ETriggerEvent::Triggered, this,  &AMainChar::LMBTap);
		EnhancedInputComponent->BindAction(RMBAction, ETriggerEvent::Triggered, this,  &AMainChar::RMB);
		EnhancedInputComponent->BindAction(RMBAction, ETriggerEvent::Completed, this,  &AMainChar::RMB);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this,  &AMainChar::Turn);
		EnhancedInputComponent->BindAction(MouseTurnAction, ETriggerEvent::Triggered, this,  &AMainChar::Turn);
		EnhancedInputComponent->BindAction(MouseTurnAction, ETriggerEvent::Completed, this,  &AMainChar::Turn);
		EnhancedInputComponent->BindAction(MouseMoveAction, ETriggerEvent::Triggered, this,  &AMainChar::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,  &AMainChar::Move);
		EnhancedInputComponent->BindAction(AutoRunAction, ETriggerEvent::Triggered, this,  &AMainChar::AutoRun);
		EnhancedInputComponent->BindAction(StrafeAction, ETriggerEvent::Triggered, this,  &AMainChar::Strafe);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this,  &AMainChar::BeginInteracting);
//		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this,  &AMainChar::StopJumping);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this,  &AMainChar::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this,  &AMainChar::StopJumping);

		EnhancedInputComponent->BindAction(ToggleMenuAction, ETriggerEvent::Triggered, this,  &AMainChar::ToggleMenu);

		
		EnhancedInputComponent->BindAction(OrbitCameraAction, ETriggerEvent::Triggered, this,  &AMainChar::OrbitCamera);
		EnhancedInputComponent->BindAction(OrbitCameraAction, ETriggerEvent::Completed, this,  &AMainChar::OrbitCamera);

		EnhancedInputComponent->BindAction(CameraHomeAction, ETriggerEvent::Triggered, this,  &AMainChar::CameraHome);
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this,  &AMainChar::Zoom);
		EnhancedInputComponent->BindAction(ZoomOutAction, ETriggerEvent::Triggered, this,  &AMainChar::ZoomOut);


	}	
	
}

void AMainChar::LMB(const FInputActionValue& Value)
{

	if (Value.Get<bool>())
	{
		ShowMousePointer(false);

		LMBDown = true;
	}
	else
	{
		LMBDown = false;

		if (!RMBDown)
			ShowMousePointer(true);

	}

}

void AMainChar::LMBTap(const FInputActionValue& Value)
{
	if (!RMBDown)
	{
		UE_LOG(LogTemp, Warning, TEXT("LMB TAPPED"));
		/*
		if(ActiveOverlappingItem)
		{
			AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
			if(Weapon)
			{
				Weapon->Equip(this);
				SetActiveOverlappingItem(nullptr);
			}
		}
	*/
	
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		FHitResult Hit;
	
		PlayerController->GetHitResultUnderCursor(ECC_GameTraceChannel1, false, Hit);

		if(Hit.GetActor())
		{
			float Distance = (GetActorLocation() - Hit.ImpactPoint).Size();
			
			UE_LOG(LogTemp, Warning, TEXT("HIT : %s" ), *Hit.GetActor()->GetName());
			if (Distance <= Reach)
			{
				if (Hit.GetActor()->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
				{
					Interacting();
				}
			}
			
		}
	}	
}



void AMainChar::RMB(const FInputActionValue& Value)
{

	
	if (Value.Get<bool>())
	{
		ShowMousePointer(false);
		FaceFront();
		RMBDown = true;
	}
	else if (!Value.Get<bool>())
	{

		ShowMousePointer(true);
		bSteering = false;
		bDriving = false;
		RMBDown = false;
	}

}

void AMainChar::Turn(const FInputActionValue& Value)
{
	static tagPOINT Point = {0,0};
	
	float CurrentValue = 0.f;
	float DeltaPitchValue = 0.f;
	
	if (Value.GetValueType() == EInputActionValueType::Axis1D)
	{
		CurrentValue = Value.Get<float>();
		AddControllerYawInput(CurrentValue * BaseTurnRate * GetWorld()->GetDeltaSeconds());
		
	}
	else if (Value.GetValueType() == EInputActionValueType::Axis2D)
	{
		ShowMousePointer(false);

		CurrentValue = Value.Get<FVector2d>().X;
		DeltaPitchValue = Value.Get<FVector2d>().Y;
		if (RMBDown && !bSteering)
		{
			GetCursorPos(&Point);
			bSteering = true;
			ShowMousePointer(false);
			
		}
		else if (RMBDown && bSteering)
		{
			SetCursorPos(Point.x,Point.y);
			
		}

		if (!RMBDown)
		{
			ShowMousePointer(true);
		
			SetCursorPos(Point.x,Point.y);
			bSteering = false;
			bDriving = false;
		}
		
		//set facing of actor
		AddControllerYawInput(CurrentValue * BaseTurnRate * GetWorld()->GetDeltaSeconds());
		
		//set pitch of cameraboom

		//do calculations for new pitch value of CameraBoom		
		const float NewPitch = FMath::Clamp(CameraBoom->GetRelativeRotation().Pitch + DeltaPitchValue,-80.f, 40.f);
	
		const FRotator LookAxisRotator = FRotator(NewPitch , CameraBoom->GetRelativeRotation().Yaw , 0.f );

		//set pitch of cameraboom
		CameraBoom->SetRelativeRotation(LookAxisRotator);
		
	}
	
}

void AMainChar::Move(const FInputActionValue& Value)
{
	FaceFront();

	if (Value.GetValueType() == EInputActionValueType::Boolean)
	{
		bAutoRun = false;
		bool InputValue = Value.Get<bool>();
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		
		AddMovementInput(Forward, InputValue * BaseMoveRate * GetWorld()->GetDeltaSeconds());

		
	}
	else if (Value.GetValueType() == EInputActionValueType::Axis1D)
	{
		bAutoRun = false;
		float InputValue = Value.Get<float>();
		
		if (Controller != nullptr && InputValue != 0 )
		{
			
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		
			//get forward vector based on rotation yaw
			const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			//get right vector based on Rotation Yaw
			const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			AddMovementInput(Forward, InputValue * BaseMoveRate * GetWorld()->GetDeltaSeconds());
		
		}
	}
	
}

void AMainChar::AutoRun(const FInputActionValue& Value)
{
	bAutoRun = bAutoRun ? false : true;
}

void AMainChar::Strafe(const FInputActionValue& Value)
{
	FaceFront();
	float CurrentValue = Value.Get<float>();

	FVector Right = GetActorRightVector();
	AddMovementInput(Right, CurrentValue);

}


void AMainChar::OrbitCamera(const FInputActionValue& Value)
{
	static tagPOINT Point = {0,0};
	
	float CurrentValue = 0.f;
	float DeltaPitchValue = 0.f;

	const FVector2d LookAxisVector = Value.Get<FVector2d>();

	FVector LookAxis = FVector(LookAxisVector.X, LookAxisVector.Y, 0.f);

	float NewPitch = FMath::Clamp(CameraBoom->GetRelativeRotation().Pitch + LookAxisVector.Y,-89.f, 40.f);

	float NewYaw = CameraBoom->GetRelativeRotation().Yaw+LookAxisVector.X;
	ShowMousePointer(false);

	CurrentValue = Value.Get<FVector2d>().X;
	DeltaPitchValue = Value.Get<FVector2d>().Y;
	if (LMBDown && !bLooking)
	{
		GetCursorPos(&Point);
		bLooking = true;
		ShowMousePointer(false);
			
	}
	else if (LMBDown && bLooking)
	{
		SetCursorPos(Point.x,Point.y);
			
	}

	if (!LMBDown)
	{
		ShowMousePointer(true);
		
		SetCursorPos(Point.x,Point.y);
		bDriving = false;
		bLooking = false;
	}

	
	FRotator LookAxisRotator = FRotator(NewPitch , NewYaw, 0.f );

	if (!bSteering)
	{
		//	ShowMousePointer(false);
		CameraBoom->SetRelativeRotation(LookAxisRotator);
		bBoomTurned = true;
	}

}


void AMainChar::CameraHome()
{
	float deltaSecs = GetWorld()->GetDeltaSeconds();
	CameraBoom->SetRelativeRotation(FRotator(-20.f,0.f,0.f));
	CameraBoom->TargetArmLength = 400.f;
}

void AMainChar::Zoom(const FInputActionValue& Value)
{
	float ZoomFactor = Value.Get<float>();

	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength - 10, 60.f, 1200.f);
}

void AMainChar::ZoomOut(const FInputActionValue& Value)
{
	float ZoomFactor = Value.Get<float>();

	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength + 10, 60.f, 1200.f);
}

void AMainChar::FaceFront()
{ 
	float BoomYaw = CameraBoom->GetRelativeRotation().Yaw;   //local space rotation
	float FrontYaw = Controller->GetControlRotation().Yaw;  //world space rotation
	
	float NewYaw = FrontYaw + BoomYaw;   //new rotation in world space
	
	Controller->SetControlRotation(FRotator(0.f, NewYaw, 0.f));

	CameraBoom->SetRelativeRotation(FRotator(CameraBoom->GetRelativeRotation().Pitch, 0.f,0.f));
	
}

void AMainChar::ShowMousePointer( const bool ShouldShow) const
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		PlayerController->SetShowMouseCursor(ShouldShow);
	}

}

// void AMainChar::TriggerLMBDown ()
// {
// 	//trigger the mouse click event. This will fire any lmb click events within blueprints.
// 	FViewportClient* Client = GEngine->GameViewport->Viewport->GetClient();
// 	FKey MouseLMB = EKeys::LeftMouseButton;
// 	Client->InputKey(GEngine->GameViewport->Viewport, 0, MouseLMB, EInputEvent::IE_Pressed);
// }

#pragma pop_macro("GetObject")