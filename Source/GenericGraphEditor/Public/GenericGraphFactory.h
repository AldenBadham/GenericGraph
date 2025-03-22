#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "GenericGraphDefinition.h"
#include "GenericGraphFactory.generated.h"

UCLASS()
class GENERICGRAPHEDITOR_API UGenericGraphFactory : public UFactory
{
	GENERATED_BODY()

public:
	UGenericGraphFactory();
	virtual ~UGenericGraphFactory() override;

	UPROPERTY(EditAnywhere, Category = DataAsset)
	TSubclassOf<UGenericGraphDefinition> GenericGraphClass;

	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
