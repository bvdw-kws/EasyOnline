#pragma once

#include "CoreMinimal.h"
#include "EasyGauntletTest.h"
#include "Gauntlet/EasyOnlineGauntletController.h"
#include "EasyQuickJoinGauntletController.generated.h"

/**
 * Gauntlet controller specifically for testing QuickHost/QuickJoin functionality
 * Tests the same code paths that players use when clicking the UI buttons
 * Includes retry logic for QuickJoin and detailed logging for debugging
 */
UCLASS()
class EASYGAUNTLETTEST_API UEasyQuickJoinGauntletController : public UEasyOnlineGauntletController
{
	GENERATED_BODY()

public:
	UEasyQuickJoinGauntletController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// UEasyGauntletController interface
	virtual void OnInit() override;
	virtual void OnSetupTestStates(TQueue<TUniquePtr<FEasyGauntletState>>& OutStates) override;

private:
	/** Parse EasyOnline-specific command line parameters */
	void ParseEasyQuickJoinParameters();

	/** Timeout for host session creation */
	UPROPERTY()
	float HostTimeout = 60.0f;
	
	/** Timeout for client join attempts */
	UPROPERTY()
	float JoinTimeout = 120.0f;
	
	/** Maximum number of join retry attempts */
	UPROPERTY()
	int32 MaxJoinRetries = 10;
	
	/** Time between join retry attempts */
	UPROPERTY()
	float JoinRetryInterval = 5.0f;
	
	/** Timeout for connection validation */
	UPROPERTY()
	float ValidationTimeout = 30.0f;
	
	/** Map to use for testing (empty = use default) */
	UPROPERTY()
	FString TestMap;
	
	/** Whether to run extended validation tests */
	UPROPERTY()
	bool bExtendedValidation = false;
	
	/** Number of clients to wait for before starting validation */
	UPROPERTY()
	int32 ExpectedClientCount = 1;
};