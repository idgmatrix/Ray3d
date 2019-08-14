#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "vector.h"
#include "primitive.h"


class RayTracer {
	int ScreenX, ScreenY, ScreenZ;
	Vector3 vCenter;
	float R;
	Vector3 vLightPosition;
	//int Offset;
	float theta;
	int current_index[4];

public:
	void Setup();
	void Move(void);
	void Render(int scan);
	float RayTrace(const Ray& ray, Vector3* pInterPos, Vector3* pNormal);
	void WaitEndThread(void);
	bool Thread_Flag;

private:	
	Vector3 MakeRay(int, int, int);
	Vector3 MakeNormal(const Vector3 vPoint, const Vector3 vCenter);
	bool CheckIntersect(Vector3 vStart, Vector3 vRay, Vector3 vCenter, float R);
	float FindIntersectPoint(Vector3 ray, Vector3 center, float R, Vector3* );
	void DrawPixel();

};
#endif