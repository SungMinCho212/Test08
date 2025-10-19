// Fill out your copyright notice in the Description page of Project Settings.


#include "SpartaGameMode.h"
#include "SpartaCharacter.h"
#include "SpartaPlayerController.h"
#include "SpartaGameState.h"

ASpartaGameMode::ASpartaGameMode()
{
    PlayerControllerClass = ASpartaPlayerController::StaticClass();
    DefaultPawnClass = ASpartaCharacter::StaticClass();
    
    GameStateClass = ASpartaGameState::StaticClass();
}
