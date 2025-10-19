// Fill out your copyright notice in the Description page of Project Settings.


#include "SpartaGameState.h"
#include "SpartaGameInstance.h"
#include "SpartaPlayerController.h"
#include "SpawnVolume.h"
#include "CoinItem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include <Components/ProgressBar.h>

ASpartaGameState::ASpartaGameState()
{
	Score = 0;
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;

	CurrentLevelIndex = 0;
	if (LevelMapNames.Num() == 0)
	{
		LevelMapNames = { FName(TEXT("BasicLevel")), FName(TEXT("AdvancedLevel")), FName(TEXT("IntermediateLevel")) };
	}
	MaxLevels = LevelMapNames.Num(); 

	CurrentWaveIndex = -1;
	MaxWaves = 3;                 
	DefaultWaveDuration = 20.0f;
	ItemsToSpawnBase = 20;
	ItemsToSpawnPerWave = 10;
}

void ASpartaGameState::BeginPlay()
{
	Super::BeginPlay();


	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&ASpartaGameState::UpdateHUD,
		0.1f,
		true
	);

	StartLevel();
}

int32 ASpartaGameState::GetScore() const
{
	return Score;
}

void ASpartaGameState::AddScore(int32 Amount)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance))
		{
			SpartaGameInstance->AddToScore(Amount);
		}
	}
}

void ASpartaGameState::StartLevel()
{

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SPC = Cast<ASpartaPlayerController>(PC))
		{
			SPC->ShowGameHUD();
		}
	}


	if (UGameInstance* GI = GetGameInstance())
	{
		if (USpartaGameInstance* SGI = Cast<USpartaGameInstance>(GI))
		{
			CurrentLevelIndex = SGI->CurrentLevelIndex;
		}
	}


	CachedSpawnVolumes.Reset();
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), Found);
	for (AActor* A : Found)
	{
		if (ASpawnVolume* V = Cast<ASpawnVolume>(A))
		{
			CachedSpawnVolumes.Add(V);
		}
	}


	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;


	StartWave(0);
}

void ASpartaGameState::StartWave(int32 WaveIndex)
{
	CurrentWaveIndex = WaveIndex;


	const FString WaveMsg = FString::Printf(TEXT("Wave %d ½ÃÀÛ!"), CurrentWaveIndex + 1);
	UE_LOG(LogTemp, Log, TEXT("%s"), *WaveMsg);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 2.5f, FColor::Cyan,
			WaveMsg
		);
	}


	const int32 ItemsToSpawn = ItemsToSpawnBase + ItemsToSpawnPerWave * (CurrentWaveIndex + 1);
	SpawnWaveItems(ItemsToSpawn);


	float WaveTime = DefaultWaveDuration;
	if (WaveDurations.IsValidIndex(CurrentWaveIndex))
	{
		WaveTime = WaveDurations[CurrentWaveIndex];
	}


	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	GetWorldTimerManager().SetTimer(
		WaveTimerHandle,
		this,
		&ASpartaGameState::OnWaveTimeUp,
		WaveTime,
		false
	);
}

void ASpartaGameState::SpawnWaveItems(int32 ItemsToSpawn)
{
	if (CachedSpawnVolumes.Num() == 0) return;


	ASpawnVolume* Volume = CachedSpawnVolumes[0];
	if (!Volume) return;

	for (int32 i = 0; i < ItemsToSpawn; ++i)
	{
		AActor* Spawned = Volume->SpawnRandomItem();
		if (Spawned && Spawned->IsA(ACoinItem::StaticClass()))
		{
			SpawnedCoinCount++;
		}
	}
}

void ASpartaGameState::OnWaveTimeUp()
{
	EndWave();
}

void ASpartaGameState::EndWave()
{

	const bool bHasNextWave = (CurrentWaveIndex + 1) < MaxWaves;
	if (bHasNextWave)
	{
		StartWave(CurrentWaveIndex + 1);
	}
	else
	{
		EndLevel();
	}
}

void ASpartaGameState::OnCoinCollected()
{
	CollectedCoinCount++;


	if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
	{

		GetWorldTimerManager().ClearTimer(WaveTimerHandle);
		EndWave();
	}
}

void ASpartaGameState::EndLevel()
{

	GetWorldTimerManager().ClearTimer(WaveTimerHandle);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (USpartaGameInstance* SGI = Cast<USpartaGameInstance>(GI))
		{
			AddScore(Score);

			CurrentLevelIndex++;
			SGI->CurrentLevelIndex = CurrentLevelIndex;


			if (CurrentLevelIndex >= MaxLevels)
			{
				OnGameOver();
				return;
			}


			if (LevelMapNames.IsValidIndex(CurrentLevelIndex))
			{
				UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
			}
			else
			{
				OnGameOver();
			}
		}
	}
}

void ASpartaGameState::OnGameOver()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SPC = Cast<ASpartaPlayerController>(PC))
		{
			SPC->ShowMainMenu(true);
		}
	}
}

float ASpartaGameState::GetRemainingWaveTime() const
{
	return GetWorldTimerManager().GetTimerRemaining(WaveTimerHandle);
}

void ASpartaGameState::UpdateHUD()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SPC = Cast<ASpartaPlayerController>(PC))
		{
			if (UUserWidget* HUD = SPC->GetHUDWidget())
			{

				if (UTextBlock* TimeText = Cast<UTextBlock>(HUD->GetWidgetFromName(TEXT("Time"))))
				{
					const float Remaining = GetRemainingWaveTime();
					TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), Remaining)));
				}


				if (UTextBlock* ScoreText = Cast<UTextBlock>(HUD->GetWidgetFromName(TEXT("Score"))))
				{
					if (UGameInstance* GI = GetGameInstance())
					{
						if (USpartaGameInstance* SGI = Cast<USpartaGameInstance>(GI))
						{
							ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), SGI->TotalScore)));
						}
					}
				}


				if (UTextBlock* LevelText = Cast<UTextBlock>(HUD->GetWidgetFromName(TEXT("Level"))))
				{
					LevelText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), CurrentLevelIndex + 1)));
				}


				if (UTextBlock* WaveText = Cast<UTextBlock>(HUD->GetWidgetFromName(TEXT("Wave"))))
				{
					WaveText->SetText(FText::FromString(FString::Printf(TEXT("Wave: %d / %d"), CurrentWaveIndex + 1, MaxWaves)));
				}



				if (UProgressBar* HPBar = Cast<UProgressBar>(HUD->GetWidgetFromName(TEXT("HealthBar"))))
				{
					APawn* Pawn = PC->GetPawn();
					if (AActor* Actor = Cast<AActor>(Pawn))
					{

						UFunction* Func = Actor->FindFunction(TEXT("GetHealthPercent"));
						if (Func)
						{
							struct FGetHealthPercent_Params { float ReturnValue; };
							FGetHealthPercent_Params Params;
							Actor->ProcessEvent(Func, &Params);

							HPBar->SetPercent(Params.ReturnValue);
						}
					}
				}





			}
		}
	}
}