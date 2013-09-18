// papi.h
//
// Copyright 1997-1998 by David K. McAllister
// http://www.cs.unc.edu/~davemc/Particle
//
// Include this file in all applications that use the Particle System API.
//
// (l) 3004:forum::für::umläute:2003 modified for Gem
//                                   added KillSlow again (like in 1.11)

#ifndef _particle_api_h
#define _particle_api_h

#include <stdlib.h>

// This is the major and minor version number of this release of the API.
#define P_VERSION 120

#ifdef _WIN32
#include <windows.h>
#define PARTICLEDLL_API
#else
#define PARTICLEDLL_API
#endif

// Actually this must be < sqrt(MAXFLOAT) since we store this value squared.
#define P_MAXFLOAT 1.0e16f

#ifdef MAXINT
#define P_MAXINT MAXINT
#else
#define P_MAXINT 0x7fffffff
#endif

#define P_EPS 1e-3f

//////////////////////////////////////////////////////////////////////
// Type codes for domains
PARTICLEDLL_API enum PDomainEnum
{
	PDPoint = 0, // Single point
	PDLine = 1, // Line segment
	PDTriangle = 2, // Triangle
	PDPlane = 3, // Arbitrarily-oriented plane
	PDBox = 4, // Axis-aligned box
	PDSphere = 5, // Sphere
	PDCylinder = 6, // Cylinder
	PDCone = 7, // Cone
	PDBlob = 8, // Gaussian blob
	PDDisc = 9, // Arbitrarily-oriented disc
	PDRectangle = 10 // Rhombus-shaped planar region
};

// State setting calls

PARTICLEDLL_API void pColor(float red, float green, float blue, float alpha = 1.0f);

PARTICLEDLL_API void pColorD(float alpha, PDomainEnum dtype,
							 float a0 = 0.0f, float a1 = 0.0f, float a2 = 0.0f,
							 float a3 = 0.0f, float a4 = 0.0f, float a5 = 0.0f,
							 float a6 = 0.0f, float a7 = 0.0f, float a8 = 0.0f);

PARTICLEDLL_API void pSize(float size_x, float size_y = 1.0f, float size_z = 1.0f);

PARTICLEDLL_API void pSizeD(PDomainEnum dtype,
						  float a0 = 0.0f, float a1 = 0.0f, float a2 = 0.0f,
						  float a3 = 0.0f, float a4 = 0.0f, float a5 = 0.0f,
						  float a6 = 0.0f, float a7 = 0.0f, float a8 = 0.0f);

PARTICLEDLL_API void pStartingAge(float age, float sigma = 1.0f);

PARTICLEDLL_API void pTimeStep(float new_dt);

PARTICLEDLL_API void pVelocity(float x, float y, float z);

PARTICLEDLL_API void pVelocityD(PDomainEnum dtype,
								float a0 = 0.0f, float a1 = 0.0f, float a2 = 0.0f,
								float a3 = 0.0f, float a4 = 0.0f, float a5 = 0.0f,
								float a6 = 0.0f, float a7 = 0.0f, float a8 = 0.0f);

PARTICLEDLL_API void pVertexB(float x, float y, float z);

PARTICLEDLL_API void pVertexBD(PDomainEnum dtype,
							  float a0 = 0.0f, float a1 = 0.0f, float a2 = 0.0f,
							  float a3 = 0.0f, float a4 = 0.0f, float a5 = 0.0f,
							  float a6 = 0.0f, float a7 = 0.0f, float a8 = 0.0f);

PARTICLEDLL_API void pVertexBTracks(bool track_vertex = true);


// Action List Calls

PARTICLEDLL_API void pCallActionList(int action_list_num);

PARTICLEDLL_API void pDeleteActionLists(int action_list_num, int action_list_count = 1);

PARTICLEDLL_API void pEndActionList();

PARTICLEDLL_API int pGenActionLists(int action_list_count = 1);

PARTICLEDLL_API void pNewActionList(int action_list_num);


// Particle Group Calls

PARTICLEDLL_API void pCopyGroup(int p_src_group_num, int index = 0, int copy_count = P_MAXINT);

PARTICLEDLL_API void pCurrentGroup(int p_group_num);

PARTICLEDLL_API void pDeleteParticleGroups(int p_group_num, int p_group_count = 1);

PARTICLEDLL_API void pDrawGroupl(int dlist, bool const_size = false,
								 bool const_color = false, bool const_rotation = false);

PARTICLEDLL_API void pDrawGroupp(int primitive, bool const_size = false,
								 bool const_color = false);

PARTICLEDLL_API int pGenParticleGroups(int p_group_count = 1, int max_particles = 0);

PARTICLEDLL_API int pGetGroupCount();

PARTICLEDLL_API int pGetParticles(int index, int count, float *position = NULL, float *color = NULL,
								  float *vel = NULL, float *size = NULL, float *age = NULL);

PARTICLEDLL_API int pSetMaxParticles(int max_count);


// Actions

PARTICLEDLL_API void pAvoid(float magnitude, float epsilon, float look_ahead, 
							 PDomainEnum dtype,
							 float a0 = 0.0f, float a1 = 0.0f, float a2 = 0.0f,
							 float a3 = 0.0f, float a4 = 0.0f, float a5 = 0.0f,
							 float a6 = 0.0f, float a7 = 0.0f, float a8 = 0.0f);

PARTICLEDLL_API void pBounce(float friction, float resilience, float cutoff,
							 PDomainEnum dtype,
							 float a0 = 0.0f, float a1 = 0.0f, float a2 = 0.0f,
							 float a3 = 0.0f, float a4 = 0.0f, float a5 = 0.0f,
							 float a6 = 0.0f, float a7 = 0.0f, float a8 = 0.0f);

PARTICLEDLL_API void pCopyVertexB(bool copy_pos = true, bool copy_vel = false);

PARTICLEDLL_API void pDamping(float damping_x, float damping_y, float damping_z,
							 float vlow = 0.0f, float vhigh = P_MAXFLOAT);

PARTICLEDLL_API void pExplosion(float center_x, float center_y, float center_z, float velocity,
				float magnitude, float stdev, float epsilon = P_EPS, float age = 0.0f);

PARTICLEDLL_API void pFollow(float magnitude = 1.0f, float epsilon = P_EPS, float max_radius = P_MAXFLOAT);

PARTICLEDLL_API void pGravitate(float magnitude = 1.0f, float epsilon = P_EPS, float max_radius = P_MAXFLOAT);

PARTICLEDLL_API void pGravity(float dir_x, float dir_y, float dir_z);

PARTICLEDLL_API void pJet(float center_x, float center_y, float center_z, float magnitude = 1.0f,
						 float epsilon = P_EPS, float max_radius = P_MAXFLOAT);

PARTICLEDLL_API void pKillOld(float age_limit, bool kill_less_than = false);

PARTICLEDLL_API void pKillSlow(float speedLimit, bool kill_less_than = true);

PARTICLEDLL_API void pMatchVelocity(float magnitude = 1.0f, float epsilon = P_EPS,
									float max_radius = P_MAXFLOAT);

PARTICLEDLL_API void pMove();

PARTICLEDLL_API void pOrbitLine(float p_x, float p_y, float p_z,
							float axis_x, float axis_y, float axis_z, float magnitude = 1.0f,
							float epsilon = P_EPS, float max_radius = P_MAXFLOAT);

PARTICLEDLL_API void pOrbitPoint(float center_x, float center_y, float center_z,
							 float magnitude = 1.0f, float epsilon = P_EPS,
							 float max_radius = P_MAXFLOAT);

PARTICLEDLL_API void pRandomAccel(PDomainEnum dtype,
								 float a0 = 0.0f, float a1 = 0.0f, float a2 = 0.0f,
								 float a3 = 0.0f, float a4 = 0.0f, float a5 = 0.0f,
								 float a6 = 0.0f, float a7 = 0.0f, float a8 = 0.0f);

PARTICLEDLL_API void pRandomDisplace(PDomainEnum dtype,
									 float a0 = 0.0f, float a1 = 0.0f, float a2 = 0.0f,
									 float a3 = 0.0f, float a4 = 0.0f, float a5 = 0.0f,
									 float a6 = 0.0f, float a7 = 0.0f, float a8 = 0.0f);

PARTICLEDLL_API void pRandomVelocity(PDomainEnum dtype,
									 float a0 = 0.0f, float a1 = 0.0f, float a2 = 0.0f,
									 float a3 = 0.0f, float a4 = 0.0f, float a5 = 0.0f,
									 float a6 = 0.0f, float a7 = 0.0f, float a8 = 0.0f);

PARTICLEDLL_API void pRestore(float time);

PARTICLEDLL_API void pShade(float color_x, float color_y, float color_z,
							float alpha, float scale);

PARTICLEDLL_API void pSink(bool kill_inside, PDomainEnum dtype,
						  float a0 = 0.0f, float a1 = 0.0f, float a2 = 0.0f,
						  float a3 = 0.0f, float a4 = 0.0f, float a5 = 0.0f,
						  float a6 = 0.0f, float a7 = 0.0f, float a8 = 0.0f);

PARTICLEDLL_API void pSinkVelocity(bool kill_inside, PDomainEnum dtype,
								  float a0 = 0.0f, float a1 = 0.0f, float a2 = 0.0f,
								  float a3 = 0.0f, float a4 = 0.0f, float a5 = 0.0f,
								  float a6 = 0.0f, float a7 = 0.0f, float a8 = 0.0f);

PARTICLEDLL_API void pSource(float particle_rate, PDomainEnum dtype,
							 float a0 = 0.0f, float a1 = 0.0f, float a2 = 0.0f,
							 float a3 = 0.0f, float a4 = 0.0f, float a5 = 0.0f,
							 float a6 = 0.0f, float a7 = 0.0f, float a8 = 0.0f);

PARTICLEDLL_API void pSpeedLimit(float min_speed, float max_speed = P_MAXFLOAT);

PARTICLEDLL_API void pTargetColor(float color_x, float color_y, float color_z,
								 float alpha, float scale);

PARTICLEDLL_API void pTargetSize(float size_x, float size_y, float size_z,
								 float scale_x = 0.0f, float scale_y = 0.0f, float scale_z = 0.0f);

PARTICLEDLL_API void pTargetVelocity(float vel_x, float vel_y, float vel_z, float scale);

PARTICLEDLL_API void pVertex(float x, float y, float z);

PARTICLEDLL_API void pVortex(float center_x, float center_y, float center_z,
							 float axis_x, float axis_y, float axis_z,
							 float magnitude = 1.0f, float epsilon = P_EPS,
							 float max_radius = P_MAXFLOAT);

#endif
