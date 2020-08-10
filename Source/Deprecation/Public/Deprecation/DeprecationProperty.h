

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectResource.h"

/**
 * Property describing the data retrieved directly from the asset file.
 */
class DEPRECATION_API FDeprecationProperty final
{
	// Friends
private:
	friend class FDeprecationScope;



	// Typedefs
public:
	/**
	 * Property map, with the key being the name of the property.
	 */
	typedef TMap<FName, FDeprecationProperty> Map;

	/**
	 * Union representing all type of data that can be held by a property.
	 */
	typedef union Variant
	{
		Variant()
			: Name()
			, Box()
			, Box2D()
			, Vector2D()
			, IntRect()
			, IntPoint()
			, Vector4()
			, Vector()
			, Rotator()
			, Color()
			, Plane()
			, Matrix()
			, LinearColor()
			, Quat()
			, Transform()
			, ObjectImport()
		{ 
			FMemory::Memset(this, 0, sizeof(Variant));
		}

		Variant(const Variant& Other)
			: bBool(Other.bBool)
			, Int8(Other.Int8)
			, Int16(Other.Int16)
			, Int32(Other.Int32)
			, Int64(Other.Int64)
			, UInt8(Other.UInt8)
			, UInt16(Other.UInt16)
			, UInt32(Other.UInt32)
			, UInt64(Other.UInt64)
			, Float(Other.Float)
			, Double(Other.Double)
			, Name(Other.Name)
			, Box(Other.Box)
			, Box2D(Other.Box2D)
			, Vector2D(Other.Vector2D)
			, IntRect(Other.IntRect)
			, IntPoint(Other.IntPoint)
			, Vector4(Other.Vector4)
			, Vector(Other.Vector)
			, Rotator(Other.Rotator)
			, Color(Other.Color)
			, Plane(Other.Plane)
			, Matrix(Other.Matrix)
			, LinearColor(Other.LinearColor)
			, Quat(Other.Quat)
			, Transform(Other.Transform)
			, ObjectImport(Other.ObjectImport)
			, XObject(Other.XObject)
			, Properties(Other.Properties)
		{
			FMemory::Memcpy(*this, Other);
		}

		Variant& operator=(const Variant& Other)
		{
			FMemory::Memcpy(*this, Other);
		}

		inline FString GetString() const
		{
			return Name.GetPlainNameString();
		}

		inline void SetString(FString Value)
		{
			Name = FName(*Value);
		}

		bool				bBool;

		int8				Int8;
		int16				Int16;
		int32				Int32;
		int64				Int64;
		uint8				UInt8;
		uint16				UInt16;
		uint32				UInt32;
		uint64				UInt64;

		float				Float;
		double				Double;

		FName				Name;

		FBox				Box;
		FBox2D				Box2D;
		FVector2D			Vector2D;
		FIntRect			IntRect;
		FIntPoint			IntPoint;
		FVector4			Vector4;
		FVector				Vector;
		FRotator			Rotator;
		FColor				Color;
		FPlane				Plane;
		FMatrix				Matrix;
		FLinearColor		LinearColor;
		FQuat				Quat;
		FTransform			Transform;

		FObjectImport		ObjectImport;
		UObject*			XObject;
		Map*				Properties;
	} Variant;




	// Constructors
public:
	FDeprecationProperty()
		: bHasKeyProperties(false)
		, bHasValueProperties(false)
	{ }




	// Destructor
public:
	~FDeprecationProperty()
	{
		if (bHasKeyProperties)
		{
			for (Variant& Variant : Keys)
			{
				delete Variant.Properties;
				Variant.Properties = nullptr;
			}
			bHasKeyProperties = false;
		}

		if (bHasValueProperties)
		{
			for (Variant& Variant : Values)
			{
				delete Variant.Properties;
				Variant.Properties = nullptr;
			}
			bHasKeyProperties = false;
		}
	}




	
	// Methods
public:
	/**
	 * Retrieves a key with the given index.
	 * @param Index Index in the array of keys to retrieve.
	 * @returns A variant holding key data.
	 */
	inline const Variant& GetKey(int32 Index) const
	{
		return Keys[Index];
	}

	/**
	 * Retrieves the first key in the array of keys.
	 * @returns A variant holding key data.
	 */
	inline const Variant& GetKey() const
	{
		return GetKey(0);
	}

	/**
	 * Adds a key in the array of property keys.
	 * @returns Newly created variant holding key data.
	 */
	inline Variant& AddKey()
	{
		return Keys.AddDefaulted_GetRef();
	}

	/**
	 * Retrieves a value with the given index.
	 * @param Index Index in the array of values to retrieve.
	 * @returns A variant holding value data.
	 */
	inline const Variant& GetValue(int32 Index) const
	{
		return Values[Index];
	}

	/**
	 * Retrieves the first key in the array of keys.
	 * @returns A variant holding key data.
	 */
	inline const Variant& GetValue() const
	{
		return GetValue(0);
	}

	/**
	 * Adds a value in the array of property values.
	 * @returns Newly created variant holding value data.
	 */
	inline Variant& AddValue()
	{
		return Values.AddDefaulted_GetRef();
	}

	/**
	 * Returns whether or not there is at least one value.
	 */
	inline bool HasValue() const { return Values.Num() > 0; }

	/**
	 * Returns all the keys associated with this property (for a map property).
	 */
	inline const TArray<Variant> GetKeys() const
	{
		return Keys;
	}
	
	/**
	 * Returns all the values associated with this property. (for an array or a map property).
	 */
	inline const TArray<Variant> GetValues() const
	{
		return Values;
	}




	// Fields
public:
	FName PropertyName;
	FName PropertyTypeName;

	FName StructTypeName;
	FName InnerTypeName;
	FName MapValueTypeName; // For Map properties

	TArray<Variant> Keys;
	TArray<Variant> Values;

	bool bHasKeyProperties;
	bool bHasValueProperties;
};