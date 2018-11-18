
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <algorithm>

const Rgba Rgba::WHITE = Rgba(255, 255, 255);
const Rgba Rgba::BLUE = Rgba(0, 0, 255);
const Rgba Rgba::YELLOW = Rgba(255, 255, 0);
const Rgba Rgba::RED = Rgba(255, 0, 0);
const Rgba Rgba::GREEN = Rgba(0, 255, 0);
const Rgba Rgba::BLACK = Rgba(0, 0, 0);
const Rgba Rgba::GREY = Rgba(120, 120, 120);
const Rgba Rgba::CYAN = Rgba(0, 255, 255);
const Rgba Rgba::MEGENTA = Rgba(255, 0, 255);
const Rgba Rgba::DARK_BROWN = Rgba(153, 76, 0);
const Rgba Rgba::TEAL = Rgba(0, 128, 128);
const Rgba Rgba::PINK = Rgba(255, 192, 203);
const Rgba Rgba::MAROON = Rgba(128, 0, 0);
const Rgba Rgba::OLIVE = Rgba(128, 128, 0);
const Rgba Rgba::NAVY = Rgba(0, 0, 128);
const Rgba Rgba::LIME = Rgba(0, 128, 0);
const Rgba Rgba::PURPLE = Rgba(128, 0, 128);
const Rgba Rgba::LIGHT_BLUE = Rgba(102, 178, 255);
const Rgba Rgba::BLACK_HALF_OPACITY = Rgba(0, 0, 0, 127);
const Rgba Rgba::WHITE_HALF_OPACITY = Rgba(255, 255, 255, 127);
const Rgba Rgba::BLUE_HALF_OPACITY = Rgba(0, 0, 255, 127);
const Rgba Rgba::YELLOW_HALF_OPACITY = Rgba(255, 255, 0, 127);
const Rgba Rgba::RED_HALF_OPACITY = Rgba(255, 0, 0, 127);
const Rgba Rgba::CYAN_HALF_OPACITY = Rgba(0, 255, 255, 127);
const Rgba Rgba::MEGENTA_HALF_OPACITY = Rgba(255, 0, 255, 127);
Rgba color_list[COLOR_LIST_SIZE] = {Rgba::RED, Rgba::GREEN, Rgba::BLUE, Rgba::YELLOW,
	Rgba::GREY, Rgba::CYAN, Rgba::MEGENTA, Rgba::DARK_BROWN, Rgba::TEAL, Rgba::PINK,
	Rgba::MAROON, Rgba::OLIVE, Rgba::NAVY, Rgba::LIME, Rgba::PURPLE};
int color_index = 0;

Rgba::Rgba()
{
	r = 255;
	g = 255;
	b = 255;
	a = 255;
}

Rgba::~Rgba()
{

}


Rgba::Rgba( unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte)
	: r( redByte ), g( greenByte ), b( blueByte ), a ( alphaByte )
{

}


void Rgba::SetAsBytes(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte)
{
	r = redByte;
	g = greenByte;
	b = blueByte;
	a = alphaByte;
}


void Rgba::SetAsFloats(float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha)
{
	r = static_cast<unsigned char>(normalizedRed * 255.f);
	g = static_cast<unsigned char>(normalizedGreen * 255.f);
	b = static_cast<unsigned char>(normalizedBlue * 255.f);
	a = static_cast<unsigned char>(normalizedAlpha * 255.f);
}


void Rgba::SetAlphaAsFloat(float normalizedAlpha)
{
	a = static_cast<unsigned char>(normalizedAlpha * 255.f);
}


void Rgba::GetAsFloats(float& out_normalizedRed, float& out_normalizedGreen, float& out_normalizedBlue, float& out_normalizedAlpha) const
{
	out_normalizedRed = static_cast<float>(r) / 255.f;
	out_normalizedGreen = static_cast<float>(g) / 255.f;
	out_normalizedBlue = static_cast<float>(b) / 255.f;
	out_normalizedAlpha = static_cast<float>(a) / 255.f;
}


void Rgba::ScaleRGB(float rgbScale)
{
	float scaledR = static_cast<float>(r) * rgbScale;
	float scaledG = static_cast<float>(g) * rgbScale;
	float scaledB = static_cast<float>(b) * rgbScale;

	float clampedR = ClampZeroToFloat(255.f, scaledR);
	float clampedG = ClampZeroToFloat(255.f, scaledG);
	float clampedB = ClampZeroToFloat(255.f, scaledB);

	r = static_cast<unsigned char>(clampedR);
	g = static_cast<unsigned char>(clampedG);
	b = static_cast<unsigned char>(clampedB);
}


void Rgba::ScaleAlpha(float alphaScale)
{
	float scaledA = static_cast<float>(a) * alphaScale;
	float clampedA = ClampZeroToFloat(255.f, scaledA);
	a = static_cast<unsigned char>(clampedA);
}


void Rgba::SetFromText( const char* text )
{
	const char* comma1 = strchr(text, ',');
	const char* comma2 = strchr(comma1 + 1, ',');
	const char* comma3 = strchr(comma2 + 1, ',');

	r = static_cast<unsigned char>(atoi(text));
	g = static_cast<unsigned char>(atoi(comma1 + 1));
	b = static_cast<unsigned char>(atoi(comma2 + 1));

	if (comma3 != nullptr)
	{
		a = static_cast<unsigned char>(atoi(comma3 + 1));
	}
}


bool Rgba::operator==( const Rgba& compare ) const
{
	if ((r == compare.r) && (g == compare.g) && (b == compare.b) && (a == compare.a))
	{
		return true;
	}

	return false;
}


const Rgba GetRandomColor()
{
	unsigned char r = (unsigned char)GetRandomIntInRange(0, 255);
	unsigned char g = (unsigned char)GetRandomIntInRange(0, 255);
	unsigned char b = (unsigned char)GetRandomIntInRange(0, 255);
	return Rgba(r, g, b, 255);
}

const Rgba Interpolate( const Rgba& start, const Rgba& end, float fractionTowardEnd )
{
	//unsigned char r = Interpolate(start.r, end.r, fractionTowardEnd);
	//unsigned char g = Interpolate(start.g, end.g, fractionTowardEnd);
	//unsigned char b = Interpolate(start.b, end.b, fractionTowardEnd);
	//unsigned char a = Interpolate(start.a, end.a, fractionTowardEnd);

	//return Rgba(r, g, b, a);

	Vector4 startVec4;
	start.GetAsFloats(startVec4.x, startVec4.y, startVec4.z, startVec4.w);

	Vector4 endVec4;
	end.GetAsFloats(endVec4.x, endVec4.y, endVec4.z, endVec4.w);

	float w = Interpolate(startVec4.w, endVec4.w, fractionTowardEnd);

	Vector3 startHSL = RGBToHSL(Vector3(startVec4.x, startVec4.y, startVec4.z));
	Vector3 endHSL = RGBToHSL(Vector3(endVec4.x, endVec4.y, endVec4.z));

	float hue = Interpolate(startHSL.x, endHSL.x, fractionTowardEnd);
	float sat = Interpolate(startHSL.y, endHSL.y, fractionTowardEnd);
	float lum = Interpolate(startHSL.z, endHSL.z, fractionTowardEnd);

	Vector3 hsl = Vector3(hue, sat, lum);

	Vector3 rgb = HSLToRGB(hsl);

	Vector4 rgbaVec4 = Vector4(rgb.x, rgb.y, rgb.z, w);
	Rgba rgba;
	rgba.SetAsFloats(rgbaVec4.x, rgbaVec4.y, rgbaVec4.z, rgbaVec4.w);

	return rgba;
}


Vector3 RGBToHSL(Vector3 rgb)
{
	float mx = max( rgb.x, max( rgb.y, rgb.z ));
	float mn = min( rgb.x, min( rgb.y, rgb.z ));

	float lum = (mx + mn) / 2.0f;
	float hue = 0.0f;
	float sat = 0.0f;

	if (mx != mn) {
		float dist = mx - mn;
		if (lum > 0.5f) 
			sat = dist / (2.0f - mx - mn); 
		else 
			sat = dist / (mx + mn);

		if (mx == rgb.x) 
			hue = ((rgb.y - rgb.z) / dist) + ((rgb.y < rgb.z) ? 6.0f : 0.0f);
		else if (mx == rgb.y)
			hue = ((rgb.z - rgb.x) / dist) + 2.0f;
		else
			hue = ((rgb.x - rgb.y) / dist) + 4.0f;
		hue /= 6.0f;
	}

	return Vector3( hue, sat, lum ); 
}


Vector3 HSLToRGB(Vector3 hsl)
{
	float h = hsl.x;
	float s = hsl.y;
	float l = hsl.z;

	Vector3 ret; 
	float c = s * (1.0f - abs( 2.0f * l - 1.0f ));
	float m = l - .5f * c;
	float x = c * (1.0f - abs(fmod( h * 6.0f, 2.0f ) - 1.0f));

	if (h < (1.0f / 6.0f))
		ret = Vector3( c, x, 0.0f );
	else if (h < (2.0f / 6.0f)) 
		ret = Vector3( x, c, 0.0f );
	else if (h < (3.0f / 6.0f)) 
		ret = Vector3( 0.0f, c, x );
	else if (h < (4.0f / 6.0f)) 
		ret = Vector3( 0.0f, x, c );
	else if (h < (5.0f / 6.0f)) 
		ret = Vector3( x, 0.0f, c );
	else 
		ret = Vector3( c, 0.0f, x );
	

	ret = ret + Vector3( m, m, m );
	return ret;
}