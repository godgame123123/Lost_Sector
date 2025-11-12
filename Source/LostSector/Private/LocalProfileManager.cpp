#include "LocalProfileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"

FString ULocalProfileManager::GetLocalProfilePath()
{
    FString SaveDir = FPaths::ProjectSavedDir() + TEXT("LocalProfile/");
    return SaveDir + TEXT("PlayerProfile.json");
}

bool ULocalProfileManager::HasLocalProfile()
{
    return FPaths::FileExists(GetLocalProfilePath());
}

bool ULocalProfileManager::SaveLocalProfile(const FLocalPlayerProfile& Profile)
{
    FString JsonString;
    if (!FJsonObjectConverter::UStructToJsonObjectString(Profile, JsonString, 0, 0, 0, nullptr, true))
    {
        return false;
    }

    FString FilePath = GetLocalProfilePath();
    FString Directory = FPaths::GetPath(FilePath);

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*Directory))
    {
        PlatformFile.CreateDirectoryTree(*Directory);
    }

    return FFileHelper::SaveStringToFile(JsonString, *FilePath);
}

bool ULocalProfileManager::LoadLocalProfile(FLocalPlayerProfile& OutProfile)
{
    FString FilePath = GetLocalProfilePath();
    if (!FPaths::FileExists(FilePath))
    {
        return false;
    }

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        return false;
    }

    if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutProfile, 0, 0))
    {
        return false;
    }

    OutProfile.LastPlayedDate = FDateTime::Now();
    SaveLocalProfile(OutProfile);
    return true;
}

FLocalPlayerProfile ULocalProfileManager::CreateNewProfile(const FString& PlayerName)
{
    FLocalPlayerProfile NewProfile;
    NewProfile.PlayerName = PlayerName;
    
    int64 Timestamp = FDateTime::Now().ToUnixTimestamp();
    FString RandomPart = FGuid::NewGuid().ToString().Left(8);
    NewProfile.UniquePlayerID = FString::Printf(TEXT("%s_%lld_%s"), 
        *PlayerName, Timestamp, *RandomPart);
    
    NewProfile.CreatedDate = FDateTime::Now();
    NewProfile.LastPlayedDate = FDateTime::Now();
    
    return NewProfile;
}

bool ULocalProfileManager::DeleteLocalProfile()
{
    FString FilePath = GetLocalProfilePath();
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    return PlatformFile.DeleteFile(*FilePath);
}