#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "vector.h"
#include "material.h"
#define EPSILON 0.0001f

class Ray {
public:
	Vector3	startp;
	Vector3 dir;

	void SetFromPoints(const Vector3& s, const Vector3& e)  
	{
		startp = s;
		dir = e - s;
		dir.Normalize();
	}

	Ray(){}
	Ray(const Vector3& s, const Vector3& d)
	{	
		startp = s; dir = d;
	}

};

class Geometry {
public:
	virtual bool CheckIntersection(const Ray& ray) const = 0;  
	virtual float FindIntersection(const Ray& ray) const = 0;  
	virtual float FindInnerIntersection(const Ray& ray) const = 0;  
	virtual Vector3 GetNormalAtPos(const Vector3& pos) const = 0;
	
	Vector3 GetIntersectionPos(float t, const Ray& ray) const
	{
		return Vector3(ray.dir * t + ray.startp);
	}
};

class Plane : public Geometry {
public:
	Vector3 normal;
	float D;

	Plane(){}
	Plane(const Vector3& n, float d)
	{
		normal = n;
		D = d;
	}

	bool CheckIntersection(const Ray& ray) const
	{
		if ( (ray.dir * normal) >= 0.0f )
			return false;
		else
			return true;
	}
	float FindIntersection(const Ray& ray) const
	{
		float t;
		float det;

		det = ray.dir * normal;
		
		if ( det < EPSILON && det > -EPSILON)
			return -999999.0f;
		else
		{
			t = (D - normal * ray.startp) / det;
			return t;
		}
	}
	float FindInnerIntersection(const Ray& ray) const
	{
		float t;
		float det;

		det = ray.dir * normal;
		
		if ( det < EPSILON && det > -EPSILON)
			return -999999.0f;
		else
		{
			t = (D - normal * ray.startp) / det;
			return t;
		}
	}
	Vector3 GetNormalAtPos(const Vector3& pos) const
	{
		return normal;
	}
};

class Sphere : public Geometry {
public:
	Vector3 center;
	float R;

	Sphere(){}
	Sphere(const Vector3& c, float r)
	{
		center = c;
		R = r;
	}
	
	bool CheckIntersection(const Ray& ray) const
	{
		// Diameter btw Sphere & Ray
		Vector3 vSphere = center - ray.startp;
		float S = ray.dir * vSphere;
		if ( S < 0.0f ) return false;
		Vector3 vLight = S * ray.dir;
		Vector3 vDiameter = vLight - vSphere;
		float Diameter = vDiameter.Length();

		if ( Diameter >= R ) return false;

		return true;
	}
	
	float FindIntersection(const Ray& ray) const
	{
		float A, B, C, D, tp, tm;

		// Diameter btw Sphere & Ray
		float S = ray.dir * (center - ray.startp);
		if ( S < 0.0f ) return -999.0f;
		Vector3 vDiameter = ray.dir * S - (center - ray.startp);
		float Diameter = vDiameter.Length();

		if ( Diameter >= R ) return -999.0f;
	
		// Intersection
		A = ray.dir * ray.dir;
		B = 2 * (ray.startp - center) * ray.dir;
		C = (ray.startp - center) * (ray.startp - center) - R * R;
		D = B * B - 4 * A * C;
		float sqrtD = sqrtf(D);
		float recA2 = 1 / (2 * A);

		tp = (-B + sqrtD) * recA2;
		tm = (-B - sqrtD) * recA2;

		if (tp < 0.0f || tm < 0.0f) return -999.0f;
		if (tp > tm){
			return tm;
		}
		else{
			return tp;
		}
	}
	
	float FindInnerIntersection(const Ray& ray) const
	{
		float A, B, C, D, tp, tm;

		// Diameter btw Sphere & Ray
		float S = ray.dir * (center - ray.startp);
		if ( S < 0.0f ) return -999.0f;
		Vector3 vDiameter = ray.dir * S - (center - ray.startp);
		float Diameter = vDiameter.Length();

		if ( Diameter >= R ) return -999.0f;
	
		// Intersection
		A = ray.dir * ray.dir;
		B = 2 * (ray.startp - center) * ray.dir;
		C = (ray.startp - center) * (ray.startp - center) - R * R;
		D = B * B - 4 * A * C;
		float sqrtD = sqrtf(D);
		float recA2 = 1 / (2 * A);

		tp = (-B + sqrtD) * recA2;
		tm = (-B - sqrtD) * recA2;

		if (tp < 0.0f && tm < 0.0f) return -999.0f;
		if (tp > tm){
			return tp;
		}
		else{
			return tm;
		}
	}
	
	Vector3 GetNormalAtPos(const Vector3& pos) const
	{
		Vector3 n;

		n = pos - center;
		n.Normalize();

		return n;
	}

};

#endif