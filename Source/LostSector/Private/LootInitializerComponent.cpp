// Fill out your copyright notice in the Description page of Project Settings.


#include "LootInitializerComponent.h"

// Sets default values for this component's properties
ULootInitializerComponent::ULootInitializerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void ULootInitializerComponent::InitializeLoot(const TArray<FItemStack>& InventoryToStore)
{
    // 서버 권한 체크 (안전을 위해)
    if (!GetOwner() || GetOwnerRole() != ROLE_Authority)
    {
        UE_LOG(LogTemp, Warning, TEXT("InitializeLoot called on client, skipping."));
        return;
    }

    AActor* OwnerActor = GetOwner();

    // 1. 부착된 액터에서 UInventoryComponent를 찾습니다. (BP_LootBox에 부착되어 있음)
    UInventoryComponent* Inventory = OwnerActor->FindComponentByClass<UInventoryComponent>();

    if (!Inventory)
    {
        UE_LOG(LogTemp, Error, TEXT("ULootInitializerComponent::InitializeLoot failed: Inventory component not found on Owner."));
        return;
    }

    // 2. 인벤토리 초기화 로직
    Inventory->Slots.Empty();
    Inventory->Slots = InventoryToStore;

    // 3. 인벤토리 업데이트 알림 (클라이언트에게 복제 및 UI 업데이트)
    Inventory->BroadcastUpdated();

    UE_LOG(LogTemp, Log, TEXT("LootInitializerComponent initialized with %d item stacks."), InventoryToStore.Num());
}


// Called when the game starts
void ULootInitializerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void ULootInitializerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

