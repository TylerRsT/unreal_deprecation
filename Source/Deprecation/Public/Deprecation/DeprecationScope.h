

#pragma once

#include "CoreMinimal.h"

#include "DeprecationProperty.h"

//------------------------
class DEPRECATION_API FDeprecationScope final
{
	// Typedefs
public:
	typedef void (UObject::*DeprecationHandler)
		(const FDeprecationProperty::Map& PropertyMap, uint64 AssetVersion, uint64 CodeVersion);



	// Constructors
public:
	FDeprecationScope(UObject* Object, FStructuredArchive::FRecord& Record, DeprecationHandler Handler);
	FDeprecationScope(const FDeprecationScope& Other) = delete;



	
	// Destructors
public:
	~FDeprecationScope();




	// Methods
public:
	template <class TObject>
	inline static TObject* LoadObjectFromImport(const FObjectImport& ObjectImport)
	{
		return LoadObject<TObject>
			(nullptr, *ObjectImport.SourceLinker->LinkerRoot->FileName.ToString());
	}

private:
	bool CheckDeprecation(uint64& AssetVersion, uint64& CodeVersion);

	void GenerateRoot(FDeprecationProperty::Map& TargetMap, FStructuredArchive::FStream& Stream);

	void GenerateValue(FPropertyTag& Tag, FLinkerLoad* Linker,
		FDeprecationProperty& TargetProperty, bool bIsKey, FStructuredArchive::FStream& ValueStream);




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
	DeprecationHandler Handler;

	uint64 PreSerializePosition;
	uint64 PostSerializePosition;

	FDeprecationProperty::Map Root;

	bool bIsLoading;
	bool bHasDeprecationProperty;

	FString VersionPropertyName;
};

#define DEPRECATION_SCOPE(Object, Record, Handler) FDeprecationScope __DeprScope__(Object, Record, (FDeprecationScope::DeprecationHandler)(Handler));
#define DEPRECATION_SCOPE_LOCAL(Handler) DEPRECATION_SCOPE(this, Record, Handler)