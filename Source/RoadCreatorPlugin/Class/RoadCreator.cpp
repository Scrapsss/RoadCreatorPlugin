// Fill out your copyright notice in the Description page of Project Settings.
#include "RoadCreator.h"

#include "InterchangeResult.h"
#include "SNegativeActionButton.h"
#include "SWarningOrErrorBox.h"
#include "Components/SplineMeshComponent.h"
#include "RoadCreatorPlugin/Struct/SDataSplineMesh.h"


// Sets default values
ARoadCreator::ARoadCreator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SplineComponent = CreateDefaultSubobject<USplineComponent>("SplineMesh");
	SetRootComponent(SplineComponent);
}

// Called when the game starts or when spawned
void ARoadCreator::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ARoadCreator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARoadCreator::GenerateRoad()
{
	// Si le mode n'est pas activé alors on lance pas la generation
	if (!LiveGeneration) return;
	
	// Si aucun mesh n'est renseigné alors on coupe
	if (RoadMeshes.Num() <= 0)
	{
		return;
	}
	
	// Pour éviter les crash :3
	if (RoadMeshes.Find(ERoadType::Road)->Get() == nullptr)
	{
		return;
	}
	
	// Définition du nombre de mesh à générer
	FVector MeshSize = RoadMeshes.Find(ERoadType::Road)->Get()->GetBoundingBox().Max - RoadMeshes.Find(ERoadType::Road)->Get()->GetBoundingBox().Min;
	int NumberOfMeshes = FMath::CeilToInt(SplineComponent->GetSplineLength() / MeshSize.X);
	
	// -- Variable mémoire pour que chaque SplineMesh connaisse son précédent
	USplineMeshComponent* PrecedentNeighbour = nullptr;
	
	// -- On va supprimer tout les Mesh en trop si jamais il y en a
	if (GeneratedSplineMeshes.Num() > NumberOfMeshes)
	{
		for (int i = 0; i < GeneratedSplineMeshes.Num() - NumberOfMeshes; i++)
		{
			GeneratedSplineMeshes[GeneratedSplineMeshes.Num() - i - 1]->DestroyComponent();
			GeneratedSplineMeshes.RemoveAt(GeneratedSplineMeshes.Num() - i - 1);
		}
	}
	
	// On va calculer chaque spline mesh et les comparer aux splines déjà éxistante (si il y en a)
	for (int i = 0; i < NumberOfMeshes; i++)
	{
		// -- Variables Locales
		FSDataSplineMesh NewSplineMesh;
		int RoadParameterIndex = GetRoadPointParameter(MeshSize.X * i);
		if (RoadParameterIndex == -1)
		{
			continue;
		}
		FSRoadPointParameter RoadParameters = RoadPointParameters[RoadParameterIndex];
		// --
		
		// -- Calcul des positions & Tangentes
		FVector StartPos = SplineComponent->GetLocationAtDistanceAlongSpline(MeshSize.X * i, ESplineCoordinateSpace::Local);
		FVector EndPos = SplineComponent->GetLocationAtDistanceAlongSpline(FMath::Min(MeshSize.X * (i + 1),SplineComponent->GetSplineLength()), ESplineCoordinateSpace::Local);
		
		FVector StartTang = SplineComponent->GetTangentAtDistanceAlongSpline(MeshSize.X * i, ESplineCoordinateSpace::Local);
		FVector EndTang = SplineComponent->GetTangentAtDistanceAlongSpline(MeshSize.X * (i + 1), ESplineCoordinateSpace::Local);
		
		StartTang = StartTang.GetClampedToMaxSize(MeshSize.X);
		EndTang = EndTang.GetClampedToMaxSize(MeshSize.X);
		
		NewSplineMesh.StartPos = StartPos;
		NewSplineMesh.EndPos = EndPos;
		
		NewSplineMesh.StartTang = StartTang;
		NewSplineMesh.EndTang = EndTang;
		// --
		
		// -- Calcul de la scale
		// StartScale
		if (PrecedentNeighbour)
		{
			NewSplineMesh.StartScale = PrecedentNeighbour->GetEndScale();
		}
		else
		{
			NewSplineMesh.StartScale = FVector2D(1, RoadPointParameters[0].Width);
		}
		// --
		
		// EndScale
		// Si il éxiste un point devant la route alors il faut gérer la scale en fonction de la largeur du prochain point
		if (RoadParameterIndex + 1 <= RoadPointParameters.Num())
		{
			FSRoadPointParameter nextRoadParameter = RoadPointParameters[RoadParameterIndex + 1];
			float StartDistance = SplineComponent->GetDistanceAlongSplineAtLocation(SplineComponent->GetSplinePointAt(RoadParameterIndex, ESplineCoordinateSpace::World).Position, ESplineCoordinateSpace::World);
			float EndDistance = SplineComponent->GetDistanceAlongSplineAtLocation(SplineComponent->GetSplinePointAt(RoadParameterIndex + 1, ESplineCoordinateSpace::World).Position, ESplineCoordinateSpace::World);
			
			float LerpValue = FMath::GetMappedRangeValueClamped(FVector2D(StartDistance,EndDistance), FVector2D(0, 1), MeshSize.X * i);
			
			float LerpedRoadScale = FMath::Lerp(RoadParameters.Width, nextRoadParameter.Width, LerpValue);
			NewSplineMesh.EndScale = (FVector2D(1, LerpedRoadScale));
		}
		else
		{
			NewSplineMesh.EndScale = (FVector2D(1, RoadParameters.Width));
		}
		
		// -- Parametrage de la staticMesh
		UStaticMesh* RoadMesh = GetRoadStaticMesh(RoadParameters.RoadType);
		if (RoadMesh != nullptr)
		{
			NewSplineMesh.StaticMesh = RoadMesh;
		}
		else
		{
			NewSplineMesh.StaticMesh = GetRoadStaticMesh(ERoadType::Road); // Normalement il y en aura forcément un
		}
		
		// Maintenant on compare ce nouveau mesh calculé avec celui déjà en place pour voir si jamais il est different
		if (GeneratedSplineMeshes.Num() > i) // Si il éxiste un mesh avec le même "ID"
		{
			if (GeneratedSplineMeshes[i] == nullptr)
			{
				GeneratedSplineMeshes.RemoveAt(i);
				USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
				
				SplineMesh->SetMobility(EComponentMobility::Movable);
				SplineMesh->SetupAttachment(SplineComponent);
				SplineMesh->SetForwardAxis(ESplineMeshAxis::Y);
				SplineMesh->SetStartRoll(FMath::DegreesToRadians(-90.0f));
				SplineMesh->SetEndRoll(FMath::DegreesToRadians(-90.0f));
				SplineMesh->SetStartAndEnd(
					NewSplineMesh.StartPos, 
					NewSplineMesh.StartTang,
					NewSplineMesh.EndPos,
					NewSplineMesh.EndTang);
				SplineMesh->SetStaticMesh(NewSplineMesh.StaticMesh);
				SplineMesh->SetStartScale(NewSplineMesh.StartScale);
				SplineMesh->SetEndScale(NewSplineMesh.EndScale);
				SplineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				SplineMesh->SetCollisionResponseToAllChannels(ECR_Block);
				SplineMesh->UpdateMesh();
		
				SplineMesh->RegisterComponent();
				
				GeneratedSplineMeshes.Add(SplineMesh);
				
			}
			else if (IsSplineMeshDifferent(GeneratedSplineMeshes[i], NewSplineMesh))
			{
				USplineMeshComponent* SplineMesh = GeneratedSplineMeshes[i];
			
				SplineMesh->SetStartAndEnd(
					NewSplineMesh.StartPos, 
					NewSplineMesh.StartTang,
					NewSplineMesh.EndPos,
					NewSplineMesh.EndTang);
				SplineMesh->SetStaticMesh(NewSplineMesh.StaticMesh);
				SplineMesh->SetStartScale(NewSplineMesh.StartScale);
				SplineMesh->SetEndScale(NewSplineMesh.EndScale);
				SplineMesh->UpdateMesh();
			}
		}
		else // Si c'est un nouveau SplineMesh
		{
			USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
				
			SplineMesh->SetMobility(EComponentMobility::Movable);
			SplineMesh->SetupAttachment(SplineComponent);
			SplineMesh->SetForwardAxis(ESplineMeshAxis::Y);
			SplineMesh->SetStartRoll(FMath::DegreesToRadians(-90.0f));
			SplineMesh->SetEndRoll(FMath::DegreesToRadians(-90.0f));
			SplineMesh->SetStartAndEnd(
				NewSplineMesh.StartPos, 
				NewSplineMesh.StartTang,
				NewSplineMesh.EndPos,
				NewSplineMesh.EndTang);
			SplineMesh->SetStaticMesh(NewSplineMesh.StaticMesh);
			SplineMesh->SetStartScale(NewSplineMesh.StartScale);
			SplineMesh->SetEndScale(NewSplineMesh.EndScale);
			SplineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			SplineMesh->SetCollisionResponseToAllChannels(ECR_Block);
			SplineMesh->UpdateMesh();
		
			SplineMesh->RegisterComponent();
				
			GeneratedSplineMeshes.Add(SplineMesh);
		}
		
		// On set le mesh précedent pour la prochaine boucle
		PrecedentNeighbour = GeneratedSplineMeshes[i];
	}
}


void ARoadCreator::OldGenerateRoad()
{
	// Si aucun mesh n'est renseigné alors on coupe
	if (RoadMeshes.Num() <= 0)
	{
		return;
	}
	
	// Pour éviter les crash :3
	if (RoadMeshes.Find(ERoadType::Road)->Get() == nullptr)
	{
		return;
	}
	
	// Définition du nombre de mesh à générer
	FVector MeshSize = RoadMeshes.Find(ERoadType::Road)->Get()->GetBoundingBox().Max - RoadMeshes.Find(ERoadType::Road)->Get()->GetBoundingBox().Min;
	int NumberOfMeshes = FMath::CeilToInt(SplineComponent->GetSplineLength() / MeshSize.X);
	
	// -- Variable mémoire pour que chaque SplineMesh connaisse son précédent
	USplineMeshComponent* PrecedentNeighbour = nullptr;
	
	// -- On va supprimer tout les Mesh en trop si jamais il y en a
	if (GeneratedSplineMeshes.Num() > NumberOfMeshes)
	{
		for (int i = 0; i < GeneratedSplineMeshes.Num() - NumberOfMeshes; i++)
		{
			GeneratedSplineMeshes[GeneratedSplineMeshes.Num() - i - 1]->DestroyComponent();
			GeneratedSplineMeshes.RemoveAt(GeneratedSplineMeshes.Num() - i - 1);
		}
	}
	
	// On va calculer chaque spline mesh et les comparer aux splines déjà éxistante (si il y en a)
	for (int i = 0; i < NumberOfMeshes; i++)
	{
		// -- Variables Locales
		FSDataSplineMesh NewSplineMesh;
		int RoadParameterIndex = GetRoadPointParameter(MeshSize.X * i);
		if (RoadParameterIndex == -1)
		{
			continue;
		}
		FSRoadPointParameter RoadParameters = RoadPointParameters[RoadParameterIndex];
		// --
		
		// -- Calcul des positions & Tangentes
		FVector StartPos = SplineComponent->GetLocationAtDistanceAlongSpline(MeshSize.X * i, ESplineCoordinateSpace::Local);
		FVector EndPos = SplineComponent->GetLocationAtDistanceAlongSpline(FMath::Min(MeshSize.X * (i + 1),SplineComponent->GetSplineLength()), ESplineCoordinateSpace::Local);
		
		FVector StartTang = SplineComponent->GetTangentAtDistanceAlongSpline(MeshSize.X * i, ESplineCoordinateSpace::Local);
		FVector EndTang = SplineComponent->GetTangentAtDistanceAlongSpline(MeshSize.X * (i + 1), ESplineCoordinateSpace::Local);
		
		StartTang = StartTang.GetClampedToMaxSize(MeshSize.X);
		EndTang = EndTang.GetClampedToMaxSize(MeshSize.X);
		
		NewSplineMesh.StartPos = StartPos;
		NewSplineMesh.EndPos = EndPos;
		
		NewSplineMesh.StartTang = StartTang;
		NewSplineMesh.EndTang = EndTang;
		// --
		
		// -- Calcul de la scale
		// StartScale
		if (PrecedentNeighbour)
		{
			NewSplineMesh.StartScale = PrecedentNeighbour->GetEndScale();
		}
		else
		{
			NewSplineMesh.StartScale = FVector2D(1, RoadPointParameters[0].Width);
		}
		// --
		
		// EndScale
		// Si il éxiste un point devant la route alors il faut gérer la scale en fonction de la largeur du prochain point
		if (RoadParameterIndex + 1 <= RoadPointParameters.Num())
		{
			FSRoadPointParameter nextRoadParameter = RoadPointParameters[RoadParameterIndex + 1];
			float StartDistance = SplineComponent->GetDistanceAlongSplineAtLocation(SplineComponent->GetSplinePointAt(RoadParameterIndex, ESplineCoordinateSpace::World).Position, ESplineCoordinateSpace::World);
			float EndDistance = SplineComponent->GetDistanceAlongSplineAtLocation(SplineComponent->GetSplinePointAt(RoadParameterIndex + 1, ESplineCoordinateSpace::World).Position, ESplineCoordinateSpace::World);
			
			float LerpValue = FMath::GetMappedRangeValueClamped(FVector2D(StartDistance,EndDistance), FVector2D(0, 1), MeshSize.X * i);
			
			float LerpedRoadScale = FMath::Lerp(RoadParameters.Width, nextRoadParameter.Width, LerpValue);
			NewSplineMesh.EndScale = (FVector2D(1, LerpedRoadScale));
		}
		else
		{
			NewSplineMesh.EndScale = (FVector2D(1, RoadParameters.Width));
		}
		
		// -- Parametrage de la staticMesh
		UStaticMesh* RoadMesh = GetRoadStaticMesh(RoadParameters.RoadType);
		if (RoadMesh != nullptr)
		{
			NewSplineMesh.StaticMesh = RoadMesh;
		}
		else
		{
			NewSplineMesh.StaticMesh = GetRoadStaticMesh(ERoadType::Road); // Normalement il y en aura forcément un
		}
		
		// Maintenant on compare ce nouveau mesh calculé avec celui déjà en place pour voir si jamais il est different
		if (GeneratedSplineMeshes.Num() > i) // Si il éxiste un mesh avec le même "ID"
		{
			if (GeneratedSplineMeshes[i] == nullptr)
			{
				GeneratedSplineMeshes.RemoveAt(i);
				USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
				
				SplineMesh->SetMobility(EComponentMobility::Movable);
				SplineMesh->SetupAttachment(SplineComponent);
				SplineMesh->SetForwardAxis(ESplineMeshAxis::Y);
				SplineMesh->SetStartRoll(FMath::DegreesToRadians(-90.0f));
				SplineMesh->SetEndRoll(FMath::DegreesToRadians(-90.0f));
				SplineMesh->SetStartAndEnd(
					NewSplineMesh.StartPos, 
					NewSplineMesh.StartTang,
					NewSplineMesh.EndPos,
					NewSplineMesh.EndTang);
				SplineMesh->SetStaticMesh(NewSplineMesh.StaticMesh);
				SplineMesh->SetStartScale(NewSplineMesh.StartScale);
				SplineMesh->SetEndScale(NewSplineMesh.EndScale);
				SplineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				SplineMesh->SetCollisionResponseToAllChannels(ECR_Block);
				SplineMesh->UpdateMesh();
		
				SplineMesh->RegisterComponent();
				
				GeneratedSplineMeshes.Add(SplineMesh);
				
			}
			else if (IsSplineMeshDifferent(GeneratedSplineMeshes[i], NewSplineMesh))
			{
				USplineMeshComponent* SplineMesh = GeneratedSplineMeshes[i];
			
				SplineMesh->SetStartAndEnd(
					NewSplineMesh.StartPos, 
					NewSplineMesh.StartTang,
					NewSplineMesh.EndPos,
					NewSplineMesh.EndTang);
				SplineMesh->SetStaticMesh(NewSplineMesh.StaticMesh);
				SplineMesh->SetStartScale(NewSplineMesh.StartScale);
				SplineMesh->SetEndScale(NewSplineMesh.EndScale);
				SplineMesh->UpdateMesh();
			}
		}
		else // Si c'est un nouveau SplineMesh
		{
			USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
				
			SplineMesh->SetMobility(EComponentMobility::Movable);
			SplineMesh->SetupAttachment(SplineComponent);
			SplineMesh->SetForwardAxis(ESplineMeshAxis::Y);
			SplineMesh->SetStartRoll(FMath::DegreesToRadians(-90.0f));
			SplineMesh->SetEndRoll(FMath::DegreesToRadians(-90.0f));
			SplineMesh->SetStartAndEnd(
				NewSplineMesh.StartPos, 
				NewSplineMesh.StartTang,
				NewSplineMesh.EndPos,
				NewSplineMesh.EndTang);
			SplineMesh->SetStaticMesh(NewSplineMesh.StaticMesh);
			SplineMesh->SetStartScale(NewSplineMesh.StartScale);
			SplineMesh->SetEndScale(NewSplineMesh.EndScale);
			SplineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			SplineMesh->SetCollisionResponseToAllChannels(ECR_Block);
			SplineMesh->UpdateMesh();
		
			SplineMesh->RegisterComponent();
				
			GeneratedSplineMeshes.Add(SplineMesh);
		}
		
		// On set le mesh précedent pour la prochaine boucle
		PrecedentNeighbour = GeneratedSplineMeshes[i];
	}
}


void ARoadCreator::SetRoadPointParameters()
{
	float PointAdded = SplineComponent->GetNumberOfSplinePoints() - RoadPointParameters.Num();
	RoadPointParameters.SetNum(SplineComponent->GetNumberOfSplinePoints());
	if (PointAdded > 0)
	{
		RoadPointParameters[RoadPointParameters.Num() - PointAdded].Width = DefaultWidth;
	}
}

int ARoadCreator::GetRoadPointParameter(float DistanceAlongSpline)
{
	int RoadParameterToUse = -1;
	float ClosestDistance = 0;
	for (int i = 0; i < SplineComponent->GetNumberOfSplinePoints(); i++)
	{
		float PointDistance = SplineComponent->GetDistanceAlongSplineAtLocation(SplineComponent->GetSplinePointAt(i, ESplineCoordinateSpace::World).Position, ESplineCoordinateSpace::World);
		
		float Distance = DistanceAlongSpline - PointDistance;
		if (Distance < 0) continue;
		if (Distance < ClosestDistance || ClosestDistance == 0)
		{
			RoadParameterToUse = i;
		}
	}
	return RoadParameterToUse;
}

UStaticMesh* ARoadCreator::GetRoadStaticMesh(ERoadType RoadType)
{
	if (!RoadMeshes.Contains(RoadType))
	{
		return nullptr;
	}
	
	return RoadMeshes.Find(RoadType)->Get();
}

bool ARoadCreator::IsSplineMeshDifferent(USplineMeshComponent* FirstSplineMesh, FSDataSplineMesh SecondSplineMesh)
{
	if (FirstSplineMesh == nullptr) return false;
	
	if (FirstSplineMesh->GetStartPosition() != SecondSplineMesh.StartPos) return true;
	if (FirstSplineMesh->GetEndPosition() != SecondSplineMesh.EndPos) return true;
	if (FirstSplineMesh->GetStartTangent() != SecondSplineMesh.StartTang) return true;
	if (FirstSplineMesh->GetEndTangent() != SecondSplineMesh.EndTang) return true;
	if (FirstSplineMesh->GetStaticMesh() != SecondSplineMesh.StaticMesh) return true;
	if (FirstSplineMesh->GetStartScale() != SecondSplineMesh.StartScale) return true;
	if (FirstSplineMesh->GetEndScale() != SecondSplineMesh.EndScale) return true;
	
	return false;
	
}

void ARoadCreator::ModifyScale(float value, bool WantToSet)
{
	for (FSRoadPointParameter& Point : RoadPointParameters)
	{
		if (WantToSet)
		{
			Point.Width = WidthForAll;
		}
		else
		{
			Point.Width += value;
		}
	}
	
	GenerateRoad();
}

void ARoadCreator::AddScale()
{
	ModifyScale(1, false);
}

void ARoadCreator::SubtractScale()
{
	ModifyScale(-1, false);
}

void ARoadCreator::SetAllPointsWidth()
{
	ModifyScale(0, true);
}
