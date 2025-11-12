
#include "TestCharacter.h"

ATestCharacter::ATestCharacter()
{
    Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
    Interaction = CreateDefaultSubobject<UInteractionComponent>(TEXT("Interaction"));
}

void ATestCharacter::SetupPlayerInputComponent(UInputComponent* IC)
{
    Super::SetupPlayerInputComponent(IC);
    IC->BindAction("Use", IE_Pressed, this, &ATestCharacter::Use);
}

void ATestCharacter::Use()
{
    if (Interaction) Interaction->Use();
}
