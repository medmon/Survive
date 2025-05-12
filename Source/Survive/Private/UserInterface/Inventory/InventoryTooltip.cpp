// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Inventory/InventoryTooltip.h"

#include "ItemBase.h"
#include "Components/TextBlock.h"
#include "UserInterface/Inventory/InventoryItemSlot.h"

void UInventoryTooltip::NativeConstruct()
{
	Super::NativeConstruct();

	UItemBase* ItemBeingHovered = InventorySlotBeingHovered->GetItemReference();

	//switch to hide unneeded fields
	switch (ItemBeingHovered->ItemType)
	{
		case EItemType::Armor:
			ItemType->SetText(FText::FromString("Armor"));
			break;
		case EItemType::Weapon:
			ItemType->SetText(FText::FromString("Weapon"));
			break;
		case EItemType::Shield:
			ItemType->SetText(FText::FromString("Shield"));
			break;
		case EItemType::Spell:
			ItemType->SetText(FText::FromString("Spell"));
			break;
		case EItemType::Quest:
			ItemType->SetText(FText::FromString("Quest"));
			break;
		case EItemType::Consumable:
			ItemType->SetText(FText::FromString("Consumable"));
			DamageValue->SetVisibility(ESlateVisibility::Collapsed);
			ArmorRating->SetVisibility(ESlateVisibility::Collapsed);
			break;
		case EItemType::Pickup:
			ItemType->SetText(FText::FromString("Pickup"));
			break;
		case EItemType::Mundane:
			ItemType->SetText(FText::FromString("Mundane Item"));
			DamageValue->SetVisibility(ESlateVisibility::Collapsed);
			ArmorRating->SetVisibility(ESlateVisibility::Collapsed);
			UsageText->SetVisibility(ESlateVisibility::Collapsed);
			break;
		
		default: ;
	}

	ItemName->SetText(ItemBeingHovered->ItemTextData.Name);
	DamageValue->SetText(FText::AsNumber(ItemBeingHovered->ItemStatistics.DamageValue));
	ArmorRating->SetText(FText::AsNumber(ItemBeingHovered->ItemStatistics.ArmorRating));
	//UsageText->SetText(ItemBeingHovered->ItemTextData.UsageText);
	ItemDescription->SetText(ItemBeingHovered->ItemTextData.Description);
	SellValue->SetText(FText::AsNumber(ItemBeingHovered->ItemStatistics.SellValue));

	const FString WeightInfo =
		{"Weight: " + FString::SanitizeFloat(ItemBeingHovered->GetItemStackWeight())};
	
	StackWeight->SetText(FText::FromString(WeightInfo));

	if (ItemBeingHovered->ItemNumericData.bIsStackable)
	{
		const FString StackInfo =
			{"Max Stack Size: " + FString::FromInt(ItemBeingHovered->ItemNumericData.MaxStackSize)};
		
		MaxStackSize->SetText(FText::FromString(StackInfo));
	}
	else
	{
		MaxStackSize->SetVisibility(ESlateVisibility::Collapsed);
	}
	
}
