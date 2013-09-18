/*

    boids2d 08/2005 a.sier / jasch adapted from boids by eric singer  1995-2003 eric l. singer
    
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include     "m_pd.h"
#include    <stdlib.h>
#include    <math.h>

// constants
#define            kAssistInlet    1
#define            kAssistOutlet    2
#define            kMaxLong        0xFFFFFFFF
#define            kMaxNeighbors    4

// util
#define MAX(a,b) ((a)>(b)?(a):(b))
#define CLIP(x,a,b) (x)=(x)<(a)?(a):(x)>(b)?(b):(x)

// initial flight parameters
const short			kNumBoids        = 12;	// number of boids
const short			kNumNeighbors    = 2;    // must be <= kMaxNeighbors
const double		kMinSpeed        = 0.15;	// boids' minimum speed
const double        kMaxSpeed        = 0.25;    // boids' maximum speed
const double        kCenterWeight    = 0.25;    // flock centering
const double        kAttractWeight    = 0.300;// attraction point seeking
const double        kMatchWeight    = 0.100;// neighbors velocity matching
const double        kAvoidWeight    = 0.10;    // neighbors avoidance
const double        kWallsWeight    = 0.500;// wall avoidance [210]
const double        kEdgeDist        = 0.5;    // vision distance to avoid wall edges [5]
const double        kSpeedupFactor    = 0.100;// alter animation speed
const double        kInertiaFactor    = 0.20;    // willingness to change speed & direction
const double        kAccelFactor    = 0.100;// neighbor avoidance accelerate or decelerate rate
const double        kPrefDist        = 0.25;    // preferred distance from neighbors
const double        kFlyRectTop        = 1.0;    // fly rect boundaries
const double		kFlyRectLeft    = -1.0;    
const double		kFlyRectBottom    = -1.0;    
const double		kFlyRectRight    = 1.0;    
// const double        kFlyRectFront    = 1.0;    
// const double        kFlyRectBack    = -1.0;    


// typedefs
typedef struct Velocity {
    double        x;
    double        y;
//    double        z;
} Velocity;

typedef struct Point2d {
    double        x;
    double        y;
//    double        z;
} Point2d;

typedef struct Box3D {
    double        left, right;
    double        top, bottom;
//    double        front, back;
} Box3D;

typedef struct _Boid {
    Point2d        oldPos;
    Point2d        newPos;
    Velocity    oldDir;
    Velocity    newDir;
    double        speed;
    short        neighbor[kMaxNeighbors];
    double        neighborDistSqr[kMaxNeighbors];
} t_one_boid, *BoidPtr;

typedef struct _FlockObject {
	t_object	ob;
    void		*out1, *out2;
    short		mode;
    long		numBoids;
    long		numNeighbors;
    Box3D		flyRect;
    double		minSpeed;
    double		maxSpeed;
    double		centerWeight;
    double        attractWeight;
    double        matchWeight;
    double        avoidWeight;
    double        wallsWeight;
    double        edgeDist;
    double        speedupFactor;
    double        inertiaFactor;
    double        accelFactor;
    double        prefDist;
    double        prefDistSqr;
    Point2d        centerPt;
    Point2d        attractPt;
    BoidPtr        boid;
    double         d2r, r2d;
} t_boids, *FlockPtr;

// variables
// void         *flock;
t_symbol     *ps_nothing;

// prototypes
void *boids2d_class;
void *Flock_new(t_symbol *s, long argc, t_atom *argv);
void Flock_free(t_boids *x);
void Flock_bang(t_boids *x);
void Flock_dump(t_boids *x);
void Flock_mode(t_boids *x, t_float arg);
void Flock_numNeighbors(t_boids *x, t_float arg);
void Flock_numBoids(t_boids *x, t_float arg);
void Flock_minSpeed(t_boids *x, t_float arg);
void Flock_maxSpeed(t_boids *x, t_float arg);
void Flock_centerWeight(t_boids *x, t_float arg);
void Flock_attractWeight(t_boids *x, t_float arg);
void Flock_matchWeight(t_boids *x, t_float arg);
void Flock_avoidWeight(t_boids *x, t_float arg);
void Flock_wallsWeight(t_boids *x, t_float arg);
void Flock_edgeDist(t_boids *x, t_float arg);
void Flock_speedupFactor(t_boids *x, t_float arg);
void Flock_inertiaFactor(t_boids *x, t_float arg);
void Flock_accelFactor(t_boids *x, t_float arg);
void Flock_prefDist(t_boids *x, t_float arg);
void Flock_flyRect(t_boids *x, t_symbol *msg, short argc, t_atom *argv);
void Flock_attractPt(t_boids *x, t_symbol *msg, short argc, t_atom *argv);
void Flock_reset(t_boids *x);
void Flock_resetBoids(t_boids *x);
void InitFlock(t_boids *x);
void FlightStep(t_boids *x);
Point2d FindFlockCenter(t_boids *x);
float MatchAndAvoidNeighbors(t_boids *x, short theBoid, Velocity *matchNeighborVel, Velocity *avoidNeighborVel);
Velocity SeekPoint(t_boids *x, short theBoid, Point2d seekPt);
Velocity AvoidWalls(t_boids *x, short theBoid);
int InFront(BoidPtr theBoid, BoidPtr neighbor);
void NormalizeVelocity(Velocity *direction);
double RandomInt(double minRange, double maxRange);
double DistSqrToPt(Point2d firstPoint, Point2d secondPoint);

void boids2d_setup(void)
{
    boids2d_class = class_new(gensym("boids2d"), (t_newmethod)Flock_new, 
            (t_method)Flock_free, sizeof(t_boids), 0, A_GIMME, 0);
    /*    setup((t_messlist **) &flock, (method)Flock_new, (method)Flock_free, (short) sizeof(FlockObject), 0L, A_LONG, A_DEFLONG, 0); */
    class_addfloat(boids2d_class,  (t_method) Flock_numBoids);
    class_addbang(boids2d_class,   (t_method) Flock_bang);
    class_addmethod(boids2d_class, (t_method) Flock_numNeighbors,        gensym("neighbors"),     A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_numBoids,            gensym("number"),         A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_mode,                gensym("mode"),         A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_minSpeed,            gensym("minspeed"),     A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_maxSpeed,            gensym("maxspeed"),     A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_centerWeight,        gensym("center"),         A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_attractWeight,        gensym("attract"),         A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_matchWeight,        gensym("match"),         A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_avoidWeight,        gensym("avoid"),         A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_wallsWeight,        gensym("repel"),         A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_edgeDist,            gensym("edgedist"),     A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_speedupFactor,        gensym("speed"),         A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_inertiaFactor,        gensym("inertia"),         A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_accelFactor,        gensym("accel"),         A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_prefDist,            gensym("prefdist"),     A_FLOAT, 0);
    class_addmethod(boids2d_class, (t_method) Flock_flyRect,            gensym("flyrect"),         A_GIMME, 0);
    class_addmethod(boids2d_class, (t_method) Flock_attractPt,             gensym("attractpt"),     A_GIMME, 0);
    class_addmethod(boids2d_class, (t_method) Flock_resetBoids,         gensym("reset"),         0);
    class_addmethod(boids2d_class, (t_method) Flock_reset,                 gensym("init"),         0);
    class_addmethod(boids2d_class, (t_method) Flock_dump,                 gensym("dump"),         0);
    
    logpost(NULL, 4, "boids2d 2005-2006 a.sier / jasch    1995-2003 eric l. singer   "__DATE__" "__TIME__);    
    ps_nothing = gensym("");
}


void *Flock_new(t_symbol *s, long argc, t_atom *argv)
{    
    t_boids *x = (t_boids *)pd_new(boids2d_class);
    x->out1 = outlet_new(&x->ob, NULL);
    x->out2 = outlet_new(&x->ob, NULL);
    
    x->numBoids = 16;
    if((argc >= 1) && (argv[0].a_type == A_FLOAT)){
        x->numBoids = argv[0].a_w.w_float;
    }
    x->boid = (t_one_boid *)malloc(sizeof(t_one_boid) * x->numBoids);
    
    InitFlock(x);
    
    x->mode = 0;
    if((argc >= 2) && (argv[1].a_type == A_FLOAT)){
        x->mode = (short)(CLIP(argv[1].a_w.w_float, 0, 2));
    }
    
    x->d2r = 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117068/180.0;
    x->r2d = 180.0/3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117068;
    
    return(x);
}


void Flock_free(t_boids *x)
{
    free(x->boid);
}

void Flock_bang(t_boids *x)
{
    short    i;
    t_atom     outlist[10];
    t_atom     *out;
    
    double     tempNew_x, tempNew_y; 
    double     tempOld_x, tempOld_y;
    double    delta_x, delta_y; 
    double     azi, speed;
    // double tempspeed;
    
    out = outlist;
        
    FlightStep(x);


    switch(x->mode) { // newpos
        case 0:
        for (i = 0; i < x->numBoids; i++){
            SETFLOAT(out+0, i);
            SETFLOAT(out+1, x->boid[i].newPos.x);
            SETFLOAT(out+2, x->boid[i].newPos.y);
        //    SETFLOAT(out+3, x->boid[i].newPos.z);
            outlet_list(x->out1, 0L, 3, out);
        }
        break;
        case 1: //newpos + oldpos
        for (i = 0; i < x->numBoids; i++){
            SETFLOAT(out+0, i);
            SETFLOAT(out+1, x->boid[i].newPos.x);
            SETFLOAT(out+2, x->boid[i].newPos.y);
            // SETFLOAT(out+3, x->boid[i].newPos.z);
            SETFLOAT(out+3, x->boid[i].oldPos.x);
            SETFLOAT(out+4, x->boid[i].oldPos.y);
            // SETFLOAT(out+6, x->boid[i].oldPos.z);
            outlet_list(x->out1, 0L, 5, out);
        }
        break;
        case 2:
        for (i = 0; i < x->numBoids; i++){                        
            tempNew_x = x->boid[i].newPos.x;
            tempNew_y = x->boid[i].newPos.y;
            // tempNew_z = x->boid[i].newPos.z;
            tempOld_x = x->boid[i].oldPos.x;
            tempOld_y = x->boid[i].oldPos.y;
            // tempOld_z = x->boid[i].oldPos.z;
            delta_x = tempNew_x - tempOld_x;
            delta_y = tempNew_y - tempOld_y;
            // delta_z = tempNew_z - tempOld_z;
            azi = atan2(delta_y, delta_x) * x->r2d;
            // ele = atan2(delta_y, delta_x) * x->r2d;
            speed = sqrt(delta_x * delta_x + delta_y * delta_y);//  + delta_z * delta_z);
            SETFLOAT(out+0, i);
            SETFLOAT(out+1, tempNew_x);
            SETFLOAT(out+2, tempNew_y);
            // SETFLOAT(out+3, tempNew_z);
            SETFLOAT(out+3, tempOld_x);
            SETFLOAT(out+4, tempOld_y);
            // SETFLOAT(out+6, tempOld_z);
            SETFLOAT(out+5, speed);
            SETFLOAT(out+6, azi);
            // SETFLOAT(out+9, ele);
            outlet_list(x->out1, 0L, 7, out);
        }
        break;    
    }
}

void Flock_dump(t_boids *x)
{
    t_atom    outList[6];
    
    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->numNeighbors;
    outlet_anything(x->out2, gensym("neighbors"), 1, outList);

    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->minSpeed;
    outlet_anything(x->out2, gensym("minspeed"), 1, outList);

    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->maxSpeed;
    outlet_anything(x->out2, gensym("maxspeed"), 1, outList);

    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->centerWeight;
    outlet_anything(x->out2, gensym("center"), 1, outList);

    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->attractWeight;
    outlet_anything(x->out2, gensym("attract"), 1, outList);

    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->matchWeight;
    outlet_anything(x->out2, gensym("match"), 1, outList);

    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->avoidWeight;
    outlet_anything(x->out2, gensym("avoid"), 1, outList);

    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->wallsWeight;
    outlet_anything(x->out2, gensym("repel"), 1, outList);

    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->edgeDist;
    outlet_anything(x->out2, gensym("edgedist"), 1, outList);

    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->speedupFactor;
    outlet_anything(x->out2, gensym("speed"), 1, outList);

    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->inertiaFactor;
    outlet_anything(x->out2, gensym("inertia"), 1, outList);

    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->accelFactor;
    outlet_anything(x->out2, gensym("accel"), 1, outList);

    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float = x->prefDist;
    outlet_anything(x->out2, gensym("prefdist"), 1, outList);

    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->flyRect.left;
    outList[1].a_type = A_FLOAT;
    outList[1].a_w.w_float =  x->flyRect.top;
    outList[2].a_type = A_FLOAT;
    outList[2].a_w.w_float =  x->flyRect.right;
    outList[3].a_type = A_FLOAT;
    outList[3].a_w.w_float =  x->flyRect.bottom;
    /*outList[4].a_type = A_FLOAT;
    outList[4].a_w.w_float =  x->flyRect.front;
    outList[5].a_type = A_FLOAT;
    outList[5].a_w.w_float =  x->flyRect.back;*/
    outlet_anything(x->out2, gensym("flyrect"), 4, outList);

    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->attractPt.x;
    outList[1].a_type = A_FLOAT;
    outList[1].a_w.w_float =  x->attractPt.y;
    /*outList[2].a_type = A_FLOAT;
    outList[2].a_w.w_float =  x->attractPt.z;*/
    outlet_anything(x->out2, gensym("attractpt"), 2, outList);
    
    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->mode;
    outlet_anything(x->out2, gensym("mode"), 1, outList);
    
    outList[0].a_type = A_FLOAT;
    outList[0].a_w.w_float =  x->numBoids;
    outlet_anything(x->out2, gensym("number"), 1, outList);
}


void Flock_mode(t_boids *x, t_float arg)
{
    long m = (long)arg;
    x->mode = CLIP(m, 0, 2);
}

void Flock_numNeighbors(t_boids *x, t_float arg)
{
    x->numNeighbors = (long)arg;
}

void Flock_numBoids(t_boids *x, t_float arg)
{
    x->boid = (t_one_boid *)realloc(x->boid, sizeof(t_one_boid) * (long)arg);
    x->numBoids = (long)arg;
    Flock_resetBoids(x);
}

void Flock_minSpeed(t_boids *x, t_float arg)
{
    x->minSpeed = MAX(arg, 0.000001);
}

void Flock_maxSpeed(t_boids *x, t_float arg)
{
    x->maxSpeed = (double)arg;
}

void Flock_centerWeight(t_boids *x, t_float arg)
{
    x->centerWeight = (double)arg;
}

void Flock_attractWeight(t_boids *x, t_float arg)
{
    x->attractWeight = (double)arg;
}

void Flock_matchWeight(t_boids *x, t_float arg)
{
    x->matchWeight = (double)arg;
}

void Flock_avoidWeight(t_boids *x, t_float arg)
{
    x->avoidWeight = (double)arg;
}

void Flock_wallsWeight(t_boids *x, t_float arg)
{
    x->wallsWeight = (double)arg;
}

void Flock_edgeDist(t_boids *x, t_float arg)
{
    x->edgeDist = (double)arg;
}

void Flock_speedupFactor(t_boids *x, t_float arg)
{
    x->speedupFactor = (double)arg;
}

void Flock_inertiaFactor(t_boids *x, t_float arg)
{
    if(arg == 0){
        x->inertiaFactor = 0.000001;
    }else{
        x->inertiaFactor = (double)arg;
    }
}

void Flock_accelFactor(t_boids *x, t_float arg)
{
    x->accelFactor = (double)arg;
}

void Flock_prefDist(t_boids *x, t_float arg)
{
    x->prefDist = (double)arg;
}

void Flock_flyRect(t_boids *x, t_symbol *msg, short argc, t_atom *argv)
{
    double temp[4];
    short i;
    if(argc == 4){
        for(i=0;i<4;i++) {
            if(argv[i].a_type == A_FLOAT) {
                temp[i] = (double)argv[i].a_w.w_float;    
            }
        }
        x->flyRect.left         = temp[0];
        x->flyRect.top         = temp[1];
        x->flyRect.right     = temp[2];
        x->flyRect.bottom     = temp[3];
        // x->flyRect.front     = temp[4];
        // x->flyRect.back         = temp[5];
    }else{
        error("boids2d: flyrect needs four values");
    }
}

void Flock_attractPt(t_boids *x, t_symbol *msg, short argc, t_atom *argv)
{
    double temp[2];
    short i;
    if(argc == 2){
    for(i=0;i<2;i++) {
        if(argv[i].a_type == A_FLOAT) {
            temp[i] = (double)argv[i].a_w.w_float;    
        }
    }
    x->attractPt.x = temp[0];
    x->attractPt.y = temp[1];
    // x->attractPt.z = temp[2];
    }else{
        error("boids2d: attractPt needs two values");
    }
}

void Flock_reset(t_boids *x)
{
    InitFlock(x);
}

void Flock_resetBoids(t_boids *x)
{
    long i, j;
    double rndAngle;
    
    for (i = 0; i <  x->numBoids; i++) { // init everything to 0.0
        x->boid[i].oldPos.x = 0.0;
        x->boid[i].oldPos.y = 0.0;
        // x->boid[i].oldPos.z = 0.0;

        x->boid[i].newPos.x = 0.0;
        x->boid[i].newPos.y = 0.0;
        // x->boid[i].newPos.z = 0.0;
        
        x->boid[i].oldDir.x = 0.0;
        x->boid[i].oldDir.y = 0.0;
        // x->boid[i].oldDir.z = 0.0;
        
        x->boid[i].newDir.x = 0.0;
        x->boid[i].newDir.y = 0.0;
        // x->boid[i].newDir.z = 0.0;
        
        x->boid[i].speed = 0.0;
        
        for(j=0; j<kMaxNeighbors;j++){
            x->boid[i].neighbor[j] = 0;
            x->boid[i].neighborDistSqr[j] = 0.0;
        }
    }
    for (i = 0; i <  x->numBoids; i++) {                // set the initial locations and velocities of the boids
        x->boid[i].newPos.x = x->boid[i].oldPos.x = RandomInt(x->flyRect.right,x->flyRect.left);        // set random location within flyRect
        x->boid[i].newPos.y = x->boid[i].oldPos.y = RandomInt(x->flyRect.bottom, x->flyRect.top);
        // x->boid[i].newPos.z = x->boid[i].oldPos.z = RandomInt(x->flyRect.back, x->flyRect.front);
        rndAngle = RandomInt(0, 360) * x->d2r;        // set velocity from random angle
        x->boid[i].newDir.x = sin(rndAngle);
        x->boid[i].newDir.y = cos(rndAngle);
        // x->boid[i].newDir.z = (cos(rndAngle) + sin(rndAngle)) * 0.5;
        x->boid[i].speed = (kMaxSpeed + kMinSpeed) * 0.5;
    }
}

void InitFlock(t_boids *x)
{
    x->numNeighbors        = kNumNeighbors;
    x->minSpeed            = kMinSpeed;
    x->maxSpeed            = kMaxSpeed;
    x->centerWeight        = kCenterWeight;
    x->attractWeight        = kAttractWeight;
    x->matchWeight        = kMatchWeight;
    x->avoidWeight        = kAvoidWeight;
    x->wallsWeight        = kWallsWeight;
    x->edgeDist            = kEdgeDist;
    x->speedupFactor        = kSpeedupFactor;
    x->inertiaFactor        = kInertiaFactor;
    x->accelFactor        = kAccelFactor;
    x->prefDist            = kPrefDist;
    x->prefDistSqr        = kPrefDist * kPrefDist;
    x->flyRect.top        = kFlyRectTop;
    x->flyRect.left        = kFlyRectLeft;
    x->flyRect.bottom    = kFlyRectBottom;
    x->flyRect.right        = kFlyRectRight;
    // x->flyRect.front        = kFlyRectFront;
    // x->flyRect.back        = kFlyRectBack;
    x->attractPt.x        = (kFlyRectLeft + kFlyRectRight) * 0.5;
    x->attractPt.y        = (kFlyRectTop + kFlyRectBottom) * 0.5;
    // x->attractPt.z        = (kFlyRectFront + kFlyRectBack) * 0.5;
    Flock_resetBoids(x);
}

void FlightStep(t_boids *x)
{
    Velocity        goCenterVel;
    Velocity        goAttractVel;
    Velocity        matchNeighborVel;
    Velocity        avoidWallsVel;
    Velocity        avoidNeighborVel;
    float            avoidNeighborSpeed;
    const Velocity    zeroVel    = {0.0, 0.0};//, 0.0};
    short            i;

    x->centerPt = FindFlockCenter(x);
    for (i = 0; i <  x->numBoids; i++) {                        // save position and velocity
        x->boid[i].oldPos.x = x->boid[i].newPos.x;
        x->boid[i].oldPos.y = x->boid[i].newPos.y;
        // x->boid[i].oldPos.z = x->boid[i].newPos.z;
        
        x->boid[i].oldDir.x = x->boid[i].newDir.x;
        x->boid[i].oldDir.y = x->boid[i].newDir.y;
        // x->boid[i].oldDir.z = x->boid[i].newDir.z;
    }
    for (i = 0; i < x->numBoids; i++) {
        if (x->numNeighbors > 0) {                            // get all velocity components
            avoidNeighborSpeed = MatchAndAvoidNeighbors(x, i,&matchNeighborVel, &avoidNeighborVel);
        } else {
            matchNeighborVel = zeroVel;
            avoidNeighborVel = zeroVel;
            avoidNeighborSpeed = 0;
        }
        goCenterVel = SeekPoint(x, i, x->centerPt);            
        goAttractVel = SeekPoint(x, i, x->attractPt);
        avoidWallsVel = AvoidWalls(x, i);
    
        // compute resultant velocity using weights and inertia
        x->boid[i].newDir.x = x->inertiaFactor * (x->boid[i].oldDir.x) +
                            (x->centerWeight * goCenterVel.x +
                             x->attractWeight * goAttractVel.x +
                             x->matchWeight * matchNeighborVel.x +
                             x->avoidWeight * avoidNeighborVel.x +
                             x->wallsWeight * avoidWallsVel.x) / x->inertiaFactor;
        x->boid[i].newDir.y = x->inertiaFactor * (x->boid[i].oldDir.y) +
                            (x->centerWeight * goCenterVel.y +
                             x->attractWeight * goAttractVel.y +
                             x->matchWeight * matchNeighborVel.y +
                             x->avoidWeight * avoidNeighborVel.y +
                             x->wallsWeight * avoidWallsVel.y) / x->inertiaFactor;
        /*x->boid[i].newDir.z = x->inertiaFactor * (x->boid[i].oldDir.z) +
                            (x->centerWeight * goCenterVel.z +
                             x->attractWeight * goAttractVel.z +
                             x->matchWeight * matchNeighborVel.z +
                             x->avoidWeight * avoidNeighborVel.z +
                             x->wallsWeight * avoidWallsVel.z) / x->inertiaFactor;*/
        NormalizeVelocity(&(x->boid[i].newDir));    // normalize velocity so its length is unity

        // set to avoidNeighborSpeed bounded by minSpeed and maxSpeed
        if ((avoidNeighborSpeed >= x->minSpeed) &&
                (avoidNeighborSpeed <= x->maxSpeed))
            x->boid[i].speed = avoidNeighborSpeed;
        else if (avoidNeighborSpeed > x->maxSpeed)
            x->boid[i].speed = x->maxSpeed;
        else
            x->boid[i].speed = x->minSpeed;

        // calculate new position, applying speedupFactor
        x->boid[i].newPos.x += x->boid[i].newDir.x * x->boid[i].speed * (x->speedupFactor / 100.0);
        x->boid[i].newPos.y += x->boid[i].newDir.y * x->boid[i].speed * (x->speedupFactor / 100.0);
        // x->boid[i].newPos.z += x->boid[i].newDir.z * x->boid[i].speed * (x->speedupFactor / 100.0);

    }
}

Point2d FindFlockCenter(t_boids *x)
{
    double            totalH = 0, totalV = 0, totalD = 0;
    Point2d            centerPoint;
    short            i;

    for (i = 0 ; i <  x->numBoids; i++)
    {
        totalH += x->boid[i].oldPos.x;
        totalV += x->boid[i].oldPos.y;
        // totalD += x->boid[i].oldPos.z;
    }
    centerPoint.x = (double)(totalH / x->numBoids);
    centerPoint.y = (double)(totalV / x->numBoids);
    // centerPoint.z = (double)    (totalD / x->numBoids);
        
    return(centerPoint);
}

float MatchAndAvoidNeighbors(t_boids *x, short theBoid, Velocity *matchNeighborVel, Velocity *avoidNeighborVel)
{
    short            i, j, neighbor;
    double            distSqr;
    double            dist, distH, distV,distD;
    double            tempSpeed;
    short            numClose = 0;
    Velocity        totalVel = {0.0,0.0};//,0.0};

    /**********************/
    /* Find the neighbors */    
    /**********************/

    /* special case of one neighbor */
    if (x->numNeighbors == 1) {
        x->boid[theBoid].neighborDistSqr[0] = kMaxLong;
    
        for (i = 0; i < x->numBoids; i++) {
            if (i != theBoid) {
                distSqr = DistSqrToPt(x->boid[theBoid].oldPos, x->boid[i].oldPos);
                
                /* if this one is closer than the closest so far, then remember it */
                if (x->boid[theBoid].neighborDistSqr[0] > distSqr) {
                    x->boid[theBoid].neighborDistSqr[0] = distSqr;
                    x->boid[theBoid].neighbor[0] = i;
                }
            }
        }
    }
    /* more than one neighbor */
    else {
        for (j = 0; j < x->numNeighbors; j++)
            x->boid[theBoid].neighborDistSqr[j] = kMaxLong;
        
        for (i = 0 ; i < x->numBoids; i++) {
            /* if this one is not me... */
            if (i != theBoid) {
                distSqr = DistSqrToPt(x->boid[theBoid].oldPos, x->boid[i].oldPos);
    
                /* if distSqr is less than the distance at the bottom of the array, sort into array */
                if (distSqr < x->boid[theBoid].neighborDistSqr[x->numNeighbors-1]) {
                    j = x->numNeighbors - 1;
                
                    /* sort distSqr in to keep array in size order, smallest first */
                    while ((distSqr < x->boid[theBoid].neighborDistSqr[j-1]) && (j > 0)) {
                        x->boid[theBoid].neighborDistSqr[j] = x->boid[theBoid].neighborDistSqr[j - 1];
                        x->boid[theBoid].neighbor[j] = x->boid[theBoid].neighbor[j - 1];
                        j--;
                    }
                    x->boid[theBoid].neighborDistSqr[j] = distSqr;
                    x->boid[theBoid].neighbor[j] = i;                    
                }
            }
        }
    }

    /*********************************/
    /* Match and avoid the neighbors */    
    /*********************************/

    matchNeighborVel->x = 0;
    matchNeighborVel->y = 0;
    // matchNeighborVel->z = 0;
    
    // set tempSpeed to old speed
    tempSpeed = x->boid[theBoid].speed;
    
    for (i = 0; i < x->numNeighbors; i++) {
        neighbor = x->boid[theBoid].neighbor[i];
        
        // calculate matchNeighborVel by averaging the neighbor velocities
        matchNeighborVel->x += x->boid[neighbor].oldDir.x;
        matchNeighborVel->y += x->boid[neighbor].oldDir.y;
        // matchNeighborVel->z += x->boid[neighbor].oldDir.z;
            
        // if distance is less than preferred distance, then neighbor influences boid
        distSqr = x->boid[theBoid].neighborDistSqr[i];
        if (distSqr < x->prefDistSqr) {
            dist = sqrt(distSqr);

            distH = x->boid[neighbor].oldPos.x - x->boid[theBoid].oldPos.x;
            distV = x->boid[neighbor].oldPos.y - x->boid[theBoid].oldPos.y;
            // distD = x->boid[neighbor].oldPos.z - x->boid[theBoid].oldPos.z;
            
            if(dist == 0.0) dist = 0.0000001;
            totalVel.x = totalVel.x - distH - (distH * ((float) x->prefDist / (dist)));
            totalVel.y = totalVel.y - distV - (distV * ((float) x->prefDist / (dist)));
            // totalVel.z = totalVel.z - distD - (distV * ((float) x->prefDist / (dist)));
        
            numClose++;
        }
        if (InFront(&(x->boid[theBoid]), &(x->boid[neighbor]))) {    // adjust speed
            if (distSqr < x->prefDistSqr) 
                tempSpeed /= (x->accelFactor / 100.0);
            else
                tempSpeed *= (x->accelFactor / 100.0);
        }
        else {
            if (distSqr < x->prefDistSqr)
                tempSpeed *= (x->accelFactor / 100.0);
            else
                tempSpeed /= (x->accelFactor / 100.0);
        }
    }
    if (numClose) {
        avoidNeighborVel->x = totalVel.x / numClose;
        avoidNeighborVel->y = totalVel.y / numClose;
        // avoidNeighborVel->z = totalVel.z / numClose;
        NormalizeVelocity(matchNeighborVel);
    }
    else {
        avoidNeighborVel->x = 0;
        avoidNeighborVel->y = 0;
        // avoidNeighborVel->z = 0;
    }
    return(tempSpeed);
}


Velocity SeekPoint(t_boids *x, short theBoid, Point2d seekPt)
{
    Velocity    tempDir;
    tempDir.x = seekPt.x - x->boid[theBoid].oldPos.x;    
    tempDir.y = seekPt.y - x->boid[theBoid].oldPos.y;
    // tempDir.z = seekPt.z - x->boid[theBoid].oldPos.z;
    NormalizeVelocity(&tempDir);
    return(tempDir);
}


Velocity AvoidWalls(t_boids *x, short theBoid)
{
    Point2d        testPoint;
    Velocity    tempVel = {0.0, 0.0};//, 0.0};
        
    /* calculate test point in front of the nose of the boid */
    /* distance depends on the boid's speed and the avoid edge constant */
    testPoint.x = x->boid[theBoid].oldPos.x + x->boid[theBoid].oldDir.x * x->boid[theBoid].speed * x->edgeDist;
    testPoint.y = x->boid[theBoid].oldPos.y + x->boid[theBoid].oldDir.y * x->boid[theBoid].speed * x->edgeDist;
    // testPoint.z = x->boid[theBoid].oldPos.z + x->boid[theBoid].oldDir.z * x->boid[theBoid].speed * x->edgeDist;

    /* if test point is out of the left (right) side of x->flyRect, */
    /* return a positive (negative) horizontal velocity component */
    if (testPoint.x < x->flyRect.left)
        tempVel.x = fabs(x->boid[theBoid].oldDir.x);
    else if (testPoint.x > x->flyRect.right)
        tempVel.x = - fabs(x->boid[theBoid].oldDir.x);

    /* same with top and bottom */
    if (testPoint.y < x->flyRect.top)
        tempVel.y = fabs(x->boid[theBoid].oldDir.y);
    else if (testPoint.y > x->flyRect.bottom)
        tempVel.y = - fabs(x->boid[theBoid].oldDir.y);

    /* same with front and back
    if (testPoint.z < x->flyRect.front)
        tempVel.z = fabs(x->boid[theBoid].oldDir.z);
    else if (testPoint.z > x->flyRect.back)
        tempVel.z = - fabs(x->boid[theBoid].oldDir.z);
    */
    
    return(tempVel);
}


int InFront(BoidPtr theBoid, BoidPtr neighbor)
{
    float    grad, intercept;
    int result;
    
/* 

Find the gradient and y-intercept of a line passing through theBoid's oldPos
perpendicular to its direction of motion.  Another boid is in front of theBoid
if it is to the right or left of this linedepending on whether theBoid is moving
right or left.  However, if theBoid is travelling vertically then just compare
their vertical coordinates.    

*/
    // xy plane
    
    // if theBoid is not travelling vertically...
    if (theBoid->oldDir.x != 0) {
        // calculate gradient of a line _perpendicular_ to its direction (hence the minus)
        grad = -theBoid->oldDir.y / theBoid->oldDir.x;
        
        // calculate where this line hits the y axis (from y = mx + c)
        intercept = theBoid->oldPos.y - (grad * theBoid->oldPos.x);

        /* compare the horizontal position of the neighbor boid with */
        /* the point on the line that has its vertical coordinate */
        if (neighbor->oldPos.x >= ((neighbor->oldPos.y - intercept) / grad)) {
            /* return true if the first boid's horizontal movement is +ve */
            result = (theBoid->oldDir.x > 0);

            if (result==0) return 0;
            else goto next;
            
        } else {
            /* return true if the first boid's horizontal movement is +ve */
            result = (theBoid->oldDir.x < 0);
            if (result==0) return 0;
            else goto next;
        }
    }
    /* else theBoid is travelling vertically, so just compare vertical coordinates */
    else if (theBoid->oldDir.y > 0) {
        result = (neighbor->oldPos.y > theBoid->oldPos.y);
        if (result==0){ 
            return 0;
        }else{
            goto next;
        }
    }else{
        result = (neighbor->oldPos.y < theBoid->oldPos.y);
        if (result==0){
            return 0;
        } else {
            goto next;
        }
    }
next:
/*
    // yz plane
    
    // if theBoid is not travelling vertically... 
    if (theBoid->oldDir.y != 0) {
        // calculate gradient of a line _perpendicular_ to its direction (hence the minus) 
        grad = -theBoid->oldDir.z / theBoid->oldDir.y;
        
        // calculate where this line hits the y axis (from y = mx + c) 
        intercept = theBoid->oldPos.z - (grad * theBoid->oldPos.y);

        // compare the horizontal position of the neighbor boid with 
        // the point on the line that has its vertical coordinate 
        if (neighbor->oldPos.y >= ((neighbor->oldPos.z - intercept) / grad)) {
            // return true if the first boid's horizontal movement is +ve 
            result = (theBoid->oldDir.y > 0);
            if (result==0){ 
                return 0;
            }else{
                goto next2;
            }
        } else {
            // return true if the first boid's horizontal movement is +ve 
            result = (theBoid->oldDir.y < 0);
            if (result==0){
                return 0;
            }else{
                goto next2;
            }
        }
    }
    // else theBoid is travelling vertically, so just compare vertical coordinates 
    else if (theBoid->oldDir.z > 0) {
        result = (neighbor->oldPos.z > theBoid->oldPos.z);
        if (result==0){ 
            return 0;
        }else{
            goto next2;
        }
    }else{
        result = (neighbor->oldPos.z < theBoid->oldPos.z);
        if (result==0){
            return 0;
        }else{
            goto next2;
        }
    }
next2: */
    return 1;
}

void NormalizeVelocity(Velocity *direction)
{
    float    my_hypot;
    
    my_hypot = sqrt(direction->x * direction->x + direction->y * direction->y);// + direction->z * direction->z );

    if (my_hypot != 0.0) {
        direction->x = direction->x / my_hypot;
        direction->y = direction->y / my_hypot;
        // direction->z = direction->z / my_hypot;
    }
}

double RandomInt(double minRange, double maxRange)
{
    unsigned short    qdRdm;
    double            t, result;
    
    qdRdm = rand();
    t = (double)qdRdm / 65536.0;     // now 0 <= t <= 1
    result = (t * (maxRange - minRange)) + minRange;
    return(result);
}

double DistSqrToPt(Point2d firstPoint, Point2d secondPoint)
{
    double    a, b,c;
    a = firstPoint.x - secondPoint.x;
    b = firstPoint.y - secondPoint.y;    
    //c = firstPoint.z - secondPoint.z;    
    return(a * a + b * b); // + c * c);
}
