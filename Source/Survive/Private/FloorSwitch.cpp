// Fill out your copyright notice in the Description page of Project Settings.


#include "FloorSwitch.h"

#include "Components/BoxComponent.h"

// Sets default values
AFloorSwitch::AFloorSwitch()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>("TriggerBox");
	RootComponent = TriggerBox;

	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	TriggerBox->SetBoxExtent(FVector(62.f,62.f, 32.f));
	
	FloorSwitch = CreateDefaultSubobject<UStaticMeshComponent>("FloorSwitch");
	FloorSwitch->SetupAttachment(GetRootComponent());

	Door = CreateDefaultSubobject<UStaticMeshComponent>("Door");
	Door->SetupAttachment(GetRootComponent());

	DoorCloseDelay = 5.f;
}

// Called when the game starts or when spawned
void AFloorSwitch::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFloorSwitch::OnOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AFloorSwitch::OnOverlapEnd);

	InitialDoorLoc = Door->GetComponentLocation();
	InitialSwitchLoc = FloorSwitch->GetComponentLocation();
	
}

// Called every frame
void AFloorSwitch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFloorSwitch::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult &SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap Begin."));
	bCharOnSwitch = true;
	RaiseDoor();
	LowerFloorSwitch();
}

void AFloorSwitch::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap End."));
	
	RaiseFloorSwitch();

	GetWorldTimerManager().SetTimer(SwitchTimerHandle,
								this,
								&AFloorSwitch::CloseDoor,
								DoorCloseDelay);
	bCharOnSwitch = false;
}

void AFloorSwitch::UpdateDoorLoc(float Z)
{
	FVector NewLoc = InitialDoorLoc;

	NewLoc.Z += Z;
	Door->SetWorldLocation(NewLoc);
}

void AFloorSwitch::UpdateFloorSwitchLoc(float Z)
{
	FVector NewLoc = InitialSwitchLoc;

	NewLoc.Z += Z;
	FloorSwitch->SetWorldLocation(NewLoc);

}

void AFloorSwitch::CloseDoor()
{
	if (!bCharOnSwitch)
	{
		LowerDoor();
	}

}


