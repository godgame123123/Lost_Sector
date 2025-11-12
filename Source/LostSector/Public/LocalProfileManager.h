// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ItemTypes.h"
#include "LocalProfileManager.generated.h"

UCLASS()
class LOSTSECTOR_API ULocalProfileManager : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 로컬 프로필 존재 여부 확인
    UFUNCTION(BlueprintCallable, Category = "Player|Profile")
    static bool HasLocalProfile();

    // 로컬 프로필 저장
    UFUNCTION(BlueprintCallable, Category = "Player|Profile")
    static bool SaveLocalProfile(const FLocalPlayerProfile& Profile);

    // 로컬 프로필 로드
    UFUNCTION(BlueprintCallable, Category = "Player|Profile")
    static bool LoadLocalProfile(FLocalPlayerProfile& OutProfile);

    // 새 프로필 생성
    UFUNCTION(BlueprintCallable, Category = "Player|Profile")
    static FLocalPlayerProfile CreateNewProfile(const FString& PlayerName);

    // 프로필 삭제
    UFUNCTION(BlueprintCallable, Category = "Player|Profile")
    static bool DeleteLocalProfile();

private:
    static FString GetLocalProfilePath();
};
