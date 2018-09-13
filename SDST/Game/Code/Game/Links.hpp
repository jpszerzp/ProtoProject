#pragma once

#include "Engine/Physics/3D/Entity3.hpp"
#include "Engine/Physics/3D/CollisionDetection.hpp"
#include "Engine/Core/Primitive/Point.hpp"

class Link 
{
public:
	Point* m_p1;
	Point* m_p2;

protected:
	float GetCurrentLength() const;

public:
	virtual uint FillContact(std::vector<Contact3>& contacts,
		uint limit) const = 0;
};

class Cable : public Link
{
public:
	float m_maxLength;
	float m_cableRestitution;

public:
	virtual uint FillContact(std::vector<Contact3>& contacts,
		uint limit) const override;

};

class Rod : public Link
{
public:
	float m_length;

public:
	Rod(float length, Point* p1, Point* p2);

	virtual uint FillContact(std::vector<Contact3>& contacts,
		uint limit) const override;
};