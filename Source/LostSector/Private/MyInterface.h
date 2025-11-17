// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMyInterface : public UInterface
{
	GENERATED_BODY()
};

#define PURE = 0
class IMyInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Host(FString ServerName)PURE;
	virtual void Join(uint32 Index) PURE;
	virtual void RefreshServerList()PURE;
	virtual void OpenMainMenuLevel()PURE;
};
