#include "FPSCharacter.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

#define COLLISION_WEAPON		ECC_GameTraceChannel1

AFPSCharacter::AFPSCharacter()
{
	//Properties
	GetCapsuleComponent()->InitCapsuleSize(44.f, 88.f);
	WeaponRange = 5000.0f;
	WeaponDamage = 20.f;
	GunOffset = FVector(100.0f, 30.0f, 10.0f);
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	//->CameraComponent	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->SetRelativeLocation(FVector(0, 0, 64.f));
	CameraComponent->bUsePawnControlRotation = true;

	//->FirstPerson(Only Owner See)
	FP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Mesh"));
	FP_Mesh->SetOnlyOwnerSee(true);							// Set so only owner can see mesh
	FP_Mesh->SetupAttachment(CameraComponent);				// Attach mesh to FirstPersonCameraComponent
	FP_Mesh->bCastDynamicShadow = false;					// Disallow mesh to cast dynamic shadows
	FP_Mesh->CastShadow = false;							// Disallow mesh to cast other shadows

	ConstructorHelpers::FObjectFinder<USkeletalMesh> ArmMeshAsset(TEXT("/Game/FirstPerson/Character/Mesh/SK_Mannequin_Arms"));
	if (ArmMeshAsset.Succeeded())
	{
		FP_Mesh->SetSkeletalMesh(ArmMeshAsset.Object);
	}

	ConstructorHelpers::FClassFinder<UAnimInstance> ArmAnimClass(TEXT("/Game/FirstPerson/Animations/FirstPerson_AnimBP"));
	if (ArmAnimClass.Succeeded())
	{
		FP_Mesh->SetAnimInstanceClass(ArmAnimClass.Class);
	}

	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// Only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;		// Disallow mesh to cast dynamic shadows
	FP_Gun->CastShadow = false;				// Disallow mesh to cast other shadows
	FP_Gun->SetupAttachment(FP_Mesh, TEXT("GripPoint"));


	//->ThirdPerson(Owner No See)
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TPMeshAsset(TEXT("/Game/AnimStarterPack/UE4_Mannequin/Mesh/SK_Mannequin"));
	if (TPMeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(TPMeshAsset.Object);
	}
	GetMesh()->SetOwnerNoSee(true);

	TP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TP_Gun"));
	TP_Gun->SetOwnerNoSee(true);
	TP_Gun->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));

	ConstructorHelpers::FClassFinder<UAnimInstance> TPAnimClass(TEXT("/Game/AnimStarterPack/UE4ASP_HeroTPP_AnimBlueprint"));
	if (TPAnimClass.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(TPAnimClass.Class);
	}
}

void AFPSCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::OnFire);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFPSCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFPSCharacter::LookUpAtRate);
}

void AFPSCharacter::OnFire()
{
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	if (FireAnimation != NULL)
	{
		UAnimInstance* AnimInstance = FP_Mesh->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	FVector ShootDir = FVector::ZeroVector;
	FVector StartTrace = FVector::ZeroVector;

	if (PlayerController)
	{
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(StartTrace, CamRot);
		ShootDir = CamRot.Vector();

		StartTrace = StartTrace + ShootDir * ((GetActorLocation() - StartTrace) | ShootDir);
	}

	const FVector EndTrace = StartTrace + ShootDir * WeaponRange;

	const FHitResult Impact = WeaponTrace(StartTrace, EndTrace);

	AActor* DamagedActor = Impact.GetActor();
	UPrimitiveComponent* DamagedComponent = Impact.GetComponent();

	if ((DamagedActor != NULL) && (DamagedActor != this) && (DamagedComponent != NULL) && DamagedComponent->IsSimulatingPhysics())
	{
		DamagedComponent->AddImpulseAtLocation(ShootDir * WeaponDamage, Impact.Location);
	}
}

void AFPSCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFPSCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFPSCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFPSCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

FHitResult AFPSCharacter::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const
{
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, COLLISION_WEAPON, TraceParams);

	return Hit;
}