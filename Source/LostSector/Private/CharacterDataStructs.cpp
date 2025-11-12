#include "CharacterDataStructs.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool LoadCharacterDataFromJson(const FString& FilePath, FCharacterData& OutData)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		JsonObject->TryGetNumberField(TEXT("Name"), OutData.Hp);
		JsonObject->TryGetNumberField(TEXT("Level"), OutData.Stamina);
		JsonObject->TryGetNumberField(TEXT("Level"), OutData.hungry);
		JsonObject->TryGetNumberField(TEXT("Level"), OutData.weight);
		const TArray<TSharedPtr<FJsonValue>>* AbilitiesArray;
		if (JsonObject->TryGetArrayField(TEXT("Abilities"), AbilitiesArray))
		{
			OutData.Abilities.Empty();
			for (const TSharedPtr<FJsonValue>& Value : *AbilitiesArray)
			{
				OutData.Abilities.Add(Value->AsString());
			}
		}
		return true;
	}
	return false;
}
