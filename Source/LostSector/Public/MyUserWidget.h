
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyUserWidget.generated.h"

class UButton;

UCLASS()
class LOSTSECTOR_API UMyUserWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

protected:
    UPROPERTY(meta = (BindWidget))
    UButton* StartGame;

    UPROPERTY(meta = (BindWidget))
    UButton* Options;

    UPROPERTY(meta = (BindWidget))
    UButton* QuitGame;

    UFUNCTION()
    void OnStartGameClicked();

    UFUNCTION()
    void OnOptionClicked();

    UFUNCTION()
    void OnQuitGameClicked();
};
