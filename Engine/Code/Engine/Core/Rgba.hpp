#pragma once

#include "Engine/Math/Vector3.hpp"

#define COLOR_LIST_SIZE 15

class Rgba 
{
public:
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;

	// Construction/Destruction
	~Rgba();									
	Rgba();											
	explicit Rgba( unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte = 255);	

	void SetAsBytes( unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte = 255);
	void SetAsFloats(float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha = 1.0f);
	void SetAlphaAsFloat(float normalizedAlpha);

	void GetAsFloats(float& out_normalizedRed, float& out_normalizedGreen, float& out_normalizedBlue, float& out_normalizedAlpha) const;

	void ScaleRGB(float rgbScale);
	void ScaleAlpha(float alphaScale);

	void SetFromText( const char* text );

	bool operator==( const Rgba& compare ) const;

	static const Rgba WHITE;
	static const Rgba BLUE;
	static const Rgba YELLOW;
	static const Rgba RED;
	static const Rgba BLACK;
	static const Rgba GREEN;
	static const Rgba CYAN;
	static const Rgba GREY;
	static const Rgba MEGENTA;
	static const Rgba DARK_BROWN;
	static const Rgba TEAL;
	static const Rgba PINK;
	static const Rgba MAROON;
	static const Rgba OLIVE;
	static const Rgba NAVY;
	static const Rgba LIME;
	static const Rgba PURPLE;
	static const Rgba BLACK_HALF_OPACITY;
	static const Rgba WHITE_HALF_OPACITY;
	static const Rgba BLUE_HALF_OPACITY;
	static const Rgba YELLOW_HALF_OPACITY;
	static const Rgba RED_HALF_OPACITY;
	static const Rgba CYAN_HALF_OPACITY;
	static const Rgba MEGENTA_HALF_OPACITY;
};

const Rgba GetRandomColor();
const Rgba Interpolate( const Rgba& start, const Rgba& end, float fractionTowardEnd );
Vector3 RGBToHSL(Vector3 rgb);
Vector3 HSLToRGB(Vector3 hsl);

extern Rgba color_list[COLOR_LIST_SIZE];
extern int color_index;