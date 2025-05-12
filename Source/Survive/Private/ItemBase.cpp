// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBase.h"

#include "Components/InventoryComponent.h"

UItemBase::UItemBase() :
	bIsCopy(false),
	bIsPickup(false)
{
	
}

void UItemBase::ResetItemFlags()
{
	bIsCopy = false;
	bIsPickup = false;
}

UItemBase* UItemBase::CreateItemCopy() const
{
	UItemBase* ItemCopy = NewObject<UItemBase>(StaticClass());

	ItemCopy->ItemID = this->ItemID;
	ItemCopy->Quantity = this->Quantity;
	ItemCopy->ItemQuality = this->ItemQuality;
	ItemCopy->ItemType = this->ItemType;
	ItemCopy->ItemTextData = this->ItemTextData;
	ItemCopy->ItemNumericData = this->ItemNumericData;
	ItemCopy->ItemStatistics = this->ItemStatistics;
	ItemCopy->ItemAssetData = this->ItemAssetData;
	ItemCopy->bIsCopy = true;
	
	return ItemCopy;
	
}

void UItemBase::SetQuantity(const int32 NewQuantity)
{
	if (NewQuantity != Quantity)
	{
		Quantity = FMath::Clamp(NewQuantity, 0, ItemNumericData.bIsStackable ? ItemNumericData.MaxStackSize : 1);

		if (OwningInventory)
		{
			if (Quantity <= 0)
		 	{
		 		OwningInventory->RemoveSingleInstanceOfItem(this);
		 	}
		}
	}
}

void UItemBase::Use(AMainChar* MainChar)
{
	
}
