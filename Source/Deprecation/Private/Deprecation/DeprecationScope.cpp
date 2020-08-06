

#include "DeprecationScope.h"

#include "UObject/LinkerLoad.h"
#include "UObject/NoExportTypes.h"
#include "UObject/UnrealType.h"

//------------------------
FDeprecationScope::FDeprecationScope(UObject* Object, FStructuredArchive::FRecord& Record)
	: Object(Object)
	, Record(&Record)
	, PreSerializePosition(Record.GetUnderlyingArchive().Tell())
	, PostSerializePosition(0)
	, bIsLoading(Record.GetUnderlyingArchive().IsLoading())
{ }

//------------------------
FDeprecationScope::~FDeprecationScope()
{
	if (!bIsLoading)
	{
		return;
	}

	PostSerializePosition = Record->GetUnderlyingArchive().Tell();

	uint64 AssetVersion;
	uint64 CodeVersion;

	if (CheckDeprecation(AssetVersion, CodeVersion))
	{
		Record->GetUnderlyingArchive().Seek(PreSerializePosition);

		FStructuredArchive::FSlot Slot = Record->EnterField(FIELD_NAME_TEXT("Properties"));
		FStructuredArchive::FStream Stream = Slot.EnterStream();
		GenerateRoot(Root, Stream);

		Record->GetUnderlyingArchive().Seek(PostSerializePosition);
	}
}

//------------------------
bool FDeprecationScope::CheckDeprecation(uint64& AssetVersion, uint64& CodeVersion)
{
	static constexpr char* DefaultVersionPropertyName = "DeprecationVersion";

	const TCHAR* DeprPropertyKey = TEXT("DeprecationProperty");
	const TCHAR* DeprVersionKey = TEXT("DeprecationVersion");

	UClass* MyClass = Object->GetClass();
	FString VersionPropertyName = MyClass->GetMetaData(DeprPropertyKey);

	if (VersionPropertyName.IsEmpty())
	{
		VersionPropertyName = DefaultVersionPropertyName;
	}

	UUInt64Property* VersionProperty =
		Cast<UUInt64Property>(MyClass->FindPropertyByName(*VersionPropertyName));
	ensureAlwaysMsgf(VersionProperty, TEXT("Version property with name '%s' not found."), *VersionPropertyName);

	AssetVersion = *(VersionProperty->ContainerPtrToValuePtr<uint64>(this));
	FString DeprVersionAsStr = MyClass->GetMetaData(DeprVersionKey);

	if (!DeprVersionAsStr.IsEmpty())
	{
		CodeVersion = FCString::Atoi(*DeprVersionAsStr);
	}

	return CodeVersion > AssetVersion;
}

//------------------------
void FDeprecationScope::GenerateRoot(FDeprecationProperty::Map& TargetMap,
	FStructuredArchive::FStream& Stream)
{
	while (true)
	{
		FStructuredArchive::FRecord PropertyRecord = Stream.EnterElement().EnterRecord();

		FPropertyTag Tag;
		PropertyRecord << NAMED_FIELD(Tag);

		if (Tag.Name == NAME_None)
		{
			break;
		}
		if (!Tag.Name.IsValid())
		{
			UE_LOG(LogClass, Warning, TEXT("Invalid tag name: struct '%s', archive '%s'"), *Object->GetName(), *Record->GetUnderlyingArchive().GetArchiveName());
			break;
		}

		FStructuredArchive::FSlot ValueSlot = PropertyRecord.EnterField(FIELD_NAME_TEXT("Value"));

		// Structures
		if (Tag.Type == NAME_StructProperty)
		{
#define BUILTIN_STRUCT(Name, VariantInner) \
		if(Tag.InnerType == Name) { \
			FStructuredArchive::FSlot ValueSlot = PropertyRecord.EnterField(FIELD_NAME_TEXT("Value")); \
			VariantInner Value; \
			ValueSlot << Value; \
		}

		}
	}
}