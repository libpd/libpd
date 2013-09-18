// action_api.cpp
//
// Copyright 1997-1998 by David K. McAllister
//
// This file implements the action API calls by creating
// action class instances, which are either executed or
// added to an action list.
// (l) 3004:forum::für::umläute:2003 modified for Gem
//                                   added KillSlow again (like in 1.11)

#include "partlib_general.h"

extern void _pAddActionToList(ParticleAction *S, int size);
extern void _pCallActionList(ParticleAction *pa, int num_actions,
							 ParticleGroup *pg);

// Do not call this function.
void _pSendAction(ParticleAction *S, PActionEnum type, int size)
{
	_ParticleState &_ps = _GetPState();

	S->type = type;
	
	if(_ps.in_new_list)
	{
		_pAddActionToList(S, size);
	}
	else
	{
		// Immediate mode. Execute it.
		// This is a hack to give them local access to dt.
		S->dt = _ps.dt;
		_pCallActionList(S, 1, _ps.pgrp);
	}
}

PARTICLEDLL_API void pAvoid(float magnitude, float epsilon, float look_ahead, 
			 PDomainEnum dtype,
			 float a0, float a1, float a2,
			 float a3, float a4, float a5,
			 float a6, float a7, float a8)
{
	PAAvoid S;
	
	S.position = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
	S.magnitude = magnitude;
	S.epsilon = epsilon;
	S.look_ahead = look_ahead;

	_pSendAction(&S, PAAvoidID, sizeof(PAAvoid));
}

PARTICLEDLL_API void pBounce(float friction, float resilience, float cutoff,
			 PDomainEnum dtype,
			 float a0, float a1, float a2,
			 float a3, float a4, float a5,
			 float a6, float a7, float a8)
{
	PABounce S;
	
	S.position = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
	S.oneMinusFriction = 1.0f - friction;
	S.resilience = resilience;
	S.cutoffSqr = fsqr(cutoff);
	
	_pSendAction(&S, PABounceID, sizeof(PABounce));
}

PARTICLEDLL_API void pCopyVertexB(bool copy_pos, bool copy_vel)
{
	PACopyVertexB S;

	S.copy_pos = copy_pos;
	S.copy_vel = copy_vel;
	
	_pSendAction(&S, PACopyVertexBID, sizeof(PACopyVertexB));
}

PARTICLEDLL_API void pDamping(float damping_x, float damping_y, float damping_z,
			 float vlow, float vhigh)
{
	PADamping S;
	
	S.damping = pVector(damping_x, damping_y, damping_z);
	S.vlowSqr = fsqr(vlow);
	S.vhighSqr = fsqr(vhigh);
	
	_pSendAction(&S, PADampingID, sizeof(PADamping));
}

PARTICLEDLL_API void pExplosion(float center_x, float center_y, float center_z, float velocity,
				float magnitude, float stdev, float epsilon, float age)
{
	PAExplosion S;
	
	S.center = pVector(center_x, center_y, center_z);
	S.velocity = velocity;
	S.magnitude = magnitude;
	S.stdev = stdev;
	S.epsilon = epsilon;
	S.age = age;
	
	if(S.epsilon < 0.0f)
		S.epsilon = P_EPS;
	
	_pSendAction(&S, PAExplosionID, sizeof(PAExplosion));
}

PARTICLEDLL_API void pFollow(float magnitude, float epsilon, float max_radius)
{
	PAFollow S;
	
	S.magnitude = magnitude;
	S.epsilon = epsilon;
	S.max_radius = max_radius;
	
	_pSendAction(&S, PAFollowID, sizeof(PAFollow));
}

PARTICLEDLL_API void pGravitate(float magnitude, float epsilon, float max_radius)
{
	PAGravitate S;
	
	S.magnitude = magnitude;
	S.epsilon = epsilon;
	S.max_radius = max_radius;
	
	_pSendAction(&S, PAGravitateID, sizeof(PAGravitate));
}

PARTICLEDLL_API void pGravity(float dir_x, float dir_y, float dir_z)
{
	PAGravity S;
	
	S.direction = pVector(dir_x, dir_y, dir_z);
	
	_pSendAction(&S, PAGravityID, sizeof(PAGravity));
}

PARTICLEDLL_API void pJet(float center_x, float center_y, float center_z,
		 float magnitude, float epsilon, float max_radius)
{
	_ParticleState &_ps = _GetPState();

	PAJet S;
	
	S.center = pVector(center_x, center_y, center_z);
	S.acc = _ps.Vel;
	S.magnitude = magnitude;
	S.epsilon = epsilon;
	S.max_radius = max_radius;
	
	_pSendAction(&S, PAJetID, sizeof(PAJet));
}

PARTICLEDLL_API void pKillOld(float age_limit, bool kill_less_than)
{
	PAKillOld S;
	
	S.age_limit = age_limit;
	S.kill_less_than = kill_less_than;
	
	_pSendAction(&S, PAKillOldID, sizeof(PAKillOld));
}

PARTICLEDLL_API void pKillSlow(float speedLimit, bool kill_less_than)
{
	PAKillSlow S;
	
	S.speedLimitSqr = fsqr(speedLimit);
	S.kill_less_than = kill_less_than;
	
	_pSendAction(&S, PAKillSlowID, sizeof(PAKillSlow));
}

PARTICLEDLL_API void pMatchVelocity(float magnitude, float epsilon, float max_radius)
{
	PAMatchVelocity S;
	
	S.magnitude = magnitude;
	S.epsilon = epsilon;
	S.max_radius = max_radius;
	
	_pSendAction(&S, PAMatchVelocityID, sizeof(PAMatchVelocity));
}

PARTICLEDLL_API void pMove()
{
	PAMove S;
	
	_pSendAction(&S, PAMoveID, sizeof(PAMove));
}

PARTICLEDLL_API void pOrbitLine(float p_x, float p_y, float p_z,
				float axis_x, float axis_y, float axis_z,
				float magnitude, float epsilon, float max_radius)
{
	PAOrbitLine S;
	
	S.p = pVector(p_x, p_y, p_z);
	S.axis = pVector(axis_x, axis_y, axis_z);
	S.axis.normalize();
	S.magnitude = magnitude;
	S.epsilon = epsilon;
	S.max_radius = max_radius;
	
	_pSendAction(&S, PAOrbitLineID, sizeof(PAOrbitLine));
}

PARTICLEDLL_API void pOrbitPoint(float center_x, float center_y, float center_z,
				 float magnitude, float epsilon, float max_radius)
{
	PAOrbitPoint S;
	
	S.center = pVector(center_x, center_y, center_z);
	S.magnitude = magnitude;
	S.epsilon = epsilon;
	S.max_radius = max_radius;
	
	_pSendAction(&S, PAOrbitPointID, sizeof(PAOrbitPoint));
}

PARTICLEDLL_API void pRandomAccel(PDomainEnum dtype,
				 float a0, float a1, float a2,
				 float a3, float a4, float a5,
				 float a6, float a7, float a8)
{
	PARandomAccel S;
	
	S.gen_acc = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
	
	_pSendAction(&S, PARandomAccelID, sizeof(PARandomAccel));
}

PARTICLEDLL_API void pRandomDisplace(PDomainEnum dtype,
					 float a0, float a1, float a2,
					 float a3, float a4, float a5,
					 float a6, float a7, float a8)
{
	PARandomDisplace S;
	
	S.gen_disp = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
	
	_pSendAction(&S, PARandomDisplaceID, sizeof(PARandomDisplace));
}

PARTICLEDLL_API void pRandomVelocity(PDomainEnum dtype,
					 float a0, float a1, float a2,
					 float a3, float a4, float a5,
					 float a6, float a7, float a8)
{
	PARandomVelocity S;
	
	S.gen_vel = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
	
	_pSendAction(&S, PARandomVelocityID, sizeof(PARandomVelocity));
}

PARTICLEDLL_API void pRestore(float time_left)
{
	PARestore S;
	
	S.time_left = time_left;
	
	_pSendAction(&S, PARestoreID, sizeof(PARestore));
}

PARTICLEDLL_API void pSink(bool kill_inside, PDomainEnum dtype,
		  float a0, float a1, float a2,
		  float a3, float a4, float a5,
		  float a6, float a7, float a8)
{
	PASink S;
	
	S.kill_inside = kill_inside;
	S.position = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
	
	_pSendAction(&S, PASinkID, sizeof(PASink));
}

PARTICLEDLL_API void pSinkVelocity(bool kill_inside, PDomainEnum dtype,
				  float a0, float a1, float a2,
				  float a3, float a4, float a5,
				  float a6, float a7, float a8)
{
	PASinkVelocity S;
	
	S.kill_inside = kill_inside;
	S.velocity = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
	
	_pSendAction(&S, PASinkVelocityID, sizeof(PASinkVelocity));
}

PARTICLEDLL_API void pSource(float particle_rate, PDomainEnum dtype,
			 float a0, float a1, float a2,
			 float a3, float a4, float a5,
			 float a6, float a7, float a8)
{
	_ParticleState &_ps = _GetPState();

	PASource S;
	
	S.particle_rate = particle_rate;
	S.position = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
	S.positionB = _ps.VertexB;
	S.size = _ps.Size;
	S.velocity = _ps.Vel;
	S.color = _ps.Color;
	S.alpha = _ps.Alpha;
	S.age = _ps.Age;
	S.age_sigma = _ps.AgeSigma;
	S.vertexB_tracks = _ps.vertexB_tracks;
	
	_pSendAction(&S, PASourceID, sizeof(PASource));
}

PARTICLEDLL_API void pSpeedLimit(float min_speed, float max_speed)
{
	PASpeedLimit S;

	S.min_speed = min_speed;
	S.max_speed = max_speed;

	_pSendAction(&S, PASpeedLimitID, sizeof(PASpeedLimit));
}

PARTICLEDLL_API void pTargetColor(float color_x, float color_y, float color_z,
				 float alpha, float scale)
{
	PATargetColor S;
	
	S.color = pVector(color_x, color_y, color_z);
	S.alpha = alpha;
	S.scale = scale;
	
	_pSendAction(&S, PATargetColorID, sizeof(PATargetColor));
}

PARTICLEDLL_API void pTargetSize(float size_x, float size_y, float size_z,
						 float scale_x, float scale_y, float scale_z)
{
	PATargetSize S;
	
	S.size = pVector(size_x, size_y, size_z);
	S.scale = pVector(scale_x, scale_y, scale_z);
	
	_pSendAction(&S, PATargetSizeID, sizeof(PATargetSize));
}

PARTICLEDLL_API void pTargetVelocity(float vel_x, float vel_y, float vel_z, float scale)
{
	PATargetVelocity S;
	
	S.velocity = pVector(vel_x, vel_y, vel_z);
	S.scale = scale;
	
	_pSendAction(&S, PATargetVelocityID, sizeof(PATargetVelocity));
}

// If in immediate mode, quickly add a vertex.
// If building an action list, call pSource.
PARTICLEDLL_API void pVertex(float x, float y, float z)
{
	_ParticleState &_ps = _GetPState();

	if(_ps.in_new_list)
	{
		pSource(1, PDPoint, x, y, z);
		return;
	}
	
	// Immediate mode. Quickly add the vertex.
	if(_ps.pgrp == NULL)
		return;
	
	pVector pos(x, y, z);
	pVector siz, vel, col, posB;
	if(_ps.vertexB_tracks)
		posB = pos;
	else
		_ps.VertexB.Generate(posB);
	_ps.Size.Generate(siz);
	_ps.Vel.Generate(vel);
	_ps.Color.Generate(col);
	_ps.pgrp->Add(pos, posB, siz, vel, col, _ps.Alpha, _ps.Age);
}

PARTICLEDLL_API void pVortex(float center_x, float center_y, float center_z,
			 float axis_x, float axis_y, float axis_z,
			 float magnitude, float epsilon, float max_radius)
{
	PAVortex S;
	
	S.center = pVector(center_x, center_y, center_z);
	S.axis = pVector(axis_x, axis_y, axis_z);
	S.axis.normalize();
	S.magnitude = magnitude;
	S.epsilon = epsilon;
	S.max_radius = max_radius;
	
	_pSendAction(&S, PAVortexID, sizeof(PAVortex));
}
