// Fill out your copyright notice in the Description page of Project Settings.


#include "Critter.h"


#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
ACritter::ACritter()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");

	Capsule = CreateDefaultSubobject<UCapsuleComponent>("Capsule");
	Capsule->SetRelativeLocation(FVector(-90.f,0.f,0.f));
	SetRootComponent(Capsule);
	
	CritterMesh = CreateDefaultSubobject<USkeletalMeshComponent>("CritterMesh");
	CritterMesh->SetupAttachment(GetRootComponent());

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;
//	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 3.0f;
	
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(CameraBoom);

	//AutoPossessPlayer = EAutoReceiveInput::Player0;

	MaxSpeed = 100.f;
	
}

// Called when the game starts or when spawned
void ACritter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(CritterMappingContext,0);
		}
	}
	
}

// Called every frame
void ACritter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACritter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Old Input system
//	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACritter::MoveForward);
//	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACritter::MoveRight);
//	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ACritter::Turn);
//	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ACritter::LookUp);

	//Enhanced Input System
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACritter::Move);
		EnhancedInputComponent->BindAction(StrafeAction, ETriggerEvent::Triggered, this, &ACritter::Strafe);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACritter::Turn);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ACritter::Turn);
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &ACritter::MoveForward);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ACritter::MouseLook);
		EnhancedInputComponent->BindAction(SteerAction, ETriggerEvent::Triggered, this, &ACritter::Steering);
		EnhancedInputComponent->BindAction(SteerAction, ETriggerEvent::Completed, this, &ACritter::Steering);
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Completed, this, &ACritter::Zoom);
		EnhancedInputComponent->BindAction(ZoomOutAction, ETriggerEvent::Completed, this, &ACritter::ZoomOut);
		EnhancedInputComponent->BindAction(CameraHomeAction, ETriggerEvent::Triggered, this, &ACritter::CameraHome);
		EnhancedInputComponent->BindAction(R90Action, ETriggerEvent::Triggered, this, &ACritter::Right90);

	}
	
}

//old input system functions
void ACritter::MoveForward(const FInputActionValue& Value)
{
	bool CurrentValue = Value.Get<bool>();

	if (Controller && CurrentValue )
	{
		FVector Forward = GetActorForwardVector();
		AddMovementInput(Forward, CurrentValue);
	}
}

// void ACritter::MoveRight(float Value)
// {
// 	if (Controller && Value != 0.f)
// 	{
// 		FVector Right = GetActorRightVector();
// 		AddMovementInput(Right, Value);
// 	}	
//
// }

void ACritter::Turn(const FInputActionValue& Value)
{
	float CurrentValue = 0.f;
	float PitchValue = 0.f;
	
	if (Value.GetValueType() == EInputActionValueType::Axis1D)
	{
		CurrentValue = Value.Get<float>();
		AddControllerYawInput(CurrentValue);
		
	}
	else if (Value.GetValueType() == EInputActionValueType::Axis2D)
	{
		CurrentValue = Value.Get<FVector2d>().X;
		PitchValue = Value.Get<FVector2d>().Y;
		
		//set facing of actor
		AddControllerYawInput(CurrentValue);

		const FVector2d LookAxisVector = Value.Get<FVector2d>();

				
		const float NewPitch = FMath::Clamp(CameraBoom->GetRelativeRotation().Pitch + LookAxisVector.Y,-80.f, 20.f);
	
		const FRotator LookAxisRotator = FRotator(NewPitch , CameraBoom->GetRelativeRotation().Yaw , 0.f );

		//set pitch of cameraboom
		CameraBoom->SetRelativeRotation(LookAxisRotator);
		
	}
	

	
}

void ACritter::MouseLook(const FInputActionValue& Value)
{
	const FVector2d LookAxisVector = Value.Get<FVector2d>();

	FVector LookAxis = FVector(LookAxisVector.X, LookAxisVector.Y, 0.f);

	float NewPitch = FMath::Clamp(CameraBoom->GetRelativeRotation().Pitch + LookAxisVector.Y,-89.f, 30.f);
	
	FRotator LookAxisRotator = FRotator(NewPitch , CameraBoom->GetRelativeRotation().Yaw+LookAxisVector.X, 0.f );

	if (!bSteering)
	{
		CameraBoom->SetRelativeRotation(LookAxisRotator);
	}
	
}

void ACritter::Steering(const FInputActionValue& Value)
{
	if (Value.Get<bool>() && !bSteering)
	{
		bSteering = true;
//		Controller->SetControlRotation(FRotator( GetActorRotation().Pitch, GetActorRotation().Yaw + CameraBoom->GetRelativeRotation().Yaw, 0.f));
	//	CameraBoom->SetRelativeRotation(FRotator(CameraBoom->GetRelativeRotation().Pitch, GetActorRotation().Yaw + CameraBoom->GetRelativeRotation().Yaw,0.f));

	}
	else
	{
		bSteering = false;
	}
}

void ACritter::Zoom(const FInputActionValue& Value)
{
	float ZoomFactor = Value.Get<float>();

	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength - 10, 60.f, 1200.f);
}

void ACritter::ZoomOut(const FInputActionValue& Value)
{
	float ZoomFactor = Value.Get<float>();

	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength + 10, 60.f, 1200.f);
}

void ACritter::CameraHome()
{
	float deltaSecs = GetWorld()->GetDeltaSeconds();
	CameraBoom->SetRelativeRotation(FRotator(-30.f,0.f,0.f));
	CameraBoom->TargetArmLength = 300.f;
}

// void ACritter::LookUp(float Value)
// {
// 	
// }


//enhanced input functions
void ACritter::Move(const FInputActionValue& Value)
{
	FVector2d CurrentValue = Value.Get<FVector2d>();

	FVector Forward = GetActorForwardVector();
	AddMovementInput(Forward, CurrentValue.X);
}

void ACritter::Strafe(const FInputActionValue& Value)
{
	FVector2d CurrentValue = Value.Get<FVector2d>();

	FVector Right = GetActorRightVector();
	AddMovementInput(Right, CurrentValue.Y);

}

void ACritter::Right90()
{
	Controller->SetControlRotation(FRotator( GetActorRotation().Pitch, GetActorRotation().Yaw + CameraBoom->GetRelativeRotation().Yaw, 0.f));
	CameraHome();
}

