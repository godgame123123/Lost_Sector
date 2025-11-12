#include "ServerDataManager.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"


UServerDataManager* UServerDataManager::Instance = nullptr;  // ✅ 줄바꿈 오류 수정


UServerDataManager* UServerDataManager::GetInstance(UWorld* World)
{
    if (!Instance && World && World->GetAuthGameMode())
    {
        Instance = NewObject<UServerDataManager>();
        Instance->AddToRoot(); // GC 방지
    }
    return Instance;
}

FString UServerDataManager::GetPlayerDataPath(const FString& PlayerID)
{
    return FPaths::ProjectSavedDir() / TEXT("PlayerData") / (PlayerID + TEXT(".json"));
}

bool UServerDataManager::LoadPlayerData(const FString& PlayerID, FPlayerData& OutData)
{
    FString FilePath = GetPlayerDataPath(PlayerID);
    FString JsonString;
    
    if (FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
        
        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            return FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &OutData);
        }
    }
    
    // 파일이 없으면 기본 데이터 생성
    OutData = FPlayerData();
    OutData.Money = 10000;
    OutData.Level = 1;
    return true;
}

bool UServerDataManager::SavePlayerData(const FString& PlayerID, const FPlayerData& Data)
{
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Data);
    if (!JsonObject.IsValid())
        return false;
    
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    FString FilePath = GetPlayerDataPath(PlayerID);
    return FFileHelper::SaveStringToFile(JsonString, *FilePath);
}
