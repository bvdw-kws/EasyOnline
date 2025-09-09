
#pragma once

#include "Engine/DataAsset.h"

#include "EasyOnlineGameModeAsset.generated.h"

UCLASS()
class EASYONLINE_API UEasyOnlineGameModeAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Unique identifier that can be used to reference this game mode.
	 * This identifier could be used to unlock game modes or during matchmaking.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName GameModeID = NAME_None;
	
	/**
	 * Priority while sorting game modes in the menu.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MenuSortOrder = 0;

	/**
	 * Class used by this game mode.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftClassPtr<class AEasyOnlineGameMode_InGame> GameModeClass;

public:
	static const FPrimaryAssetType AssetType;

protected:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
