

#include "Deprecation/DeprecationScope.h"

#include "UObject/LinkerLoad.h"
#include "UObject/NoExportTypes.h"
#include "UObject/UnrealType.h"

//------------------------
namespace
{
	//------------------------
	FDeprecationProperty& MakeProperty(
		FDeprecationProperty::Map& TargetMap, FPropertyTag& Tag)
	{
		FDeprecationProperty& DeprProperty = TargetMap.Add(Tag.Name);
		DeprProperty.PropertyName = Tag.Name;
		DeprProperty.PropertyTypeName = Tag.Type;
		DeprProperty.StructTypeName = Tag.StructName;
		DeprProperty.InnerTypeName = Tag.InnerType;
		DeprProperty.MapValueTypeName = Tag.ValueType;

		return DeprProperty;
	}

	//------------------------
	FDeprecationProperty::Variant& MakeVariant(FDeprecationProperty& Property, bool bIsKey)
	{
		return bIsKey ? Property.AddKey() : Property.AddValue();
	}
}

//------------------------
FDeprecationScope::FDeprecationScope(UObject* Object,
	FStructuredArchive::FRecord& Record, DeprecationHandler Handler, FString VersionPropertyName)
	: Object(Object)
	, Record(&Record)
	, Handler(Handler)
	, ObjectClass(nullptr)
	, VersionProperty(nullptr)
	, VersionPropertyName(VersionPropertyName)
	, PreSerializePosition(Record.GetUnderlyingArchive().Tell())
	, PostSerializePosition(0)
	, bIsLoading(Record.GetUnderlyingArchive().IsLoading())
	, bAssetHasDeprecationProperty(false)
	, CodeVersion(0)
{
	static constexpr char* DefaultVersionPropertyName = "DeprecationVersion";

	if (VersionPropertyName.IsEmpty())
	{
		VersionPropertyName = DefaultVersionPropertyName;
	}

	check(Object);
	check(this->Record);
	
	ObjectClass = Object->GetClass();
	VersionProperty = CastField<FUInt64Property>(ObjectClass->FindPropertyByName(*VersionPropertyName));
	ensureAlwaysMsgf(VersionProperty, TEXT("Version property with name '%s' not found."), *VersionPropertyName);

	uint64* CodeVersionPtr = VersionProperty->ContainerPtrToValuePtr<uint64>(ObjectClass->GetDefaultObject());
	CodeVersion = *CodeVersionPtr;
	ensureAlwaysMsgf(CodeVersion != (uint64)-1, TEXT("Version property with name '%s' can not have MAX value (reserved)."), *VersionPropertyName);

	if (!bIsLoading)
	{
		// If saving, we have to set the code version to invalid value,
		// so the system serializes the current value (which should be the default value).
		*CodeVersionPtr = (uint64)-1;
		return;
	}

	// Looking for the deprecation property in the asset (if present).
	FStructuredArchive::FSlot Slot = Record.EnterField(SA_FIELD_NAME(TEXT("Properties")));
	FStructuredArchive::FStream Stream = Slot.EnterStream();

	FName FNameVersionPropertyName(*VersionPropertyName);

	while (true)
	{
		FStructuredArchive::FRecord PropertyRecord = Stream.EnterElement().EnterRecord();

		FPropertyTag Tag;
		PropertyRecord << SA_VALUE(TEXT("Tag"), Tag);

		if (Tag.Name == NAME_None)
		{
			break;
		}
		if (!Tag.Name.IsValid())
		{
			UE_LOG(LogClass, Warning, TEXT("Invalid tag name: struct '%s', archive '%s'"), *Object->GetName(), *Record.GetUnderlyingArchive().GetArchiveName());
			break;
		}
		if (Tag.Name == FNameVersionPropertyName)
		{
			bAssetHasDeprecationProperty = true;
			break;
		}

		Record.GetUnderlyingArchive().Seek(Record.GetUnderlyingArchive().Tell()
			+ Tag.Size);
	}

	// We don't want to disturb the serializer :)
	Record.GetUnderlyingArchive().Seek(PreSerializePosition);
}

//------------------------
FDeprecationScope::~FDeprecationScope()
{
	if (!bIsLoading)
	{
		// Resetting the code version (for the same reason than in constructor).
		uint64* CodeVersionPtr = VersionProperty->ContainerPtrToValuePtr<uint64>(ObjectClass->GetDefaultObject());
		*CodeVersionPtr = CodeVersion;
		return;
	}

	PostSerializePosition = Record->GetUnderlyingArchive().Tell();

	uint64 AssetVersion;

	if (CheckDeprecation(AssetVersion))
	{
		Record->GetUnderlyingArchive().Seek(PreSerializePosition);

		FStructuredArchive::FSlot Slot = Record->EnterField(SA_FIELD_NAME(TEXT("Properties")));
		FStructuredArchive::FStream Stream = Slot.EnterStream();
		GenerateRoot(Root, Stream);

		if (Handler)
		{
			(Object->*Handler)(Root, AssetVersion, CodeVersion);
		}

		Record->GetUnderlyingArchive().Seek(PostSerializePosition);
	}
}

//------------------------
bool FDeprecationScope::CheckDeprecation(uint64& AssetVersion)
{
	uint64* AssetVersionPtr = VersionProperty->ContainerPtrToValuePtr<uint64>(Object);
	AssetVersion = *AssetVersionPtr;

	if (!bAssetHasDeprecationProperty)
	{
		AssetVersion = 0;
	}

	*AssetVersionPtr = CodeVersion;
	return CodeVersion > AssetVersion;
}

//------------------------
void FDeprecationScope::GenerateRoot(FDeprecationProperty::Map& TargetMap,
	FStructuredArchive::FStream& Stream)
{
	FLinkerLoad* Linker = (FLinkerLoad*)(Stream.GetUnderlyingArchive().GetLinker());
	while (true)
	{
		FStructuredArchive::FRecord PropertyRecord = Stream.EnterElement().EnterRecord();

		FPropertyTag Tag;
		PropertyRecord << SA_VALUE(TEXT("Tag"), Tag);

		if (Tag.Name == NAME_None)
		{
			break;
		}
		if (!Tag.Name.IsValid())
		{
			UE_LOG(LogClass, Warning, TEXT("Invalid tag name: struct '%s', archive '%s'"), *Object->GetName(), *Record->GetUnderlyingArchive().GetArchiveName());
			break;
		}

		FStructuredArchive::FStream ValueStream = PropertyRecord.EnterField(SA_FIELD_NAME(TEXT("Value"))).EnterStream();
		FDeprecationProperty& TargetProperty = MakeProperty(TargetMap, Tag);
		GenerateValue(Tag, Linker, TargetProperty, false, ValueStream);
	}
}

//------------------------
void FDeprecationScope::GenerateValue(FPropertyTag& Tag, FLinkerLoad* Linker,
	FDeprecationProperty& TargetProperty, bool bIsKey, FStructuredArchive::FStream& ValueStream)
{
	// Structures
	if (Tag.Type == NAME_StructProperty)
	{
		ValueStream.EnterElement().EnterRecord().EnterField(SA_FIELD_NAME(TEXT("Properties")));

		//------------------------
#define BUILTIN_STRUCT(TypeName) \
		if(Tag.StructName == NAME_##TypeName) { \
			F##TypeName Value; \
			ValueStream << Value; \
			FDeprecationProperty::Variant& Variant = MakeVariant(TargetProperty, bIsKey); \
			Variant.TypeName = Value; \
			return; \
		}

		// Commented lines below mean that there are no implicit converters from Slot.

		//BUILTIN_STRUCT(BoxSphereBounds);
		//BUILTIN_STRUCT(Sphere);
		BUILTIN_STRUCT(Box);
		BUILTIN_STRUCT(Vector2D);
		//BUILTIN_STRUCT(IntRect);
		BUILTIN_STRUCT(IntPoint);
		//BUILTIN_STRUCT(Vector4);
		BUILTIN_STRUCT(Vector);
		//BUILTIN_STRUCT(Rotator);
		BUILTIN_STRUCT(Color);
		BUILTIN_STRUCT(Plane);
		//BUILTIN_STRUCT(Matrix);
		BUILTIN_STRUCT(LinearColor);
		//BUILTIN_STRUCT(Quat);
		//BUILTIN_STRUCT(Transform);

#undef BUILTIN_STRUCT

		FDeprecationProperty::Variant& Variant = MakeVariant(TargetProperty, bIsKey);

		if (bIsKey)
		{
			TargetProperty.bHasKeyProperties = true;
		}
		else
		{
			TargetProperty.bHasValueProperties = true;
		}

		Variant.Properties = new FDeprecationProperty::Map();
		GenerateRoot(*Variant.Properties, ValueStream);

		return;
	}

	// Arrays
	else if (Tag.Type == NAME_ArrayProperty)
	{
		int32 Size;
		ValueStream << Size;

		FStructuredArchive::FStream ValuesStream = ValueStream.EnterElement().EnterRecord().EnterField(SA_FIELD_NAME(TEXT("Values"))).EnterStream();
		FPropertyTag ValuePropertyTag = Tag;
		ValuePropertyTag.Type = Tag.InnerType;
		
		for (int32 Index = 0; Index < Size; ++Index)
		{
			GenerateValue(ValuePropertyTag, Linker, TargetProperty, bIsKey, ValuesStream);
		}

		return;
	}

	else if (Tag.Type == NAME_SetProperty)
	{
		FStructuredArchive::FRecord SetRecord = ValueStream.EnterElement().EnterRecord();

		int32 NumElementsToRemove;
		FStructuredArchive::FArray KeysToRemoveArray = SetRecord.EnterArray(SA_FIELD_NAME(TEXT("ElementsToRemove")), NumElementsToRemove);

		int32 Size;
		FStructuredArchive::FArray ElementArray = SetRecord.EnterArray(SA_FIELD_NAME(TEXT("Elements")), Size);

		FPropertyTag ElementPropertyTag = Tag;
		ElementPropertyTag.Type = Tag.InnerType;

		for (int32 Index = 0; Index < Size; ++Index)
		{
			FStructuredArchive::FStream ElementStream = ElementArray.EnterElement().EnterStream();
			GenerateValue(ElementPropertyTag, Linker, TargetProperty, bIsKey, ElementStream);
		}

		return;
	}

	// Maps
	else if (Tag.Type == NAME_MapProperty)
	{
		FStructuredArchive::FRecord MapRecord = ValueStream.EnterElement().EnterRecord();

		int32 NumKeysToRemove;
		FStructuredArchive::FArray KeysToRemoveArray = MapRecord.EnterArray(SA_FIELD_NAME(TEXT("KeysToRemove")), NumKeysToRemove);

		int32 NumEntries;
		FStructuredArchive::FArray EntriesArray = MapRecord.EnterArray(SA_FIELD_NAME(TEXT("Entries")), NumEntries);

		FPropertyTag KeyPropertyTag = Tag;
		KeyPropertyTag.Type = Tag.InnerType;

		FPropertyTag ValuePropertyTag = Tag;
		ValuePropertyTag.Type = Tag.ValueType;

		for (int32 Index = 0; Index < NumEntries; ++Index)
		{
			FStructuredArchive::FRecord EntryRecord = EntriesArray.EnterElement().EnterRecord();

			FStructuredArchive::FStream EntryKeyStream = EntryRecord.EnterField(SA_FIELD_NAME(TEXT("Key"))).EnterStream();
			GenerateValue(KeyPropertyTag, Linker, TargetProperty, true, EntryKeyStream);

			FStructuredArchive::FStream EntryValueStream = EntryRecord.EnterField(SA_FIELD_NAME(TEXT("Value"))).EnterStream();
			GenerateValue(ValuePropertyTag, Linker, TargetProperty, false, EntryValueStream);
		}

		return;
	}

	FDeprecationProperty::Variant& Variant = MakeVariant(TargetProperty, bIsKey);

	// Objects
	if (Tag.Type == NAME_ObjectProperty)
	{
		FPackageIndex PackageIndex;
		ValueStream << PackageIndex;

		if (Linker)
		{
			if (PackageIndex.IsImport())
			{
				FObjectImport ObjImport = Linker->Imp(PackageIndex);
				Variant.ObjectImport = ObjImport;
			}
			else
			{
				FObjectExport ObjExport = Linker->Exp(PackageIndex);
				Variant.XObject = ObjExport.Object;
			}
		}
	}

	// Soft Objects
	else if (Tag.Type == NAME_SoftObjectProperty)
	{
		FSoftObjectPath PackagePath;
		ValueStream << PackagePath;

		Variant.Name = PackagePath.GetAssetPathName();
	}

	// Booleans
	else if (Tag.Type == NAME_BoolProperty)
	{
		Variant.bBool = Tag.BoolVal != 0;
	}

	// Strings
	else if (Tag.Type == NAME_StrProperty)
	{
		FString Value;
		ValueStream << Value;

		Variant.SetString(Value);
	}

	// Builtins
	else
	{
#define BUILTIN_TYPE(Name, CppType, VariantField) if(Tag.Type == Name){ CppType Value; ValueStream << Value; Variant.VariantField = Value; return; }

		BUILTIN_TYPE(NAME_Int8Property, int8, Int8);
		BUILTIN_TYPE(NAME_Int16Property, int16, Int16);
		BUILTIN_TYPE(NAME_IntProperty, int32, Int32);
		BUILTIN_TYPE(NAME_Int64Property, int64, Int64);

		BUILTIN_TYPE(NAME_ByteProperty, uint8, UInt8);
		BUILTIN_TYPE(NAME_UInt16Property, uint16, UInt16);
		BUILTIN_TYPE(NAME_UInt32Property, uint32, UInt32);
		BUILTIN_TYPE(NAME_UInt64Property, uint64, UInt64);

		BUILTIN_TYPE(NAME_FloatProperty, float, Float);
		BUILTIN_TYPE(NAME_DoubleProperty, double, Double);

		BUILTIN_TYPE(NAME_NameProperty, FName, Name);

#undef BUILTIN_TYPE
	}
}