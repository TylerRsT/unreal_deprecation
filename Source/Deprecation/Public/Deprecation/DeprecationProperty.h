

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectResource.h"

//------------------------
class DEPRECATION_API FDeprecationProperty final
{
	// Typedefs
public:
	typedef TMap<FName, FDeprecationProperty> Map;

	typedef union Variant
	{
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
			, BoxSphereBounds(Other.BoxSphereBounds)
			, Sphere(Other.Sphere)
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
			, Map(Other.Map)
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
			BoxSphereBounds = Other.BoxSphereBounds;
			Sphere = Other.Sphere;
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
			Map = Other.Map;
		}

		inline FString GetString() const
		{
			return Name.GetPlainNameString();
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

		FBoxSphereBounds	BoxSphereBounds;
		FSphere				Sphere;
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

		Map*				Map;
	} Variant;

private:
	typedef struct VariantPair
	{
		Variant Key;
		Variant Value;
	} VariantPair;




	// Methods
public:
	inline const Variant& GetKey(int32 Index) const
	{
		return Variants[Index].Key;
	}

	inline const Variant& GetKey() const
	{
		return GetKey(0);
	}

	inline const Variant& GetValue(int32 Index) const
	{
		return Variants[Index].Value;
	}

	inline const Variant& GetValue() const
	{
		return GetValue(0);
	}

	inline int32 Num() const { return Variants.Num(); }

	inline bool HasValue() const { return Num() == 0; }

	inline void GetKeys(TArray<Variant>& OutVariants) const
	{
		OutVariants.Reserve(Variants.Num());
		for (const VariantPair& Pair : Variants)
		{
			OutVariants.Add(Pair.Key);
		}
	}

	inline void GetValues(TArray<Variant>& OutVariants) const
	{
		OutVariants.Reserve(Variants.Num());
		for (const VariantPair& Pair : Variants)
		{
			OutVariants.Add(Pair.Value);
		}
	}



	// Fields
public:
	FName PropertyName;
	FName PropertyTypeName;

	FName InnerTypeName;
	FName MapValueTypeName; // For Map properties

private:
	TArray<VariantPair> Variants;
};