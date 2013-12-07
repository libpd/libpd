// actions.cpp
//
// Copyright 1997-1998 by David K. McAllister
//
// I used code Copyright 1997 by Jonathan P. Leech
// as an example in implenting this.
//
// This file implements the dynamics of particle actions.
// (l) 3004:forum::für::umläute:2003 modified for Gem
//                                   added KillSlow again (like in 1.11)

#include "partlib_general.h"
#include <float.h>

#define SQRT2PI 2.506628274631000502415765284811045253006
#define ONEOVERSQRT2PI (1. / SQRT2PI)

#include "Utils/GemMath.h"

// To offset [0 .. 1] vectors to [-.5 .. .5]
static pVector vHalf(0.5, 0.5, 0.5);

static inline pVector RandVec()
{
	return pVector(drand48(), drand48(), drand48());
}

// Return a random number with a normal distribution.
static inline float NRand(float sigma = 1.0f)
{
#define ONE_OVER_SIGMA_EXP (1.0f / 0.7975f)
	
	if(sigma == 0) return 0;
	
	float y;
	do
	{
		y = -logf(drand48());
	}
	while(drand48() > expf(-fsqr(y - 1.0f)*0.5f));
	
	if(rand() & 0x1)
		return y * sigma * ONE_OVER_SIGMA_EXP;
	else
		return -y * sigma * ONE_OVER_SIGMA_EXP;
}

void PAAvoid::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	
	switch(position.type)
	{
	case PDPlane:
		{
			if(look_ahead < P_MAXFLOAT)
			{
				for(int i = 0; i < group->p_count; i++)
				{
					Particle &m = group->list[i];
					
					// p2 stores the plane normal (the a,b,c of the plane eqn).
					// Old and new distances: dist(p,plane) = n * p + d
					// radius1 stores -n*p, which is d.
					float dist = m.pos * position.p2 + position.radius1;
					
					if(dist < look_ahead)
					{
						float vm = m.vel.length();
						pVector Vn = m.vel / vm;
						// float dot = Vn * position.p2;
						
						pVector tmp = (position.p2 * (magdt / (dist*dist+epsilon))) + Vn;
						m.vel = tmp * (vm / tmp.length());
					}
				}
			}
			else
			{
				for(int i = 0; i < group->p_count; i++)
				{
					Particle &m = group->list[i];
					
					// p2 stores the plane normal (the a,b,c of the plane eqn).
					// Old and new distances: dist(p,plane) = n * p + d
					// radius1 stores -n*p, which is d.
					float dist = m.pos * position.p2 + position.radius1;
					
					float vm = m.vel.length();
					pVector Vn = m.vel / vm;
					// float dot = Vn * position.p2;
					
					pVector tmp = (position.p2 * (magdt / (dist*dist+epsilon))) + Vn;
					m.vel = tmp * (vm / tmp.length());
				}
			}
		}
		break;
	case PDRectangle:
		{
			// Compute the inverse matrix of the plane basis.
			pVector &u = position.u;
			pVector &v = position.v;
			
			// The normalized bases are needed inside the loop.
			pVector un = u / position.radius1Sqr;
			pVector vn = v / position.radius2Sqr;
			
			// w = u cross v
			float wx = u.y*v.z-u.z*v.y;
			float wy = u.z*v.x-u.x*v.z;
			float wz = u.x*v.y-u.y*v.x;
			
			float det = 1/(wz*u.x*v.y-wz*u.y*v.x-u.z*wx*v.y-u.x*v.z*wy+v.z*wx*u.y+u.z*v.x*wy);
			
			pVector s1((v.y*wz-v.z*wy), (v.z*wx-v.x*wz), (v.x*wy-v.y*wx));
			s1 *= det;
			pVector s2((u.y*wz-u.z*wy), (u.z*wx-u.x*wz), (u.x*wy-u.y*wx));
			s2 *= -det;
			
			// See which particles bounce.
			for(int i = 0; i < group->p_count; i++)
			{
				Particle &m = group->list[i];
				
				// See if particle's current and next positions cross plane.
				// If not, couldn't bounce, so keep going.
				pVector pnext(m.pos + m.vel * dt * look_ahead);
				
				// p2 stores the plane normal (the a,b,c of the plane eqn).
				// Old and new distances: dist(p,plane) = n * p + d
				// radius1 stores -n*p, which is d.
				float distold = m.pos * position.p2 + position.radius1;
				float distnew = pnext * position.p2 + position.radius1;
				
				// Opposite signs if product < 0
				// There is no faster way to do this.
				if(distold * distnew >= 0)
					continue;
				
				float nv = position.p2 * m.vel;
				float t = -distold / nv;
				
				// Actual intersection point p(t) = pos + vel t
				pVector phit(m.pos + m.vel * t);
				
				// Offset from origin in plane, p - origin
				pVector offset(phit - position.p1);
				
				// Dot product with basis vectors of old frame
				// in terms of new frame gives position in uv frame.
				float upos = offset * s1;
				float vpos = offset * s2;
				
				// Did it cross plane outside triangle?
				if(upos < 0 || vpos < 0 || upos > 1 || vpos > 1)
					continue;
				
				// A hit! A most palpable hit!
				// Compute distance to the three edges.
				pVector uofs = (un * (un * offset)) - offset;
				float udistSqr = uofs.length2();
				pVector vofs = (vn * (vn * offset)) - offset;
				float vdistSqr = vofs.length2();
				
				pVector foffset((u + v) - offset);
				pVector fofs = (un * (un * foffset)) - foffset;
				float fdistSqr = fofs.length2();
				pVector gofs = (un * (un * foffset)) - foffset;
				float gdistSqr = gofs.length2();
				
				pVector S;
				if(udistSqr <= vdistSqr && udistSqr <= fdistSqr
					&& udistSqr <= gdistSqr) S = uofs;
				else if(vdistSqr <= fdistSqr && vdistSqr <= gdistSqr) S = vofs;
				else if(fdistSqr <= gdistSqr) S = fofs;
				else S = gofs;
				
				S.normalize();
				
				// We now have a vector to safety.
				float vm = m.vel.length();
				pVector Vn = m.vel / vm;
				
				// Blend S into V.
				pVector tmp = (S * (magdt / (t*t+epsilon))) + Vn;
				m.vel = tmp * (vm / tmp.length());
			}
		}
		break;
	case PDTriangle:
		{
			// Compute the inverse matrix of the plane basis.
			pVector &u = position.u;
			pVector &v = position.v;
			
			// The normalized bases are needed inside the loop.
			pVector un = u / position.radius1Sqr;
			pVector vn = v / position.radius2Sqr;
			
			// f is the third (non-basis) triangle edge.
			pVector f = v - u;
			pVector fn(f);
			fn.normalize();
			
			// w = u cross v
			float wx = u.y*v.z-u.z*v.y;
			float wy = u.z*v.x-u.x*v.z;
			float wz = u.x*v.y-u.y*v.x;
			
			float det = 1/(wz*u.x*v.y-wz*u.y*v.x-u.z*wx*v.y-u.x*v.z*wy+v.z*wx*u.y+u.z*v.x*wy);
			
			pVector s1((v.y*wz-v.z*wy), (v.z*wx-v.x*wz), (v.x*wy-v.y*wx));
			s1 *= det;
			pVector s2((u.y*wz-u.z*wy), (u.z*wx-u.x*wz), (u.x*wy-u.y*wx));
			s2 *= -det;
			
			// See which particles bounce.
			for(int i = 0; i < group->p_count; i++)
			{
				Particle &m = group->list[i];
				
				// See if particle's current and next positions cross plane.
				// If not, couldn't bounce, so keep going.
				pVector pnext(m.pos + m.vel * dt * look_ahead);
				
				// p2 stores the plane normal (the a,b,c of the plane eqn).
				// Old and new distances: dist(p,plane) = n * p + d
				// radius1 stores -n*p, which is d.
				float distold = m.pos * position.p2 + position.radius1;
				float distnew = pnext * position.p2 + position.radius1;
				
				// Opposite signs if product < 0
				// Is there a faster way to do this?
				if(distold * distnew >= 0)
					continue;
				
				float nv = position.p2 * m.vel;
				float t = -distold / nv;
				
				// Actual intersection point p(t) = pos + vel t
				pVector phit(m.pos + m.vel * t);
				
				// Offset from origin in plane, p - origin
				pVector offset(phit - position.p1);
				
				// Dot product with basis vectors of old frame
				// in terms of new frame gives position in uv frame.
				float upos = offset * s1;
				float vpos = offset * s2;
				
				// Did it cross plane outside triangle?
				if(upos < 0 || vpos < 0 || (upos + vpos) > 1)
					continue;
				
				// A hit! A most palpable hit!
				// Compute distance to the three edges.
				pVector uofs = (un * (un * offset)) - offset;
				float udistSqr = uofs.length2();
				pVector vofs = (vn * (vn * offset)) - offset;
				float vdistSqr = vofs.length2();
				pVector foffset(offset - u);
				pVector fofs = (fn * (fn * foffset)) - foffset;
				float fdistSqr = fofs.length2();
				pVector S;
				if(udistSqr <= vdistSqr && udistSqr <= fdistSqr) S = uofs;
				else if(vdistSqr <= fdistSqr) S = vofs;
				else S = fofs;
				
				S.normalize();
				
				// We now have a vector to safety.
				float vm = m.vel.length();
				pVector Vn = m.vel / vm;
				
				// Blend S into V.
				pVector tmp = (S * (magdt / (t*t+epsilon))) + Vn;
				m.vel = tmp * (vm / tmp.length());
			}
		}
		break;
	case PDDisc:
		{
			float r1Sqr = fsqr(position.radius1);
			float r2Sqr = fsqr(position.radius2);
			
			// See which particles bounce.
			for(int i = 0; i < group->p_count; i++)
			{
				Particle &m = group->list[i];
				
				// See if particle's current and next positions cross plane.
				// If not, couldn't bounce, so keep going.
				pVector pnext(m.pos + m.vel * dt * look_ahead);
				
				// p2 stores the plane normal (the a,b,c of the plane eqn).
				// Old and new distances: dist(p,plane) = n * p + d
				// radius1 stores -n*p, which is d. radius1Sqr stores d.
				float distold = m.pos * position.p2 + position.radius1Sqr;
				float distnew = pnext * position.p2 + position.radius1Sqr;
				
				// Opposite signs if product < 0
				// Is there a faster way to do this?
				if(distold * distnew >= 0)
					continue;
				
				// Find position at the crossing point by parameterizing
				// p(t) = pos + vel * t
				// Solve dist(p(t),plane) = 0 e.g.
				// n * p(t) + D = 0 ->
				// n * p + t (n * v) + D = 0 ->
				// t = -(n * p + D) / (n * v)
				// Could factor n*v into distnew = distold + n*v and save a bit.
				// Safe since n*v != 0 assured by quick rejection test.
				// This calc is indep. of dt because we have established that it
				// will hit before dt. We just want to know when.
				float nv = position.p2 * m.vel;
				float t = -distold / nv;
				
				// Actual intersection point p(t) = pos + vel t
				pVector phit(m.pos + m.vel * t);
				
				// Offset from origin in plane, phit - origin
				pVector offset(phit - position.p1);
				
				float rad = offset.length2();
				
				if(rad > r1Sqr || rad < r2Sqr)
					continue;
				
				// A hit! A most palpable hit!
				pVector S = offset;
				S.normalize();
				
				// We now have a vector to safety.
				float vm = m.vel.length();
				pVector Vn = m.vel / vm;
				
				// Blend S into V.
				pVector tmp = (S * (magdt / (t*t+epsilon))) + Vn;
				m.vel = tmp * (vm / tmp.length());
			}
		}
		break;
	case PDSphere:
		{
			float rSqr = position.radius1 * position.radius1;
			
			// See which particles are aimed toward the sphere.
			for(int i = 0; i < group->p_count; i++)
			{
				Particle &m = group->list[i];
				
				// First do a ray-sphere intersection test and
				// see if it's soon enough.
				// Can I do this faster without t?
				float vm = m.vel.length();
				pVector Vn = m.vel / vm;
				
				pVector L = position.p1 - m.pos;
				float v = L * Vn;
				
				float disc = rSqr - (L * L) + v * v;
				if(disc < 0)
					continue; // I'm not heading toward it.
				
				// Compute length for second rejection test.
				float t = v - sqrtf(disc);
				if(t < 0 || t > (vm * look_ahead))
					continue;
				
				// Get a vector to safety.
				pVector C = Vn ^ L;
				C.normalize();
				pVector S = Vn ^ C;
				
				// Blend S into V.
				pVector tmp = (S * (magdt / (t*t+epsilon))) + Vn;
				m.vel = tmp * (vm / tmp.length());
			}
		}
		break;
	default:
		break;
	}
}

void PABounce::Execute(ParticleGroup *group)
{
	switch(position.type)
	{
	case PDTriangle:
		{
			// Compute the inverse matrix of the plane basis.
			pVector &u = position.u;
			pVector &v = position.v;
			
			// w = u cross v
			float wx = u.y*v.z-u.z*v.y;
			float wy = u.z*v.x-u.x*v.z;
			float wz = u.x*v.y-u.y*v.x;
			
			float det = 1/(wz*u.x*v.y-wz*u.y*v.x-u.z*wx*v.y-u.x*v.z*wy+v.z*wx*u.y+u.z*v.x*wy);
			
			pVector s1((v.y*wz-v.z*wy), (v.z*wx-v.x*wz), (v.x*wy-v.y*wx));
			s1 *= det;
			pVector s2((u.y*wz-u.z*wy), (u.z*wx-u.x*wz), (u.x*wy-u.y*wx));
			s2 *= -det;
			
			// See which particles bounce.
			for(int i = 0; i < group->p_count; i++)
			{
				Particle &m = group->list[i];
				
				// See if particle's current and next positions cross plane.
				// If not, couldn't bounce, so keep going.
				pVector pnext(m.pos + m.vel * dt);
				
				// p2 stores the plane normal (the a,b,c of the plane eqn).
				// Old and new distances: dist(p,plane) = n * p + d
				// radius1 stores -n*p, which is d.
				float distold = m.pos * position.p2 + position.radius1;
				float distnew = pnext * position.p2 + position.radius1;
				
				// Opposite signs if product < 0
				// Is there a faster way to do this?
				if(distold * distnew >= 0)
					continue;
				
				// Find position at the crossing point by parameterizing
				// p(t) = pos + vel * t
				// Solve dist(p(t),plane) = 0 e.g.
				// n * p(t) + D = 0 ->
				// n * p + t (n * v) + D = 0 ->
				// t = -(n * p + D) / (n * v)
				// Could factor n*v into distnew = distold + n*v and save a bit.
				// Safe since n*v != 0 assured by quick rejection test.
				// This calc is indep. of dt because we have established that it
				// will hit before dt. We just want to know when.
				float nv = position.p2 * m.vel;
				float t = -distold / nv;
				
				// Actual intersection point p(t) = pos + vel t
				pVector phit(m.pos + m.vel * t);
				
				// Offset from origin in plane, p - origin
				pVector offset(phit - position.p1);
				
				// Dot product with basis vectors of old frame
				// in terms of new frame gives position in uv frame.
				float upos = offset * s1;
				float vpos = offset * s2;
				
				// Did it cross plane outside triangle?
				if(upos < 0 || vpos < 0 || (upos + vpos) > 1)
					continue;
				
				// A hit! A most palpable hit!
				
				// Compute tangential and normal components of velocity
				pVector vn(position.p2 * nv); // Normal Vn = (V.N)N
				pVector vt(m.vel - vn); // Tangent Vt = V - Vn
				
				// Compute new velocity heading out:
				// Don't apply friction if tangential velocity < cutoff
				if(vt.length2() <= cutoffSqr)
					m.vel = vt - vn * resilience;
				else
					m.vel = vt * oneMinusFriction - vn * resilience;
			}
		}
		break;
	case PDDisc:
		{
			float r1Sqr = fsqr(position.radius1);
			float r2Sqr = fsqr(position.radius2);
			
			// See which particles bounce.
			for(int i = 0; i < group->p_count; i++)
			{
				Particle &m = group->list[i];
				
				// See if particle's current and next positions cross plane.
				// If not, couldn't bounce, so keep going.
				pVector pnext(m.pos + m.vel * dt);
				
				// p2 stores the plane normal (the a,b,c of the plane eqn).
				// Old and new distances: dist(p,plane) = n * p + d
				// radius1 stores -n*p, which is d. radius1Sqr stores d.
				float distold = m.pos * position.p2 + position.radius1Sqr;
				float distnew = pnext * position.p2 + position.radius1Sqr;
				
				// Opposite signs if product < 0
				// Is there a faster way to do this?
				if(distold * distnew >= 0)
					continue;
				
				// Find position at the crossing point by parameterizing
				// p(t) = pos + vel * t
				// Solve dist(p(t),plane) = 0 e.g.
				// n * p(t) + D = 0 ->
				// n * p + t (n * v) + D = 0 ->
				// t = -(n * p + D) / (n * v)
				// Could factor n*v into distnew = distold + n*v and save a bit.
				// Safe since n*v != 0 assured by quick rejection test.
				// This calc is indep. of dt because we have established that it
				// will hit before dt. We just want to know when.
				float nv = position.p2 * m.vel;
				float t = -distold / nv;
				
				// Actual intersection point p(t) = pos + vel t
				pVector phit(m.pos + m.vel * t);
				
				// Offset from origin in plane, phit - origin
				pVector offset(phit - position.p1);
				
				float rad = offset.length2();
				
				if(rad > r1Sqr || rad < r2Sqr)
					continue;
				
				// A hit! A most palpable hit!
				
				// Compute tangential and normal components of velocity
				pVector vn(position.p2 * nv); // Normal Vn = (V.N)N
				pVector vt(m.vel - vn); // Tangent Vt = V - Vn
				
				// Compute new velocity heading out:
				// Don't apply friction if tangential velocity < cutoff
				if(vt.length2() <= cutoffSqr)
					m.vel = vt - vn * resilience;
				else
					m.vel = vt * oneMinusFriction - vn * resilience;
			}
		}
		break;
	case PDPlane:
		{
			// See which particles bounce.
			for(int i = 0; i < group->p_count; i++)
			{
				Particle &m = group->list[i];
				
				// See if particle's current and next positions cross plane.
				// If not, couldn't bounce, so keep going.
				pVector pnext(m.pos + m.vel * dt);
				
				// p2 stores the plane normal (the a,b,c of the plane eqn).
				// Old and new distances: dist(p,plane) = n * p + d
				// radius1 stores -n*p, which is d.
				float distold = m.pos * position.p2 + position.radius1;
				float distnew = pnext * position.p2 + position.radius1;
				
				// Opposite signs if product < 0
				if(distold * distnew >= 0)
					continue;
				
				// Compute tangential and normal components of velocity
				float nmag = m.vel * position.p2;
				pVector vn(position.p2 * nmag); // Normal Vn = (V.N)N
				pVector vt(m.vel - vn); // Tangent Vt = V - Vn
				
				// Compute new velocity heading out:
				// Don't apply friction if tangential velocity < cutoff
				if(vt.length2() <= cutoffSqr)
					m.vel = vt - vn * resilience;
				else
					m.vel = vt * oneMinusFriction - vn * resilience;
			}
		}
		break;
	case PDRectangle:
		{
			// Compute the inverse matrix of the plane basis.
			pVector &u = position.u;
			pVector &v = position.v;
			
			// w = u cross v
			float wx = u.y*v.z-u.z*v.y;
			float wy = u.z*v.x-u.x*v.z;
			float wz = u.x*v.y-u.y*v.x;
			
			float det = 1/(wz*u.x*v.y-wz*u.y*v.x-u.z*wx*v.y-u.x*v.z*wy+v.z*wx*u.y+u.z*v.x*wy);
			
			pVector s1((v.y*wz-v.z*wy), (v.z*wx-v.x*wz), (v.x*wy-v.y*wx));
			s1 *= det;
			pVector s2((u.y*wz-u.z*wy), (u.z*wx-u.x*wz), (u.x*wy-u.y*wx));
			s2 *= -det;
			
			// See which particles bounce.
			for(int i = 0; i < group->p_count; i++)
			{
				Particle &m = group->list[i];
				
				// See if particle's current and next positions cross plane.
				// If not, couldn't bounce, so keep going.
				pVector pnext(m.pos + m.vel * dt);
				
				// p2 stores the plane normal (the a,b,c of the plane eqn).
				// Old and new distances: dist(p,plane) = n * p + d
				// radius1 stores -n*p, which is d.
				float distold = m.pos * position.p2 + position.radius1;
				float distnew = pnext * position.p2 + position.radius1;
				
				// Opposite signs if product < 0
				if(distold * distnew >= 0)
					continue;
				
				// Find position at the crossing point by parameterizing
				// p(t) = pos + vel * t
				// Solve dist(p(t),plane) = 0 e.g.
				// n * p(t) + D = 0 ->
				// n * p + t (n * v) + D = 0 ->
				// t = -(n * p + D) / (n * v)
				float t = -distold / (position.p2 * m.vel);
				
				// Actual intersection point p(t) = pos + vel t
				pVector phit(m.pos + m.vel * t);
				
				// Offset from origin in plane, p - origin
				pVector offset(phit - position.p1);
				
				// Dot product with basis vectors of old frame
				// in terms of new frame gives position in uv frame.
				float upos = offset * s1;
				float vpos = offset * s2;
				
				// Crossed plane outside bounce region if !(0<=[uv]pos<=1)
				if(upos < 0 || upos > 1 || vpos < 0 || vpos > 1)
					continue;
				
				// A hit! A most palpable hit!
				
				// Compute tangential and normal components of velocity
				float nmag = m.vel * position.p2;
				pVector vn(position.p2 * nmag); // Normal Vn = (V.N)N
				pVector vt(m.vel - vn); // Tangent Vt = V - Vn
				
				// Compute new velocity heading out:
				// Don't apply friction if tangential velocity < cutoff
				if(vt.length2() <= cutoffSqr)
					m.vel = vt - vn * resilience;
				else
					m.vel = vt * oneMinusFriction - vn * resilience;
			}
		}
		break;
	case PDSphere:
		{
			// Sphere that particles bounce off
			// The particles are always forced out of the sphere.
			for(int i = 0; i < group->p_count; i++)
			{
				Particle &m = group->list[i];
				
				// See if particle's next position is inside domain.
				// If so, bounce it.
				pVector pnext(m.pos + m.vel * dt);
				
				if(position.Within(pnext))
				{
					// See if we were inside on previous timestep.
					bool pinside = position.Within(m.pos);
					
					// Normal to surface. This works for a sphere. Isn't
					// computed quite right, should extrapolate particle
					// position to surface.
					pVector n(m.pos - position.p1);
					n.normalize();
					
					// Compute tangential and normal components of velocity
					float nmag = m.vel * n;
					
					pVector vn(n * nmag); // Normal Vn = (V.N)N
					pVector vt = m.vel - vn; // Tangent Vt = V - Vn
					
					if(pinside)
					{
						// Previous position was inside. If normal component of
						// velocity points in, reverse it. This effectively
						// repels particles which would otherwise be trapped
						// in the sphere.
						if(nmag < 0)
							m.vel = vt - vn;
					}
					else
					{
						// Previous position was outside -> particle will cross
						// surface boundary. Reverse normal component of velocity,
						// and apply friction (if Vt >= cutoff) and resilience.
						
						// Compute new velocity heading out:
						// Don't apply friction if tangential velocity < cutoff
						if(vt.length2() <= cutoffSqr)
							m.vel = vt - vn * resilience;
						else
							m.vel = vt * oneMinusFriction - vn * resilience;
					}
				}
			}
		}
	default:
		break;
	}
}

// Set the secondary position of each particle to be its position.
void PACallActionList::Execute(ParticleGroup *group)
{
	pCallActionList(action_list_num);
}

// Set the secondary position of each particle to be its position.
void PACopyVertexB::Execute(ParticleGroup *group)
{
	int i;
	
	if(copy_pos)
	{
		for(i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			m.posB = m.pos;
		}
	}
	
	if(copy_vel)
	{
		for(i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			m.velB = m.vel;
		}
	}
}

// Dampen velocities
void PADamping::Execute(ParticleGroup *group)
{
	// This is important if dt is != 1.
	pVector one(1,1,1);
	pVector scale(one - ((one - damping) * dt));
	
	for(int i = 0; i < group->p_count; i++)
	{
		Particle &m = group->list[i];
		float vSqr = m.vel.length2();
		
		if(vSqr >= vlowSqr && vSqr <= vhighSqr)
		{
			m.vel.x *= scale.x;
			m.vel.y *= scale.y;
			m.vel.z *= scale.z;
		}
	}
}

// Exert force on each particle away from explosion center
void PAExplosion::Execute(ParticleGroup *group)
{
	float radius = velocity * age;
	float magdt = magnitude * dt;
	float oneOverSigma = 1.0f / stdev;
	float inexp = -0.5f*fsqr(oneOverSigma);
	float outexp = (float)(ONEOVERSQRT2PI * oneOverSigma);
	
	for(int i = 0; i < group->p_count; i++)
	{
		Particle &m = group->list[i];
		
		// Figure direction to particle.
		pVector dir(m.pos - center);
		float distSqr = dir.length2();
		float dist = sqrtf(distSqr);
		float DistFromWaveSqr = fsqr(radius - dist);
		
		float Gd = (float)(exp(DistFromWaveSqr * inexp) * outexp);
		
		m.vel += dir * (Gd * magdt / (dist * (distSqr + epsilon)));
	}
	
	age += dt;
}

// Follow the next particle in the list
void PAFollow::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = max_radius * max_radius;
	
	if(max_radiusSqr < P_MAXFLOAT)
	{
		for(int i = 0; i < group->p_count - 1; i++)
		{
			Particle &m = group->list[i];
			
			// Accelerate toward the particle after me in the list.
			pVector tohim(group->list[i+1].pos - m.pos); // tohim = p1 - p0
			float tohimlenSqr = tohim.length2();
			
			if(tohimlenSqr < max_radiusSqr)
			{
				// Compute force exerted between the two bodies
				m.vel += tohim * (magdt / (sqrtf(tohimlenSqr) * (tohimlenSqr + epsilon)));
			}
		}
	}
	else
	{
		for(int i = 0; i < group->p_count - 1; i++)
		{
			Particle &m = group->list[i];
			
			// Accelerate toward the particle after me in the list.
			pVector tohim(group->list[i+1].pos - m.pos); // tohim = p1 - p0
			float tohimlenSqr = tohim.length2();
			
			// Compute force exerted between the two bodies
			m.vel += tohim * (magdt / (sqrtf(tohimlenSqr) * (tohimlenSqr + epsilon)));
		}
	}
}

// Inter-particle gravitation
void PAGravitate::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = max_radius * max_radius;
	
	if(max_radiusSqr < P_MAXFLOAT)
	{
		for(int i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			
			// Add interactions with other particles
			for(int j = i + 1; j < group->p_count; j++)
			{
				Particle &mj = group->list[j];
				
				pVector tohim(mj.pos - m.pos); // tohim = p1 - p0
				float tohimlenSqr = tohim.length2();
				
				if(tohimlenSqr < max_radiusSqr)
				{
					// Compute force exerted between the two bodies
					pVector acc(tohim * (magdt / (sqrtf(tohimlenSqr) * (tohimlenSqr + epsilon))));
					m.vel += acc;
					mj.vel -= acc;
				}
			}
		}
	}
	else
	{
		for(int i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			
			// Add interactions with other particles
			for(int j = i + 1; j < group->p_count; j++)
			{
				Particle &mj = group->list[j];
				
				pVector tohim(mj.pos - m.pos); // tohim = p1 - p0
				float tohimlenSqr = tohim.length2();
				
				// Compute force exerted between the two bodies
				pVector acc(tohim * (magdt / (sqrtf(tohimlenSqr) * (tohimlenSqr + epsilon))));
				
				m.vel += acc;
				mj.vel -= acc;
			}
		}
	}
}

// Acceleration in a constant direction
void PAGravity::Execute(ParticleGroup *group)
{
	pVector ddir(direction * dt);
	
	for(int i = 0; i < group->p_count; i++)
	{
		// Step velocity with acceleration
		group->list[i].vel += ddir;
	}
}

// Accelerate particles along a line
void PAJet::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = max_radius * max_radius;
	
	if(max_radiusSqr < P_MAXFLOAT)
	{
		for(int i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			
			// Figure direction to particle.
			pVector dir(m.pos - center);
			
			// Distance to jet (force drops as 1/r^2)
			// Soften by epsilon to avoid tight encounters to infinity
			float rSqr = dir.length2();
			
			if(rSqr < max_radiusSqr)
			{
				pVector accel;
				acc.Generate(accel);
				
				// Step velocity with acceleration
				m.vel += accel * (magdt / (rSqr + epsilon));
			}
		}
	}
	else
	{
		for(int i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			
			// Figure direction to particle.
			pVector dir(m.pos - center);
			
			// Distance to jet (force drops as 1/r^2)
			// Soften by epsilon to avoid tight encounters to infinity
			float rSqr = dir.length2();
			
			pVector accel;
			acc.Generate(accel);
			
			// Step velocity with acceleration
			m.vel += accel * (magdt / (rSqr + epsilon));
		}
	}
}

// Get rid of older particles
void PAKillOld::Execute(ParticleGroup *group)
{
	// Must traverse list in reverse order so Remove will work
	for(int i = group->p_count-1; i >= 0; i--)
	{
		Particle &m = group->list[i];
		
		if(!((m.age < age_limit) ^ kill_less_than))
			group->Remove(i);
	}
}

// Get rid of slower particles
void PAKillSlow::Execute(ParticleGroup *group)
{
	// Must traverse list in reverse order so Remove will work
	for (int i = group->p_count-1; i >= 0; i--) {
		Particle &m = group->list[i];
		
		if ((m.vel.length2() < speedLimitSqr) == kill_less_than)
			group->Remove(i);
	}
}

// Match velocity to near neighbors
void PAMatchVelocity::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = max_radius * max_radius;
	
	if(max_radiusSqr < P_MAXFLOAT)
	{
		for(int i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			
			// Add interactions with other particles
			for(int j = i + 1; j < group->p_count; j++)
			{
				Particle &mj = group->list[j];
				
				pVector tohim(mj.pos - m.pos); // tohim = p1 - p0
				float tohimlenSqr = tohim.length2();
				
				if(tohimlenSqr < max_radiusSqr)
				{
					// Compute force exerted between the two bodies
					pVector acc(mj.vel * (magdt / (tohimlenSqr + epsilon)));
					
					m.vel += acc;
					mj.vel -= acc;
				}
			}
		}
	}
	else
	{
		for(int i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			
			// Add interactions with other particles
			for(int j = i + 1; j < group->p_count; j++)
			{
				Particle &mj = group->list[j];
				
				pVector tohim(mj.pos - m.pos); // tohim = p1 - p0
				float tohimlenSqr = tohim.length2();
				
				// Compute force exerted between the two bodies
				pVector acc(mj.vel * (magdt / (tohimlenSqr + epsilon)));
				
				m.vel += acc;
				mj.vel -= acc;
			}
		}
	}
}

void PAMove::Execute(ParticleGroup *group)
{
	// Step particle positions forward by dt, and age the particles.
	for(int i = 0; i < group->p_count; i++)
	{
		Particle &m = group->list[i];
		
		m.age += dt;
		m.pos += m.vel * dt;
	}
}

// Accelerate particles towards a line
void PAOrbitLine::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = max_radius * max_radius;
	
	if(max_radiusSqr < P_MAXFLOAT)
	{
		for(int i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			
			// Figure direction to particle from base of line.
			pVector f(m.pos - p);
			
			pVector w(axis * (f * axis));
			
			// Direction from particle to nearest point on line.
			pVector into = w - f;
			
			// Distance to line (force drops as 1/r^2, normalize by 1/r)
			// Soften by epsilon to avoid tight encounters to infinity
			float rSqr = into.length2();
			
			if(rSqr < max_radiusSqr)
				// Step velocity with acceleration
				m.vel += into * (magdt / (sqrtf(rSqr) + (rSqr + epsilon)));
		}
	}
	else
	{
		// Removed because it causes pipeline stalls.
		for(int i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			
			// Figure direction to particle from base of line.
			pVector f(m.pos - p);
			
			pVector w(axis * (f * axis));
			
			// Direction from particle to nearest point on line.
			pVector into = w - f;
			
			// Distance to line (force drops as 1/r^2, normalize by 1/r)
			// Soften by epsilon to avoid tight encounters to infinity
			float rSqr = into.length2();
			
			// Step velocity with acceleration
			m.vel += into * (magdt / (sqrtf(rSqr) + (rSqr + epsilon)));
		}
	}
}

// Accelerate particles towards a point
void PAOrbitPoint::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = max_radius * max_radius;

	if(max_radiusSqr < P_MAXFLOAT)
	{
		for(int i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			
			// Figure direction to particle.
			pVector dir(center - m.pos);
			
			// Distance to gravity well (force drops as 1/r^2, normalize by 1/r)
			// Soften by epsilon to avoid tight encounters to infinity
			float rSqr = dir.length2();
			
			// Step velocity with acceleration
			if(rSqr < max_radiusSqr)
				m.vel += dir * (magdt / (sqrtf(rSqr) + (rSqr + epsilon)));
		}
	}
	else
	{
		// Avoids pipeline stalls.
		for(int i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			
			// Figure direction to particle.
			pVector dir(center - m.pos);
			
			// Distance to gravity well (force drops as 1/r^2, normalize by 1/r)
			// Soften by epsilon to avoid tight encounters to infinity
			float rSqr = dir.length2();
			
			// Step velocity with acceleration
			m.vel += dir * (magdt / (sqrtf(rSqr) + (rSqr + epsilon)));
		}
	}
}

// Accelerate in random direction each time step
void PARandomAccel::Execute(ParticleGroup *group)
{
	for(int i = 0; i < group->p_count; i++)
	{
		Particle &m = group->list[i];
		
		pVector acceleration;
		gen_acc.Generate(acceleration);
		
		// dt will affect this by making a higher probability of
		// being near the original velocity after unit time. Smaller
		// dt approach a normal distribution instead of a square wave.
		m.vel += acceleration * dt;
	}
}

// Immediately displace position randomly
void PARandomDisplace::Execute(ParticleGroup *group)
{
	for(int i = 0; i < group->p_count; i++)
	{
		Particle &m = group->list[i];
		
		pVector displacement;
		gen_disp.Generate(displacement);
		
		// dt will affect this by making a higher probability of
		// being near the original position after unit time. Smaller
		// dt approach a normal distribution instead of a square wave.
		m.pos += displacement * dt;
	}
}

// Immediately assign a random velocity
void PARandomVelocity::Execute(ParticleGroup *group)
{
	for(int i = 0; i < group->p_count; i++)
	{
		Particle &m = group->list[i];
		
		pVector velocity;
		gen_vel.Generate(velocity);
		
		// Shouldn't multiply by dt because velocities are
		// invariant of dt. How should dt affect this?
		m.vel = velocity;
	}
}

#if 0
// Produce coefficients of a velocity function v(t)=at^2 + bt + c
// satisfying initial x(0)=x0,v(0)=v0 and desired x(t)=xf,v(t)=vf,
// where x = x(0) + integrate(v(T),0,t)
static inline void _pconstrain(float x0, float v0, float xf, float vf,
							   float t, float *a, float *b, float *c)
{
	*c = v0;
	*b = 2 * (-t*vf - 2*t*v0 + 3*xf - 3*x0) / (t * t);
	*a = 3 * (t*vf + t*v0 - 2*xf + 2*x0) / (t * t * t);
}
#endif

// Over time, restore particles to initial positions
// Put all particles on the surface of a statue, explode the statue,
// and then suck the particles back to the original position. Cool!
void PARestore::Execute(ParticleGroup *group)
{
	if(time_left <= 0)
	{
		for(int i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			
			// Already constrained, keep it there.
			m.pos = m.posB;
			m.vel = pVector(0,0,0);
		}
	}
	else
	{
		float t = time_left;
		float dtSqr = dt * dt;
		float tSqrInv2dt = dt * 2.0f / (t * t);
		float tCubInv3dtSqr = dtSqr * 3.0f / (t * t * t);
		
		for(int i = 0; i < group->p_count; i++)
		{
#if 1
			Particle &m = group->list[i];
			
			// Solve for a desired-behavior velocity function in each axis
			// _pconstrain(m.pos.x, m.vel.x, m.posB.x, 0., timeLeft, &a, &b, &c);
			
			// Figure new velocity at next timestep
			// m.vel.x = a * dtSqr + b * dt + c;
			
			float b = (-2*t*m.vel.x + 3*m.posB.x - 3*m.pos.x) * tSqrInv2dt;
			float a = (t*m.vel.x - m.posB.x - m.posB.x + m.pos.x + m.pos.x) * tCubInv3dtSqr;
			
			// Figure new velocity at next timestep
			m.vel.x += a + b;
			
			b = (-2*t*m.vel.y + 3*m.posB.y - 3*m.pos.y) * tSqrInv2dt;
			a = (t*m.vel.y - m.posB.y - m.posB.y + m.pos.y + m.pos.y) * tCubInv3dtSqr;
			
			// Figure new velocity at next timestep
			m.vel.y += a + b;
			
			b = (-2*t*m.vel.z + 3*m.posB.z - 3*m.pos.z) * tSqrInv2dt;
			a = (t*m.vel.z - m.posB.z - m.posB.z + m.pos.z + m.pos.z) * tCubInv3dtSqr;
			
			// Figure new velocity at next timestep
			m.vel.z += a + b;
#else
			Particle &m = group->list[i];
			
			// XXX Optimize this.
			// Solve for a desired-behavior velocity function in each axis
			float a, b, c; // Coefficients of velocity function needed
			
			_pconstrain(m.pos.x, m.vel.x, m.posB.x, 0.,
				timeLeft, &a, &b, &c);
			
			// Figure new velocity at next timestep
			m.vel.x = a * dtSqr + b * dt + c;
			
			_pconstrain(m.pos.y, m.vel.y, m.posB.y, 0.,
				timeLeft, &a, &b, &c);
			
			// Figure new velocity at next timestep
			m.vel.y = a * dtSqr + b * dt + c;
			
			_pconstrain(m.pos.z, m.vel.z, m.posB.z, 0.,
				timeLeft, &a, &b, &c);
			
			// Figure new velocity at next timestep
			m.vel.z = a * dtSqr + b * dt + c;
			
#endif
		}
	}
	
	time_left -= dt;
}

// Kill particles with positions on wrong side of the specified domain
void PASink::Execute(ParticleGroup *group)
{
	// Must traverse list in reverse order so Remove will work
	for(int i = group->p_count-1; i >= 0; i--)
	{
		Particle &m = group->list[i];
		
		// Remove if inside/outside flag matches object's flag
		if(!(position.Within(m.pos) ^ kill_inside))
			group->Remove(i);
	}
}

// Kill particles with velocities on wrong side of the specified domain
void PASinkVelocity::Execute(ParticleGroup *group)
{
	// Must traverse list in reverse order so Remove will work
	for(int i = group->p_count-1; i >= 0; i--)
	{
		Particle &m = group->list[i];
		
		// Remove if inside/outside flag matches object's flag
		if(!(velocity.Within(m.vel) ^ kill_inside))
			group->Remove(i);
	}
}

// Randomly add particles to the system
void PASource::Execute(ParticleGroup *group)
{
	int rate = int(floor(particle_rate * dt));
	
	// Dither the fraction particle in time.
	if(drand48() < particle_rate * dt - float(rate))
		rate++;
	
	// Don't emit more than it can hold.
	if(group->p_count + rate > group->max_particles)
		rate = group->max_particles - group->p_count;
	
	pVector pos, posB, vel, col, siz;
	
	if(vertexB_tracks)
	{
		for(int i = 0; i < rate; i++)
		{
			position.Generate(pos);
			size.Generate(siz);
			velocity.Generate(vel);
			color.Generate(col);
			float ag = age + NRand(age_sigma);

			/* GG take the velocity into account if there are more
			   particles generated in one go */
			pos.x = pos.x + vel.x*i*dt/(float)rate;
			pos.y = pos.y + vel.y*i*dt/(float)rate;
			pos.z = pos.z + vel.z*i*dt/(float)rate;

			group->Add(pos, pos, siz, vel, col, alpha, ag);
		}
	}
	else
	{
		for(int i = 0; i < rate; i++)
		{
			position.Generate(pos);
			positionB.Generate(posB);
			size.Generate(siz);
			velocity.Generate(vel);
			color.Generate(col);
			float ag = age + NRand(age_sigma);
			
			group->Add(pos, posB, siz, vel, col, alpha, ag);
		}
	}
}

void PASpeedLimit::Execute(ParticleGroup *group)
{
	float min_sqr = min_speed*min_speed;
	float max_sqr = max_speed*max_speed;
	
	for(int i = 0; i < group->p_count; i++)
	{
		Particle &m = group->list[i];
		float sSqr = m.vel.length2();
		if(sSqr<min_sqr && sSqr)
		{
			float s = sqrtf(sSqr);
			m.vel *= (min_speed/s);
		}
		else if(sSqr>max_sqr)
		{
			float s = sqrtf(sSqr);
			m.vel *= (max_speed/s);
		}
	}
}

// Change color of all particles toward the specified color
void PATargetColor::Execute(ParticleGroup *group)
{
	float scaleFac = scale * dt;
	
	for(int i = 0; i < group->p_count; i++)
	{
		Particle &m = group->list[i];
		m.color += (color - m.color) * scaleFac;
		m.alpha += (alpha - m.alpha) * scaleFac;
	}
}

// Change sizes of all particles toward the specified size
void PATargetSize::Execute(ParticleGroup *group)
{
	float scaleFac_x = scale.x * dt;
	float scaleFac_y = scale.y * dt;
	float scaleFac_z = scale.z * dt;
	
	for(int i = 0; i < group->p_count; i++)
	{
		Particle &m = group->list[i];
		pVector dif(size - m.size);
		dif.x *= scaleFac_x;
		dif.y *= scaleFac_y;
		dif.z *= scaleFac_z;
		m.size += dif;
	}
}

// Change velocity of all particles toward the specified velocity
void PATargetVelocity::Execute(ParticleGroup *group)
{
	float scaleFac = scale * dt;
	
	for(int i = 0; i < group->p_count; i++)
	{
		Particle &m = group->list[i];
		m.vel += (velocity - m.vel) * scaleFac;
	}
}

// Immediately displace position using vortex
// Vortex tip at center, around axis, with magnitude
// and tightness exponent
void PAVortex::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = max_radius * max_radius;
	
	if(max_radiusSqr < P_MAXFLOAT)
	{
		for(int i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			
			// Vector from tip of vortex
			pVector offset(m.pos - center);
			
			// Compute distance from particle to tip of vortex.
			float rSqr = offset.length2();
			
			// Don't do anything to particle if too close or too far.
			if(rSqr > max_radiusSqr)
				continue;
			
			float r = sqrtf(rSqr);
			
			// Compute normalized offset vector.
			pVector offnorm(offset / r);
			
			// Construct orthogonal vector frame in which to rotate
			// transformed point around origin
			float axisProj = offnorm * axis; // offnorm . axis
			
			// Components of offset perpendicular and parallel to axis
			pVector w(axis * axisProj); // parallel component
			pVector u(offnorm - w); // perpendicular component
			
			// Perpendicular component completing frame:
			pVector v(axis ^ u);
			
			// Figure amount of rotation
			// Resultant is (cos theta) u + (sin theta) v
			float theta = magdt / (rSqr + epsilon);
			float s = sinf(theta);
			float c = cosf(theta);
			
			offset = (u * c + v * s + w) * r;
			
			// Translate back to object space
			m.pos = offset + center;
		}
	}
	else
	{
		for(int i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			
			// Vector from tip of vortex
			pVector offset(m.pos - center);
			
			// Compute distance from particle to tip of vortex.
			float rSqr = offset.length2();
			
			float r = sqrtf(rSqr);
			
			// Compute normalized offset vector.
			pVector offnorm(offset / r);
			
			// Construct orthogonal vector frame in which to rotate
			// transformed point around origin
			float axisProj = offnorm * axis; // offnorm . axis
			
			// Components of offset perpendicular and parallel to axis
			pVector w(axis * axisProj); // parallel component
			pVector u(offnorm - w); // perpendicular component
			
			// Perpendicular component completing frame:
			pVector v(axis ^ u);
			
			// Figure amount of rotation
			// Resultant is (cos theta) u + (sin theta) v
			float theta = magdt / (rSqr + epsilon);
			float s = sinf(theta);
			float c = cosf(theta);
			
			offset = (u * c + v * s + w) * r;
			
			// Translate back to object space
			m.pos = offset + center;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Stuff for the pDomain.

pDomain::pDomain(PDomainEnum dtype, float a0, float a1,
				 float a2, float a3, float a4, float a5,
				 float a6, float a7, float a8)
{
	type = dtype;
	switch(type)
	{
	case PDPoint:
		p1 = pVector(a0, a1, a2);
		break;
	case PDLine:
		{
			p1 = pVector(a0, a1, a2);
			pVector tmp(a3, a4, a5);
			// p2 is vector from p1 to other endpoint.
			p2 = tmp - p1;
		}
		break;
	case PDBox:
		// p1 is the min corner. p2 is the max corner.
		if(a0 < a3)
		{
			p1.x = a0; p2.x = a3;
		}
		else
		{
			p1.x = a3; p2.x = a0;
		}
		if(a1 < a4)
		{
			p1.y = a1; p2.y = a4;
		}
		else
		{
			p1.y = a4; p2.y = a1;
		}
		if(a2 < a5)
		{
			p1.z = a2; p2.z = a5;
		}
		else
		{
			p1.z = a5; p2.z = a2;
		}
		break;
	case PDTriangle:
		{
			p1 = pVector(a0, a1, a2);
			pVector tp2 = pVector(a3, a4, a5);
			pVector tp3 = pVector(a6, a7, a8);
			
			u = tp2 - p1;
			v = tp3 - p1;
			
			// The rest of this is needed for bouncing.
			radius1Sqr = u.length();
			pVector tu = u / radius1Sqr;
			radius2Sqr = v.length();
			pVector tv = v / radius2Sqr;
			
			p2 = tu ^ tv; // This is the non-unit normal.
			p2.normalize(); // Must normalize it.
			
			// radius1 stores the d of the plane eqn.
			radius1 = -(p1 * p2);
		}
		break;
	case PDRectangle:
		{
			p1 = pVector(a0, a1, a2);
			u = pVector(a3, a4, a5);
			v = pVector(a6, a7, a8);
			
			// The rest of this is needed for bouncing.
			radius1Sqr = u.length();
			pVector tu = u / radius1Sqr;
			radius2Sqr = v.length();
			pVector tv = v / radius2Sqr;
			
			p2 = tu ^ tv; // This is the non-unit normal.
			p2.normalize(); // Must normalize it.
			
			// radius1 stores the d of the plane eqn.
			radius1 = -(p1 * p2);
		}
		break;
	case PDPlane:
		{
			p1 = pVector(a0, a1, a2);
			p2 = pVector(a3, a4, a5);
			p2.normalize(); // Must normalize it.
			
			// radius1 stores the d of the plane eqn.
			radius1 = -(p1 * p2);
		}
		break;
	case PDSphere:
		p1 = pVector(a0, a1, a2);
		if(a3 > a4)
		{
			radius1 = a3; radius2 = a4;
		}
		else
		{
			radius1 = a4; radius2 = a3;
		}
		radius1Sqr = radius1 * radius1;
		radius2Sqr = radius2 * radius2;
		break;
	case PDCone:
	case PDCylinder:
		{
			// p2 is a vector from p1 to the other end of cylinder.
			// p1 is apex of cone.
			
			p1 = pVector(a0, a1, a2);
			pVector tmp(a3, a4, a5);
			p2 = tmp - p1;
			
			if(a6 > a7)
			{
				radius1 = a6; radius2 = a7;
			}
			else
			{
				radius1 = a7; radius2 = a6;
			}
			radius1Sqr = fsqr(radius1);
			
			// Given an arbitrary nonzero vector n, make two orthonormal
			// vectors u and v forming a frame [u,v,n.normalize()].
			pVector n = p2;
			float p2l2 = n.length2(); // Optimize this.
			n.normalize();
			
			// radius2Sqr stores 1 / (p2.p2)
			// XXX Used to have an actual if.
			radius2Sqr = p2l2 ? 1.0f / p2l2 : 0.0f;
			
			// Find a vector orthogonal to n.
			pVector basis(1.0f, 0.0f, 0.0f);
			if(fabs(basis * n) > 0.999)
				basis = pVector(0.0f, 1.0f, 0.0f);
			
			// Project away N component, normalize and cross to get
			// second orthonormal vector.
			u = basis - n * (basis * n);
			u.normalize();
			v = n ^ u;
		}
		break;
	case PDBlob:
		{
			p1 = pVector(a0, a1, a2);
			radius1 = a3;
			float tmp = (float)(1./radius1);
			radius2Sqr = -0.5f*fsqr(tmp);
			radius2 = (float)(ONEOVERSQRT2PI * tmp);
		}
		break;
	case PDDisc:
		{
			p1 = pVector(a0, a1, a2); // Center point
			p2 = pVector(a3, a4, a5); // Normal (not used in Within and Generate)
			p2.normalize();
			
			if(a6 > a7)
			{
				radius1 = a6; radius2 = a7;
			}
			else
			{
				radius1 = a7; radius2 = a6;
			}
			
			// Find a vector orthogonal to n.
			pVector basis(1.0f, 0.0f, 0.0f);
			if(fabs(basis * p2) > 0.999)
				basis = pVector(0.0f, 1.0f, 0.0f);
			
			// Project away N component, normalize and cross to get
			// second orthonormal vector.
			u = basis - p2 * (basis * p2);
			u.normalize();
			v = p2 ^ u;
			radius1Sqr = -(p1 * p2); // D of the plane eqn.
		}
		break;
	}
}

// Determines if pos is inside the domain
bool pDomain::Within(const pVector &pos) const
{
	switch (type)
	{
	case PDBox:
		return !((pos.x < p1.x) || (pos.x > p2.x) ||
			(pos.y < p1.y) || (pos.y > p2.y) ||
			(pos.z < p1.z) || (pos.z > p2.z));
	case PDPlane:
		// Distance from plane = n * p + d
		// Inside is the positive half-space.
		return pos * p2 >= -radius1;
	case PDSphere:
		{
			pVector rvec(pos - p1);
			float rSqr = rvec.length2();
			return rSqr <= radius1Sqr && rSqr >= radius2Sqr;
		}
	case PDCylinder:
	case PDCone:
		{
			// This is painful and slow. Might be better to do quick
			// accept/reject tests.
			// Let p2 = vector from base to tip of the cylinder
			// x = vector from base to test point
			// x . p2
			// dist = ------ = projected distance of x along the axis
			// p2. p2 ranging from 0 (base) to 1 (tip)
			//
			// rad = x - dist * p2 = projected vector of x along the base
			// p1 is the apex of the cone.
			
			pVector x(pos - p1);
			
			// Check axial distance
			// radius2Sqr stores 1 / (p2.p2)
			float dist = (p2 * x) * radius2Sqr;
			if(dist < 0.0f || dist > 1.0f)
				return false;
			
			// Check radial distance; scale radius along axis for cones
			pVector xrad = x - p2 * dist; // Radial component of x
			float rSqr = xrad.length2();
			
			if(type == PDCone)
				return (rSqr <= fsqr(dist * radius1) &&
				rSqr >= fsqr(dist * radius2));
			else
				return (rSqr <= radius1Sqr && rSqr >= fsqr(radius2));
		}
	case PDBlob:
		{
			pVector x(pos - p1);
			// return exp(-0.5 * xSq * Sqr(oneOverSigma)) * ONEOVERSQRT2PI * oneOverSigma;
			float Gx = expf(x.length2() * radius2Sqr) * radius2;
			return (drand48() < Gx);
		}
	case PDPoint:
	case PDLine:
	case PDRectangle:
	case PDTriangle:
	case PDDisc:
	default:
		return false; // XXX Is there something better?
	}
}

// Generate a random point uniformly distrbuted within the domain
void pDomain::Generate(pVector &pos) const
{
	switch (type)
	{
	case PDPoint:
		pos = p1;
		break;
	case PDLine:
		pos = p1 + p2 * drand48();
		break;
	case PDBox:
		// Scale and translate [0,1] random to fit box
		pos.x = p1.x + (p2.x - p1.x) * drand48();
		pos.y = p1.y + (p2.y - p1.y) * drand48();
		pos.z = p1.z + (p2.z - p1.z) * drand48();
		break;
	case PDTriangle:
		{
			float r1 = drand48();
			float r2 = drand48();
			if(r1 + r2 < 1.0f)
				pos = p1 + u * r1 + v * r2;
			else
				pos = p1 + u * (1.0f-r1) + v * (1.0f-r2);
		}
		break;
	case PDRectangle:
		pos = p1 + u * drand48() + v * drand48();
		break;
	case PDPlane: // How do I sensibly make a point on an infinite plane?
		pos = p1;
		break;
	case PDSphere:
		// Place on [-1..1] sphere
		pos = RandVec() - vHalf;
		pos.normalize();
		
		// Scale unit sphere pos by [0..r] and translate
		// (should distribute as r^2 law)
		if(radius1 == radius2)
			pos = p1 + pos * radius1;
		else
			pos = p1 + pos * (radius2 + drand48() * (radius1 - radius2));
		break;
	case PDCylinder:
	case PDCone:
		{
			// For a cone, p2 is the apex of the cone.
			float dist = drand48(); // Distance between base and tip
			float theta = drand48() * 2.0f * float(M_PI); // Angle around axis
			// Distance from axis
			float r = radius2 + drand48() * (radius1 - radius2);
			
			float x = r * cosf(theta); // Weighting of each frame vector
			float y = r * sinf(theta);
			
			// Scale radius along axis for cones
			if(type == PDCone)
			{
				x *= dist;
				y *= dist;
			}
			
			pos = p1 + p2 * dist + u * x + v * y;
		}
		break;
	case PDBlob:
		pos.x = p1.x + NRand(radius1);
		pos.y = p1.y + NRand(radius1);
		pos.z = p1.z + NRand(radius1);
		
		break;
	case PDDisc:
		{
			float theta = drand48() * 2.0f * float(M_PI); // Angle around normal
			// Distance from center
			float r = radius2 + drand48() * (radius1 - radius2);
			
			float x = r * cosf(theta); // Weighting of each frame vector
			float y = r * sinf(theta);
			
			pos = p1 + u * x + v * y;
		}
		break;
	default:
		pos = pVector(0,0,0);
	}
}
