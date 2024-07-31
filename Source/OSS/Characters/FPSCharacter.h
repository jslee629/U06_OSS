#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSCharacter.generated.h"

class UInputComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class USoundBase;
class UAnimMontage;

UCLASS(config = Game)
class AFPSCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Arm mesh */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* FP_Mesh;

	/** Gun mesh */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* FP_Gun;

	/** Gun mesh for Client */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* TP_Gun;

	/** camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;

public:
	AFPSCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	/** AnimMontage to play each time we fire (for Other Player) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* TP_FireAnimation;

	/* This is when calculating the trace to determine what the weapon has hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float WeaponRange;
	
	/* This is multiplied by the direction vector when the weapon trace hits something to apply velocity to the component that is hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float WeaponDamage;

protected:

	/** Fires a virtual projectile. */
	void OnFire();

	void MoveForward(float Val);
	void MoveRight(float Val);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	/* 
	 * Performs a trace between two points
	 * 
	 * @param	StartTrace	Trace starting point
	 * @param	EndTrac		Trace end point
	 * @returns FHitResult returns a struct containing trace result - who/what the trace hit etc.
	 */
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
};

