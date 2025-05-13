// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Inventory/InventoryItemSlot.h"

#include "IDetailTreeNode.h"
#include "UserInterface/Inventory/InventoryTooltip.h"
#include "ItemBase.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UserInterface/Inventory/DragItemVisual.h"
#include "UserInterface/Inventory/ItemDragDropOperation.h"


void UInventoryItemSlot::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ToolTipClass)
	{
		UInventoryTooltip* ToolTip = CreateWidget<UInventoryTooltip>(this, ToolTipClass);
		ToolTip->InventorySlotBeingHovered = this;
		SetToolTip(ToolTip);
	}
}

void UInventoryItemSlot::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemReference)
	{
		switch ( ItemReference->ItemQuality )
		{
		case EItemQuality::Shoddy:
				ItemBorder->SetBrushColor(FLinearColor::Gray);
				break;
			case EItemQuality::Ordinary:
				ItemBorder->SetBrushColor(FLinearColor::White);
				break;
			case EItemQuality::Fine:
				ItemBorder->SetBrushColor(FLinearColor::Green);
				break;
			case EItemQuality::Masterwork:
				ItemBorder->SetBrushColor(FLinearColor::Blue);
				break;
			case EItemQuality::Grandmaster:
				ItemBorder->SetBrushColor(FLinearColor(.55f, .05f, .55f, 1.0f));   //purple
				break;
			default: ;

		}

		ItemIcon->SetBrushFromTexture(ItemReference->ItemAssetData.Icon);

		if (ItemReference->ItemNumericData.bIsStackable)
		{
			ItemQuantity->SetText(FText::AsNumber(ItemReference->Quantity));
			
		}
		else
		{
			ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);
			
		}
	}
	
}

FReply UInventoryItemSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
//	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return Reply.Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	//TODO:
	//handle submenu on right mouse click
	
	return Reply.Unhandled();
	
}

void UInventoryItemSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UInventoryItemSlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (DragItemVisualClass)
	{
		UDragItemVisual* DragVisual = CreateWidget<UDragItemVisual>(this, DragItemVisualClass);
		DragVisual->ItemIcon->SetBrushFromTexture(ItemReference->ItemAssetData.Icon);
		DragVisual->ItemBorder->SetBrushColor(ItemBorder->GetBrushColor());

		ItemReference->ItemNumericData.bIsStackable ?
			DragVisual->ItemQuantity->SetText(FText::AsNumber(ItemReference->Quantity))
			: DragVisual->ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);

		UItemDragDropOperation* DragItemOperation = NewObject<UItemDragDropOperation>();
		DragItemOperation->SourceItem = ItemReference;
		DragItemOperation->SourceInventory = ItemReference->OwningInventory;

		DragItemOperation->DefaultDragVisual = DragVisual;
		DragItemOperation->Pivot = EDragPivot::TopLeft;

		OutOperation = DragItemOperation;
	}
}

bool UInventoryItemSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
