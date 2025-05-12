// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVE_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()
public:

	/* Reference to the UMG Asset in the Editor */
	UPROPERTY(EditAnywhere,BlueprintReadWrite, category = "Widgets")
	TSubclassOf<class UUserWidget> HUDOverlayAsset;

	/* Variable to hold the widget after creating it */
	UPROPERTY(EditAnywhere,BlueprintReadWrite, category = "Widgets")
	UUserWidget* HUDOverlay;

protected:
	virtual void BeginPlay() override;

};
