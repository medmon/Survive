// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryComponent.h"

#include "ItemBase.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}


UItemBase* UInventoryComponent::FindMatchingItem(UItemBase* ItemIn) const
{
	if (ItemIn)
	{
		if (InventoryContents.Contains(ItemIn))
		{
			return ItemIn;
		}
	}
	return nullptr;
}

UItemBase* UInventoryComponent::FindNextITemByID(UItemBase* ItemIn) const
{
	if (ItemIn)
	{
		if (const TArray<TObjectPtr<UItemBase>>::ElementType* Result = InventoryContents.FindByKey(ItemIn))
		{
			return *Result;
		}
	}
	return nullptr;
}

UItemBase* UInventoryComponent::FindNextPartialStack(UItemBase* ItemIn) const
{
	if (const TArray<TObjectPtr<UItemBase>>::ElementType* Result =
		InventoryContents.FindByPredicate([&ItemIn](const UItemBase* InventoryItem)
		{
			return InventoryItem->ItemID == ItemIn->ItemID && !InventoryItem->IsFullItemStack();	
		}
		))
	{
		return *Result;
	}

	return nullptr;
}

void UInventoryComponent::RemoveSingleInstanceOfItem(UItemBase* ItemToRemove)
{
	InventoryContents.RemoveSingle(ItemToRemove);
	OnInventoryUpdated.Broadcast();
}

int32 UInventoryComponent::RemoveAmountOfItem(UItemBase* ItemIn, int32 DesiredAmountToRemove)
{
	const int32 AmountToRemove = FMath::Min(DesiredAmountToRemove, ItemIn->Quantity);

	ItemIn->SetQuantity(ItemIn->Quantity - AmountToRemove);

	InventoryTotalWeight -= AmountToRemove * ItemIn->GetItemSingleWeight();

	OnInventoryUpdated.Broadcast();

	return AmountToRemove;
}

void UInventoryComponent::SplitExistingStack(UItemBase* ItemIn, const int32 AmountToSplit)
{
	if (!(InventoryContents.Num() + 1 > InventorySlotsCapacity))
	{
		RemoveAmountOfItem(ItemIn, AmountToSplit);
		AddNewItem(ItemIn, AmountToSplit);
	}
}

int32 UInventoryComponent::CalculateWeightAddAmount(UItemBase* ItemIn, int32 RequestedAddAmount)
{
	const int32 WeightMaxAddAmount = FMath::FloorToInt(GetWeightCapacity() - InventoryTotalWeight) / ItemIn->GetItemSingleWeight();

	if (WeightMaxAddAmount >= RequestedAddAmount)
	{
		return RequestedAddAmount;
	}
	
	return WeightMaxAddAmount;
}

int32 UInventoryComponent::CalculateNumberForFullStack(UItemBase* StackableItem, int32 InitialRequestedAddAmount)
{
	const int32 AddAmountToMakeFullStack = StackableItem->ItemNumericData.MaxStackSize - StackableItem->Quantity;

	return FMath::Min(InitialRequestedAddAmount, AddAmountToMakeFullStack);
}

FItemAddResult UInventoryComponent::HandleNonStackableItems(UItemBase* InputItem, int32 RequestedAddAmount)
{
	//check if the item has valid weight
	if (FMath::IsNearlyZero(InputItem->GetItemStackWeight()) || InputItem->GetItemStackWeight() <= 0)
	{
		//return added none
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Could not add {0} to the inventory. Item has invalid Weight value."), InputItem->ItemTextData.Name));
	}

	//will the item weight overflow weight capacity
	if (InventoryTotalWeight + InputItem->GetItemSingleWeight() > GetWeightCapacity())
	{
		//return added none
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Could not add {0} to the inventory. Item exceeds inventory weight capacity."), InputItem->ItemTextData.Name));
	}

	if (InventoryContents.Num() + RequestedAddAmount > InventorySlotsCapacity)
	{
		//return added none
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Could not add {0} to the inventory. No slot available for item."), InputItem->ItemTextData.Name));
	}

	AddNewItem(InputItem, RequestedAddAmount);

	//return added all result
	return FItemAddResult::AddedAll(RequestedAddAmount,FText::Format(FText::FromString("Added {0} to the inventory."), InputItem->ItemTextData.Name));
	
}

int32 UInventoryComponent::HandleStackableItems(UItemBase* ItemIn, int32 RequestedAddAmount)
{
	if (RequestedAddAmount <= 0 || FMath::IsNearlyZero(ItemIn->GetItemStackWeight()))
	{
		// invalid item data
		return 0;
	}

	int32 AmountToDistribute = RequestedAddAmount;

	// Check if the incoming item already exists in inventory and is not a full stack
	UItemBase* ExistingItemStack = FindNextPartialStack(ItemIn);

	while(ExistingItemStack)
	{
		// calculate how many of the existing item would be needed to make the next stack full
		const int32 AmountToMakeFullStack = CalculateNumberForFullStack(ExistingItemStack, AmountToDistribute);
		// calculate how many of the AmountToMakeFullStack can actually be carried based on Weight Capacity
		const int32 WeightLimitAddAmount = CalculateWeightAddAmount(ExistingItemStack, AmountToMakeFullStack);

		// as long as the remaining amount of the item does not exceed weight capacity
		if(WeightLimitAddAmount > 0)
		{		
			// adjust the existing item stack quantity and inventory total weight
			ExistingItemStack->SetQuantity(ExistingItemStack->Quantity + WeightLimitAddAmount);
			InventoryTotalWeight += (ExistingItemStack->GetItemSingleWeight() * WeightLimitAddAmount);

			//adjust the amount to be distributed
			AmountToDistribute -= WeightLimitAddAmount;

			ItemIn->SetQuantity(AmountToDistribute);

			// if max weight capacity is reached, no need to run the loop again
			if (InventoryTotalWeight + ExistingItemStack->GetItemSingleWeight() > InventoryWeightCapacity)
			{
				OnInventoryUpdated.Broadcast();
				return RequestedAddAmount - AmountToDistribute;
			}
		}
		else if (WeightLimitAddAmount <= 0)
		{
			if (AmountToDistribute != RequestedAddAmount)
			{
				//this block will be reached if distributing an item across multiple stacks
				//and the weight limit is hit during that process
				OnInventoryUpdated.Broadcast();
				return RequestedAddAmount - AmountToDistribute;				
			}

			//reached if there is a partial stack but none of it can be added at all
			return 0;
			
		}

		if (AmountToDistribute <= 0)
		{
			//all of the incoming items were distributed across existing stacks
			OnInventoryUpdated.Broadcast();
			return RequestedAddAmount;
		}

		//check for another valid partial stack
		ExistingItemStack = FindNextPartialStack(ItemIn);
		
	}	//no more partial stacks found

	//if there is capacity to hold incoming item as a new stack
	if(InventoryContents.Num() + 1 <= InventorySlotsCapacity )
	{
		//attempt to add as many from the remaining item quantity that can fit inventory weight capacity
		const int32 WeightLimitAddAmount =  CalculateWeightAddAmount(ItemIn, AmountToDistribute);

		if (WeightLimitAddAmount > 0)
		{
			if (WeightLimitAddAmount < AmountToDistribute)
			{
				//adjust the input item and add a new stack with as many items as can be held
				AmountToDistribute -= WeightLimitAddAmount;
				ItemIn->SetQuantity(AmountToDistribute);

				//create a copy since only a partial stack is being added (items left behind) 
				AddNewItem(ItemIn->CreateItemCopy(), WeightLimitAddAmount);
				return RequestedAddAmount - AmountToDistribute;
			}
			
			//otherwise, the full remainder of the stack is added
			AddNewItem(ItemIn, AmountToDistribute);
			return RequestedAddAmount;
		}

		//reached if there are free item slots, but no remaining weight capacity
		return RequestedAddAmount - AmountToDistribute;
	}
	
	// can only be reached if there is no existing stack and no remaining capacity slots
	return 0;

}


FItemAddResult UInventoryComponent::HandleAddItem(UItemBase* InputItem) 
{
	if (GetOwner())
	{
		const int32 InitialRequestedAddAmount = InputItem->Quantity;

		// handle non-stackable items
		if (!InputItem->ItemNumericData.bIsStackable)
		{
			return HandleNonStackableItems(InputItem, InitialRequestedAddAmount);
		}

		// handle stackable items
		const int32 StackableAmountAdded = HandleStackableItems(InputItem, InitialRequestedAddAmount);

		if (StackableAmountAdded == InitialRequestedAddAmount)
		{
			//return added all amount result
			return FItemAddResult::AddedAll(InitialRequestedAddAmount,
				FText::Format(FText::FromString("Added {0} to the inventory."),
					InputItem->ItemTextData.Name));

		}

		if (StackableAmountAdded < InitialRequestedAddAmount  &&  StackableAmountAdded > 0)
		{
			//return added partial amount result
			return FItemAddResult::AddedPartial(StackableAmountAdded, FText::Format(
				FText::FromString("Partial Stack Added of {0} x{1} to the inventory."),
				InputItem->ItemTextData.Name,StackableAmountAdded));
		}

		if (StackableAmountAdded <= 0)
		{
			//return none added result
			return FItemAddResult::AddedNone(FText::Format(
				FText::FromString("Could not add {0} to the inventory. No available slots or invalid item."),
				InputItem->ItemTextData.Name));

		}
		
	}

	return FItemAddResult::AddedNone(FText::Format(
		FText::FromString("Could not add {0} to the inventory.  GetOwner() Check Failed."),
		InputItem->ItemTextData.Name));

	
}

void UInventoryComponent::AddNewItem(UItemBase* Item, const int32 AmountToAdd)
{
	UItemBase* NewItem;

	if (Item->bIsCopy || Item->bIsPickup)
	{
		NewItem = Item;
		NewItem->ResetItemFlags();
	}
	else
	{
		NewItem = Item->CreateItemCopy();
	}

	NewItem->OwningInventory = this;
	NewItem->SetQuantity(AmountToAdd);

	InventoryContents.Add(NewItem);
	InventoryTotalWeight += NewItem->GetItemStackWeight();

	OnInventoryUpdated.Broadcast();
	
}




