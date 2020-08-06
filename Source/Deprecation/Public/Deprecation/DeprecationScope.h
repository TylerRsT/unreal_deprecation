

#pragma once

#include "CoreMinimal.h"

#include "DeprecationProperty.h"

//------------------------
class DEPRECATION_API FDeprecationScope final
{
	// Constructors
public:
	FDeprecationScope(UObject* Object, FStructuredArchive::FRecord& Record);
	FDeprecationScope(const FDeprecationScope& Other) = delete;



	
	// Destructors
public:
	~FDeprecationScope();




	// Methods
private:
	bool CheckDeprecation(uint64& AssetVersion, uint64& CodeVersion);
	void GenerateRoot(FDeprecationProperty::Map& TargetMap, FStructuredArchive::FStream& Stream);




	// Operators overload
public:
	FDeprecationScope& operator=(const FDeprecationScope& Other) = delete;




	// Properties
public:
	inline const FDeprecationProperty::Map& GetRoot() const
	{
		return Root;
	}




	// Fields
private:
	UObject* Object;
	FStructuredArchive::FRecord* Record;

	uint64 PreSerializePosition;
	uint64 PostSerializePosition;

	FDeprecationProperty::Map Root;

	bool bIsLoading;
};

#if !defined(UE_BUILD_SHIPPING)
#define DEPRECATION_SCOPE(Object, Record) FDeprecationScope(Object, Record);
#else
#define DEPRECATION_SCOPE(Object, Record)
#endif // UE_BUILD_SHIPPING