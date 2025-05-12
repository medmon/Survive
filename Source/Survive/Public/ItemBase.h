// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStructs.h"
#include "ItemBase.generated.h"

class UInventoryComponent;
class AMainChar;
enum class EItemType : uint8;
enum class EItemQuality : uint8;
/**
 * 
 */
UCLASS()
class SURVIVE_API UItemBase : public UObject
{
	GENERATED_BODY()
	
public:

	UPROPERTY()
	UInventoryComponent* OwningInventory;
	
	UPROPERTY(VisibleAnywhere, category = "Item Data")
	int32 Quantity;
	
	UPROPERTY(VisibleAnywhere, category = "Item Data")
	FName ItemID;
	
	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	EItemType ItemType;

	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	EItemQuality ItemQuality;

	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	FItemStatistics ItemStatistics;

	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	FItemTextData ItemTextData;

	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	FItemNumericData ItemNumericData;

	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	FItemAssetData ItemAssetData;

	bool bIsCopy;
	bool bIsPickup;

	UItemBase();

	void ResetItemFlags();
	
	UItemBase* CreateItemCopy() const;

	UFUNCTION(Category = "Item")
	FORCEINLINE float GetItemStackWeight() const { return Quantity * ItemNumericData.Weight; }
	UFUNCTION(Category = "Item")
	FORCEINLINE float GetItemSingleWeight() const { return ItemNumericData.Weight; }
	UFUNCTION(Category = "Item")
	FORCEINLINE bool IsFullItemStack() const { return Quantity == ItemNumericData.MaxStackSize; }

	UFUNCTION(Category = "Item")
	void SetQuantity(const int32 NewQuantity);

	UFUNCTION(Category = "Item")
	virtual void Use(AMainChar* MainChar);

protected:
	bool operator==(const FName& OtherID) const
	{
		return this->ItemID == OtherID;
	}
	
	
};
