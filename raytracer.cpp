#include "stdafx.h"
#include "vertex.h"
#include "raytracer.h"
#include "graphics.h"
#include "util.h"
//#include "sphere.h"
#include "primitive.h"
#include "scene.h"

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

#define MAX_GEOM 4
#define SCENE_CENTER 350
Geometry *pGeom[MAX_GEOM];
Sphere sph[3];
Plane pln[2];


void RayTracer::Setup(void)
{
	sph[0].center = Vector3(0, 0, SCENE_CENTER);
	sph[0].R = 100.0f;

	sph[1].center = Vector3(100, 0, SCENE_CENTER/2);
	sph[1].R = 30.0f;

	sph[2].center = Vector3(250, 150, SCENE_CENTER);
	sph[2].R = 30.0f;

	pln[0].D = -100.0f;
	pln[0].normal = Vector3(0.0f, 1.0f, 0.0f);

	pln[1].D = -500.0f;
	pln[1].normal = Vector3(0.0f, -1.0f, 0.0f);

	pGeom[0] = &sph[0];
	pGeom[1] = &sph[1];
	pGeom[2] = &sph[2];
	pGeom[3] = &pln[0];
	//pGeom[4] = &pln[1];

	
	vLightPosition = Vector3(100, 200, 0);
}

Vector3 RayTracer::MakeRay(int x, int y, int z)
{
	Vector3 vS((float)x, (float)y, (float)z);
	
	vS.Normalize();

	return vS;
}

bool RayTracer::CheckIntersect(Vector3 vStart, Vector3 vRay, Vector3 vCenter, float R)
{

	// Diameter btw Sphere & Ray
	Vector3 vSphere = vCenter - vStart;
	float S = vRay * vSphere;
	if ( S < 0.0f ) return false;
	Vector3 vLight = S * vRay;
	Vector3 vDiameter = vLight - vSphere;
	float Diameter = vDiameter.Length();

	if ( Diameter >= R ) return false;

	return true;
	
}

float RayTracer::FindIntersectPoint(Vector3 vRay, Vector3 vCenter, float R, Vector3 *out)
{
	float A, B, C, D, tp, tm;

	// Diameter btw Sphere & Ray
	float S = vRay * vCenter;
	Vector3 vDiameter = vRay * S - vCenter;
	float Diameter = vDiameter.Length();

	if ( Diameter >= R ) return -1.0f;
	
	// Intersection
	A = vRay * vRay;
	B = -2 * vRay * vCenter;
	C = vCenter * vCenter - R * R;
	D = B * B - 4 * A * C;
	float sqrtD = sqrtf(D);
	float recA2 = 1 / (2 * A);

	tp = (-B + sqrtD) * recA2;
	tm = (-B - sqrtD) * recA2;

	if (tp < 0.0f || tm < 0.0f) return -1.0f;
	if (tp > tm){
		*out = tm * vRay;
		return tm;
	}
	else{
		*out = tp * vRay;
		return tp;
	}
}

Vector3 RayTracer::MakeNormal(const Vector3 vPoint, const Vector3 vCenter)
{
	Vector3 vNormal;

	vNormal = vPoint - vCenter;
	vNormal.Normalize();

	return vNormal;
}

// Just Simple Animations
void RayTracer::Move(void)
{
	//static float theta = 0;
	
	//vLightPosition = Vector3(200 * cosf(theta), 300, 0.0f);
	vLightPosition = Vector3(350 * cosf(3*theta), 250, 350 * sinf(3*theta) + SCENE_CENTER);
	//sph[0].center = Vector3(0, 0, SCENE_CENTER);
	sph[0].center += Vector3(0, 2 * cosf(2*theta), 0);
	sph[1].center = Vector3(200 * cosf(theta), 10, 200 * sinf(theta) + SCENE_CENTER);
	sph[2].center = Vector3(150 * cosf(-theta), 50, 150 * sinf(-theta) + SCENE_CENTER);
	//pln[0].D = -100 + 100 * cosf(theta);

	theta += 0.07f;
}

void RayTracer::WaitEndThread(void)
{
	while(Thread_Flag);
}

float RayTracer::RayTrace(const Ray& ray, Vector3* pInterPos, Vector3* pNormal)
{
	//int index;
	float Intensity = 0.0f;
	float t[MAX_GEOM];
	int current_index;
	
	Ray eray = ray;
	eray.startp += ray.dir * EPSILON * 2;
	float temp = 999999.0f;
	int t_index = -1;
	for (int i = 0; i < MAX_GEOM; i++)
	{
		t[i] = pGeom[i]->FindIntersection(eray);
		if ( t[i] >= 0 && t[i] < temp ) {
			temp = t[i];
			t_index = i;
		}
	}
	if ( temp == 999999.0f ) {return -999999.0f;}

	current_index = t_index;
		
	///////////////////////////////////////////////
	// Get Intersection Position
	Vector3 InterPos = pGeom[current_index]->GetIntersectionPos(t[current_index], ray);
	//if (InterPos.x < -500 || InterPos.x > 500) {return -999999.0f;}
	//if (InterPos.z < -500 || InterPos.z > 500) {return -999999.0f;}

	Vector3 vNormal = pGeom[current_index]->GetNormalAtPos(InterPos);
	
	*pInterPos = InterPos;
	*pNormal = vNormal;

	//////////////////////////////////
	// Get Light Vector
	Vector3 vLight = vLightPosition - InterPos;
	float Light_Length = vLight.Length();
	vLight.Normalize();

	////////////////////////////////////
	// check shadow region
	bool IsShadow = false;
	for (int i = 0; i < MAX_GEOM; i++)
	{
		if ( i == current_index ) continue;
		float t = pGeom[i]->FindIntersection(Ray(InterPos, vLight));
		if ( t > 0.0f && t < Light_Length )
		{
			IsShadow = true;
			break;
		}
	}

	/////////////////////////////////
	// Diffuse & Specular light
	if (!IsShadow) {
		float intensity = MAX(vLight * vNormal, 0.0f);
		Intensity += intensity;
		if (intensity > 0.0f) {
			Vector3 vReflect = vNormal * (vLight * vNormal) * 2  - vLight;
			Intensity += powf(MAX(-ray.dir * vReflect, 0.0f), 30);
		}
	}

	//Intensity += 0.1f;  // Ambient

	return Intensity;
}


void RayTracer::Render(int thread_no)
{
	Vector3 vRay, vPoint;
	Vector3 InterPos, vNormal, vLight;
	float intensity;
	unsigned offset;
	WORD color565;
	//float t[MAX_GEOM];
	int index = 0;
	
	offset = BytesPerScanline * thread_no;

	for (int y = thread_no; y < SCREEN_HEIGHT; y+= 4)
	{
		for (int x = 0; x < SCREEN_WIDTH; x++)
		{
			// make incident view ray
			vRay = MakeRay(x - SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - y, 256);
			
			Ray ViewRay;
			ViewRay.dir = vRay;
			ViewRay.startp = Vector3(0.0f, 0.0f, 0.0f);

			float Intensity = 0;
			
			intensity = RayTrace(ViewRay, &InterPos, &vNormal);
			if (intensity < 0.0f) {offset += 2; continue;}
			Intensity += intensity;


			/////////////////////////////////
			///// Reflection 
			float R_Factor = 0.4f;

			if (true) {
			//if ( current_index[thread_no] != 0) {
				Vector3 vReflect = vNormal * (-ViewRay.dir * vNormal) * 2  + ViewRay.dir;
				
				Ray R_ray(InterPos, vReflect);
				intensity = RayTrace(R_ray, &InterPos, &vNormal); 
				if (intensity < 0.0f) {goto DRAWPIXEL;}
				Intensity += intensity * R_Factor;
				
				Vector3 vReflect2 = vNormal * (-R_ray.dir * vNormal) * 2  + R_ray.dir;
				
				intensity = RayTrace(Ray(InterPos, vReflect2), &InterPos, &vNormal); 
				if (intensity < 0.0f) {goto DRAWPIXEL;}
				Intensity += intensity * R_Factor;
			}
		
			//////////////////////////////
			///  Refraction
			float T_Factor = 0.9f;
			if ( false ) {
			//if ( current_index[thread_no] == 0) {
				float n = 1.0f / 1.5f;
				float cosI = ViewRay.dir * vNormal;
				float sinT2 = n * n * (1.0f - cosI * cosI);
				if ( sinT2 > 1.0f ) {goto DRAWPIXEL;} // total internal reflection
				Vector3 vRefract = n * ViewRay.dir - (n + sqrtf(1.0f - sinT2)) * vNormal;
				Ray T_ray(InterPos, vRefract);
				
				T_ray.startp += vRefract * EPSILON; ////////// 
				//T_ray.dir = ViewRay.dir;
				float t = pGeom[current_index[thread_no]]->FindInnerIntersection(T_ray);
				Vector3 T_pos = pGeom[current_index[thread_no]]->GetIntersectionPos(t, T_ray);
				Vector3 T_normal = -(pGeom[current_index[thread_no]]->GetNormalAtPos(T_pos));

				n = 1.0f / 1.0f;
				cosI = T_ray.dir * T_normal;
				sinT2 = n * n * (1.0f - cosI * cosI);
				if ( sinT2 > 1.0f ) {goto DRAWPIXEL;} // total internal reflection
				Vector3 vRefract2 = n * T_ray.dir - (n + sqrtf(1.0f - sinT2)) * T_normal;
				Ray O_ray(T_pos, vRefract2);
				
				intensity = RayTrace(T_ray, &InterPos, &vNormal); 
				if (intensity < 0.0f) {goto DRAWPIXEL;}
				Intensity += intensity * T_Factor;
				
				///////////////////////////////////////////////////////
				Vector3 vReflect = vNormal * (-ViewRay.dir * vNormal) * 2  + ViewRay.dir;
				
				Ray R_ray(InterPos, vReflect);
				intensity = RayTrace(R_ray, &InterPos, &vNormal); 
				if (intensity < 0.0f) {goto DRAWPIXEL;}
				Intensity += intensity * R_Factor;
				
				Vector3 vReflect2 = vNormal * (-R_ray.dir * vNormal) * 2  + R_ray.dir;
				
				intensity = RayTrace(Ray(InterPos, vReflect2), &InterPos, &vNormal); 
				if (intensity < 0.0f) {goto DRAWPIXEL;}
				Intensity += intensity * R_Factor;
			}
		
DRAWPIXEL:
			if ( Intensity > 1.0f ) Intensity = 1.0f;
			if ( Intensity < 0.0f ) Intensity = 0.0f;

			// Draw Pixel
			BYTE B,G,R;
			BYTE color = BYTE(Intensity * 255);

/*
			switch (thread_no) {
			case 0:
				B = 0;
				G = 0;
				R = color;
				break;

			case 1:
				B = 0;
				G = color;
				R = 0;

				break;

			case 2:
				B = color;
				G = 0;
				R = 0;

				break;

			case 3:
				B = 0;
				G = color;
				R = color;

				break;
			}
*/

			B = color;
			G = color;
			R = color;

			color565 = R >> 3 << 11;
			color565 |= G >> 2 << 5;
			color565 |= B >> 3;
			*(WORD*)(pBits + offset) = color565;

			offset += 2;
			
		}
		offset += BytesPerScanline * 3;
	}
}