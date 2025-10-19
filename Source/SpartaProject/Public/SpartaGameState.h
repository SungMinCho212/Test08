// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SpartaGameState.generated.h"

class ASpawnVolume;

UCLASS()
class SPARTAPROJECT_API ASpartaGameState : public AGameState
{
	GENERATED_BODY()
public:
	ASpartaGameState();
	virtual void BeginPlay() override;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Score")
	int32 Score;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
	int32 SpawnedCoinCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
	int32 CollectedCoinCount;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 CurrentLevelIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 MaxLevels;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TArray<FName> LevelMapNames;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 CurrentWaveIndex;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 MaxWaves;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TArray<float> WaveDurations;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	float DefaultWaveDuration;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 ItemsToSpawnBase;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 ItemsToSpawnPerWave;

	
	UFUNCTION(BlueprintPure, Category = "Wave")
	float GetRemainingWaveTime() const;

	
	UFUNCTION(BlueprintPure, Category = "Score")
	int32 GetScore() const;

	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(int32 Amount);

	
	UFUNCTION(BlueprintCallable, Category = "Level")
	void OnGameOver();

	
	void OnCoinCollected();

	
	void UpdateHUD();

protected:
	
	void StartLevel();

	
	void StartWave(int32 WaveIndex);
	void EndWave();
	void OnWaveTimeUp();

	
	void EndLevel();

	
	void SpawnWaveItems(int32 ItemsToSpawn);

	
	FTimerHandle WaveTimerHandle;
	FTimerHandle HUDUpdateTimerHandle;

	
	UPROPERTY()
	TArray<ASpawnVolume*> CachedSpawnVolumes;
};