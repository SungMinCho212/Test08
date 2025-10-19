

#include "SpartaCharacter.h"
#include "SpartaPlayerController.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

ASpartaCharacter::ASpartaCharacter()
{
 		PrimaryActorTick.bCanEverTick = false;

		SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
		SpringArmComp->SetupAttachment(RootComponent);
		SpringArmComp->TargetArmLength = 300.0f;
		SpringArmComp->bUsePawnControlRotation = true;

		CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
		CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
		CameraComp->bUsePawnControlRotation = false;

        NormalSpeed = 250.0f;
        SprintSpeedMultiplier = 2.0f;
        SprintSpeed = NormalSpeed * SprintSpeedMultiplier;

        GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;

        MaxHealth = 100.0f;
        Health = MaxHealth;
}

void ASpartaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        
        if (ASpartaPlayerController* PlayerController = Cast<ASpartaPlayerController>(GetController()))
        {
            if (PlayerController->MoveAction)
            {
               
                EnhancedInput->BindAction(
                    PlayerController->MoveAction,
                    ETriggerEvent::Triggered,
                    this,
                    &ASpartaCharacter::Move
                );
            }

            if (PlayerController->JumpAction)
            {
                
                EnhancedInput->BindAction(
                    PlayerController->JumpAction,
                    ETriggerEvent::Triggered,
                    this,
                    &ASpartaCharacter::StartJump
                );

                
                EnhancedInput->BindAction(
                    PlayerController->JumpAction,
                    ETriggerEvent::Completed,
                    this,
                    &ASpartaCharacter::StopJump
                );
            }

            if (PlayerController->LookAction)
            {
               
                EnhancedInput->BindAction(
                    PlayerController->LookAction,
                    ETriggerEvent::Triggered,
                    this,
                    &ASpartaCharacter::Look
                );
            }

            if (PlayerController->SprintAction)
            {
                
                EnhancedInput->BindAction(
                    PlayerController->SprintAction,
                    ETriggerEvent::Triggered,
                    this,
                    &ASpartaCharacter::StartSprint
                );
                
                EnhancedInput->BindAction(
                    PlayerController->SprintAction,
                    ETriggerEvent::Completed,
                    this,
                    &ASpartaCharacter::StopSprint
                );
            }
        }
    }
}

void ASpartaCharacter::Move(const FInputActionValue& value)
{
    if (!Controller) return;

    
    const FVector2D MoveInput = value.Get<FVector2D>();

    if (!FMath::IsNearlyZero(MoveInput.X))
    {
        
        AddMovementInput(GetActorForwardVector(), MoveInput.X);
    }

    if (!FMath::IsNearlyZero(MoveInput.Y))
    {
        
        AddMovementInput(GetActorRightVector(), MoveInput.Y);
    }
}

void ASpartaCharacter::StartJump(const FInputActionValue& value)
{
    
    if (value.Get<bool>())
    {
        Jump();
    }
}

void ASpartaCharacter::StopJump(const FInputActionValue& value)
{
    if (!value.Get<bool>())
    {
        StopJumping();
    }
}

void ASpartaCharacter::Look(const FInputActionValue& value)
{
    
    FVector2D LookInput = value.Get<FVector2D>();

    
    AddControllerYawInput(LookInput.X);
    
    AddControllerPitchInput(LookInput.Y);
}

void ASpartaCharacter::StartSprint(const FInputActionValue& value)
{
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    }
}

void ASpartaCharacter::StopSprint(const FInputActionValue& value)
{
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
    }
}

int32 ASpartaCharacter::GetHealth() const
{
    return Health;
}

void ASpartaCharacter::AddHealth(float Amount)
{
    
    Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
    UE_LOG(LogTemp, Log, TEXT("Health increased to: %f"), Health);
}


float ASpartaCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{ 

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    
    Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
    UE_LOG(LogTemp, Warning, TEXT("Health decreased to: %f"), Health);

    
    if (Health <= 0.0f)
    {
        OnDeath();
    }

    
    return ActualDamage;
}
float ASpartaCharacter::GetHealthPercent() const
{
    return MaxHealth > 0.0f ? Health / MaxHealth : 0.0f;
}

void ASpartaCharacter::OnDeath()
{
    if (bIsDead)
    {
        return; 
    }
    bIsDead = true;

    UE_LOG(LogTemp, Warning, TEXT("Character is Dead"));

    
    if (AController* C = GetController())
    {
        if (APlayerController* PC = Cast<APlayerController>(C))
        {
            DisableInput(PC);
        }
    }

    
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->StopMovementImmediately();
        Move->DisableMovement();
    }

   
    

    
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        MeshComp->SetHiddenInGame(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    SetActorHiddenInGame(true);

    DetachFromControllerPendingDestroy();

    
    if (ASpartaPlayerController* PC = Cast<ASpartaPlayerController>(GetController()))
    {
       
        PC->ShowMainMenu(true);
    }
    else
    {
       
        UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("MenuLevel")));
    }

   
    SetLifeSpan(0.1f);
}
