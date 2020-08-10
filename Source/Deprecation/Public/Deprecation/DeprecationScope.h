

#pragma once

#include "CoreMinimal.h"

#include "DeprecationProperty.h"

#include "UObject/PropertyTag.h"

/**
 * Creates a deprecation property map from an asset so old structure can be handled by new code.
 * Property map is generated and deprecation is handled at destruction time.
 */
class DEPRECATION_API FDeprecationScope final
{
	// Typedefs
public:
	/**
	 * Signature to handle an asset from old structure before converting it to new structure.
	 * @param PropertyMap Holds all the properties from the file with the old structure.
	 * @param AssetVersion Version of the asset at load time.
	 * @param CodeVersion Version of the code.
	 */
	typedef void (UObject::*DeprecationHandler)
		(const FDeprecationProperty::Map& PropertyMap, uint64 AssetVersion, uint64 CodeVersion);



	// Constructors
public:
	/**
	 * Creates a new scope for the given asset.
	 * @param Object Instance of the asset before serialization.
	 * @param Record Pointer to the record file before serialization.
	 * @param Handler Pointer to member function that handles data deprecation.
	 * @param VersionPropertyName Name of the property holding the deprecation version. Mandatory in the class of Object.
	 */
	FDeprecationScope(UObject* Object, FStructuredArchive::FRecord& Record, DeprecationHandler Handler, FString VersionPropertyName = "DeprecationVersion");
	FDeprecationScope(const FDeprecationScope& Other) = delete;



	
	// Destructors
public:
	~FDeprecationScope();




	// Methods
public:
	/**
	 * Helper function to load an asset from the ObjectImport instance.
	 * @param <TObject> Type of the object to load.
	 * @param ObjectImport Instance of the ObjectImport (retrieved in deprecation data).
	 * @returns Instance of the loaded object.
	 */
	template <class TObject>
	inline static TObject* LoadObjectFromImport(const FObjectImport& ObjectImport)
	{
		if (ObjectImport.XObject)
		{
			return Cast<TObject>(ObjectImport.XObject);
		}

		return LoadObject<TObject>
			(nullptr, *ObjectImport.SourceLinker->LinkerRoot->FileName.ToString());
	}

private:
	/**
	 * Checks if asset is deprecated comparing the versions of the asset and the code.
	 * @param AssetVersion Version of the asset (retrieved through the file).
	 * @param CodeVersion Current version of the structure.
	 * @returns True if the asset is deprecated, false otherwise.
	 */
	bool CheckDeprecation(uint64& AssetVersion, uint64& CodeVersion);

	/**
	 * Generates the property map from the asset file.
	 * @param TargetMap Map to fill with found properties.
	 * @param Stream File stream used to retrieve data.
	 */
	void GenerateRoot(FDeprecationProperty::Map& TargetMap, FStructuredArchive::FStream& Stream);

	/**
	 * Generates value data from the file stream.
	 * @param Tag Property tag used to know the type of data streamed.
	 * @param Linker Optional linker instance for Object properties.
	 * @param TargetProperty Property to fill with data.
	 * @param bIsKey Indicates whether data should be stored in keys or values.
	 * @param ValueStream File stream used to retrieve data.
	 */
	void GenerateValue(FPropertyTag& Tag, FLinkerLoad* Linker,
		FDeprecationProperty& TargetProperty, bool bIsKey, FStructuredArchive::FStream& ValueStream);




	// Operators overload
public:
	FDeprecationScope& operator=(const FDeprecationScope& Other) = delete;




	// Properties
public:
	/**
	 * Returns the root map from the current object.
	 */
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

/**
 * Creates a temporary Deprecation Scope for the current asset.
 * @param Object Object to check deprecation for.
 * @param Record Instance of the asset file record, before serialization.
 * @param Handler Pointer to member function that will handle deprecation.
 */
#define DEPRECATION_SCOPE(Object, Record, Handler) FDeprecationScope __DeprScope__(Object, Record, (FDeprecationScope::DeprecationHandler)(Handler));
#define DEPRECATION_SCOPE_CUSTOM_VERSION_PROPERTY(Object, Record, Handler, VersionPropertyName) FDeprecationScope __DeprScope__(Object, Record, (FDeprecationScope::DeprecationHandler)(Handler), VersionPropertyName);
#define DEPRECATION_SCOPE_LOCAL(Handler) DEPRECATION_SCOPE(this, Record, Handler)
#define DEPRECATION_SCOPE_LOCAL_CUSTOM_VERSION_PROPERTY(Handler, VersionPropertyName) DEPRECATION_SCOPE_CUSTOM_VERSION_PROPERTY(Handler, VersionPropertyName)