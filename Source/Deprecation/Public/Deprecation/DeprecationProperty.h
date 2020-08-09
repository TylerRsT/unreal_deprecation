

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectResource.h"

//------------------------
class DEPRECATION_API FDeprecationProperty final
{
	// Friends
private:
	friend class FDeprecationScope;



	// Typedefs
public:
	typedef TMap<FName, FDeprecationProperty> Map;

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
		{
		}

		Variant& operator=(const Variant& Other)
		{
			bBool = Other.bBool;
			Int8 = Other.Int8;
			Int16 = Other.Int16;
			Int32 = Other.Int32;
			Int64 = Other.Int64;
			UInt8 = Other.UInt8;
			UInt16 = Other.UInt16;
			UInt32 = Other.UInt32;
			UInt64 = Other.UInt64;
			Float = Other.Float;
			Double = Other.Double;
			Name = Other.Name;
			Box = Other.Box;
			Box2D = Other.Box2D;
			Vector2D = Other.Vector2D;
			IntRect = Other.IntRect;
			IntPoint = Other.IntPoint;
			Vector4 = Other.Vector4;
			Vector = Other.Vector;
			Rotator = Other.Rotator;
			Color = Other.Color;
			Plane = Other.Plane;
			Matrix = Other.Matrix;
			LinearColor = Other.LinearColor;
			Quat = Other.Quat;
			Transform = Other.Transform;
			ObjectImport = Other.ObjectImport;
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
	} Variant;




	
	// Constructors
public:
	FDeprecationProperty()
		: KeyProperties(nullptr)
		, ValueProperties(nullptr)
	{ }




	// Destructors
public:
	~FDeprecationProperty()
	{
		if (KeyProperties)
		{
			delete KeyProperties;
			KeyProperties = nullptr;
		}

		if (ValueProperties)
		{
			delete ValueProperties;
			ValueProperties = nullptr;
		}
	}




	// Methods
public:
	inline const Variant& GetKey(int32 Index) const
	{
		return Keys[Index];
	}

	inline const Variant& GetKey() const
	{
		return GetKey(0);
	}

	inline Variant& AddKey()
	{
		return Keys.AddDefaulted_GetRef();
	}

	inline const Variant& GetValue(int32 Index) const
	{
		return Values[Index];
	}

	inline const Variant& GetValue() const
	{
		return GetValue(0);
	}

	inline Variant& AddValue()
	{
		return Values.AddDefaulted_GetRef();
	}

	inline bool HasValue() const { return Values.Num() > 0; }

	inline const TArray<Variant> GetKeys() const
	{
		return Keys;
	}

	inline const TArray<Variant> GetValues() const
	{
		return Values;
	}

	inline Map const* const GetKeyProperties() const
	{
		return KeyProperties;
	}

	inline Map const* const GetValueProperties() const
	{
		return ValueProperties;
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

private:
	Map* KeyProperties;
	Map* ValueProperties;
};