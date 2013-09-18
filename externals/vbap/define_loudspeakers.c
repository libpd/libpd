/* define_loudspeakers.c 1.00b1----> x-max4.2

written by Ville Pulkki 1999-2003
Helsinki University of Technology 
and 
Unversity of California at Berkeley

See copyright in file with name LICENSE.txt  */

#include "define_loudspeakers.h"

#ifndef VBAP_OBJECT
# ifdef PD
// If we are within VBAP (which includes define_loudspeakers), then don't create a main for define_loudspeakres
void define_loudspeakers_setup(void)
{
	def_ls_class = class_new(gensym("define_loudspeakers"), (t_newmethod)def_ls_new, 0, (short)sizeof(t_def_ls), 0, A_GIMME, 0); 
	/* def_ls_new = creation function, A_DEFLONG = its (optional) arguement is a long (32-bit) int */
	
	class_addbang(def_ls_class, (t_method)def_ls_bang);			/* the procedure it uses when it gets a bang in the left inlet */
	class_addmethod(def_ls_class, (t_method)def_ls_read_directions, gensym("ls-directions"), A_GIMME, 0);	
	class_addmethod(def_ls_class, (t_method)def_ls_read_triplets, gensym("ls-triplets"), A_GIMME, 0);

	post(DFLS_VERSION);
}
# else /* Max */
void main(void)
{
	setup((t_messlist **)&def_ls_class, (method)def_ls_new, 0L, (short)sizeof(t_def_ls), 0L, A_GIMME, 0); 
	/* def_ls_new = creation function, A_DEFLONG = its (optional) arguement is a long (32-bit) int */
	
	addbang((method)def_ls_bang);			/* the procedure it uses when it gets a bang in the left inlet */
	addmess((method)def_ls_read_directions, "ls-directions", A_GIMME, 0);	
	addmess((method)def_ls_read_triplets, "ls-triplets", A_GIMME, 0);
	addmess((method)traces, "enabletrace", A_LONG, 0);

	post(DFLS_VERSION);
}
# endif /* PD */
#endif /* ! VBAP_OBJECT */

static void def_ls_bang(t_def_ls *x)						/* x = reference to this instance of the object */ 
{   // calculate and print out chosen loudspeaker sets and corresponding  matrices
	
	if(x->x_ls_read == 1)
	{
		if(x->x_def_ls_amount < x->x_def_ls_dimension)
		{
			error("define-loudspeakers: Too few loudspeakers!");
			return;
		} 
		else 
		{
			if(x->x_def_ls_dimension == 3)
			{
				if(x->x_triplets_specified==0) choose_ls_triplets(x);
				calculate_3x3_matrixes(x);
			} 
			else if(x->x_def_ls_dimension == 2)
			{
		  	choose_ls_tuplets(x);
			}
		  else 
			{
		  	error("define-loudspeakers: Error in loudspeaker direction data");
		  	error("dimension azimuth1 [elevation1] azimuth2 [elevation2]...");
		  	error("dimension == 2 for horizontal ls arrays");
		  	error("dimension == 3 for 3-D ls arrays (speakers also upward and/or downward ");
		  }
		}
	} 
	else
	{
		error("define-loudspeakers: Error in loudspeaker direction data");
		error("dimension azimuth1 [elevation1] azimuth2 [elevation2]...");
		error("dimension == 2 for horizontal ls arrays");
		error("dimension == 3 for 3-D ls arrays (speakers also upward and/or downward ");
	}
}

/*--------------------------------------------------------------------------*/

/*
void def_ls_int(t_def_ls *x, long n)		// x = the instance of the object, n = the int received in the right inlet 
{
 // do something if an int comes in the left inlet???	
}
*/

static void def_ls_read_triplets(t_def_ls *x, t_symbol *s, int ac, Atom *av)
// when loudspeaker triplets come in a message
{
	t_ls_set *trip_ptr,  *tmp_ptr, *prev;
	if(x->x_ls_read == 0)
	{
		error("define_loudspeakers: Define loudspeaker directions first!");
		return;
	}
	
	if(x->x_def_ls_dimension == 2)
	{
		error("define_loudspeakers: Can't specify loudspeaker triplets in 2-D setup!");
		return;
	}
		
 	trip_ptr = x->x_ls_set;
	prev = NULL;
	while (trip_ptr != NULL)
	{
		tmp_ptr = trip_ptr;
		trip_ptr = trip_ptr->next;
		freebytes(tmp_ptr, sizeof (struct t_ls_set));
	}
	x->x_ls_set = NULL;
	
    int i;
	for(i=0;i<ac;i+=3)
	{
		long l1 = 0,l2 = 0,l3 = 0;

/*
		if(av[i].a_type == A_LONG)
			l1 = av[i].a_w.w_long;
		else */ if(av[i].a_type == A_FLOAT) 
			l1 =  (long) av[i].a_w.w_float;

/*
		if(av[i+1].a_type == A_LONG) 
			l2 = av[i+1].a_w.w_long;
		else */ if(av[i+1].a_type == A_FLOAT) 
			l2 =  (long) av[i+1].a_w.w_float;

/*
		if(av[i+2].a_type == A_LONG) 
			l3 = av[i+2].a_w.w_long;
		else */ if(av[i+2].a_type == A_FLOAT) 
			l3 =  (long) av[i+2].a_w.w_float;

		add_ldsp_triplet(l1-1,l2-1,l3-1,x);
	}
	x->x_triplets_specified=1;
}



static void def_ls_read_directions(t_def_ls *x, t_symbol *s, int ac, Atom *av)
// when loudspeaker directions come in a message
{
	if (x->x_ls_read)
	{
		// Remove old matrices
		t_ls_set* trip_ptr = x->x_ls_set;
		while (trip_ptr != NULL)
		{ // remove old matrices
		 t_ls_set* tmp_ptr = trip_ptr;
		 trip_ptr = trip_ptr->next;
		 freebytes(tmp_ptr, sizeof (struct t_ls_set));
		}
	}
	x->x_ls_set = NULL;

	initContent_ls_directions(x,ac,av);
}

/*--------------------------------------------------------------------------*/

static void ls_angles_to_cart(t_ls *ls)
// convert angular direction to cartesian
{
  t_float azi = ls->azi;
  t_float ele = ls->ele;
  ls->x = cos((t_float) azi * atorad) * cos((t_float) ele * atorad);
  ls->y = sin((t_float) azi * atorad) * cos((t_float) ele * atorad);
  ls->z = sin((t_float) ele * atorad);
}

/* create new instance of object... MUST send it an int even if you do nothing with this int!! */
static void *def_ls_new(t_symbol *s, int ac, Atom *av)	
{
	// s is object name (we ignore it)
	t_def_ls *x = (t_def_ls *)newobject(def_ls_class);

#ifdef PD
	x->x_outlet0 =  outlet_new(&x->x_obj, gensym("list"));  /* create a (list) outlet */
#else /* Max */
	x->x_outlet0 =  outlet_new(x, 0L);	/* create a (list) outlet */
#endif /* PD */

	initContent_ls_directions(x,ac,av); // Initialize object internal data from a ls-directions list

	return x;					/* return a reference to the object instance */
}

/* define-loudspeakers message integrated into vbap object */
void vbap_def_ls(t_def_ls *x, t_symbol *s, int ac, Atom *av)	
{
	initContent_ls_directions(x,ac,av); // Initialize object internal data from a ls-directions list
	def_ls_bang(x); // calculate and send matrix to vbap
}

/** Initialize the object content from parameters : ls-directions list */
static void initContent_ls_directions(t_def_ls *x,int ac,Atom*av)
{
	x->x_ls_read = 0;
	
	long d = 0;
/*	if (av[0].a_type == A_LONG) d = av[0].a_w.w_long;
	else */ if(av[0].a_type == A_FLOAT) d = (long)av[0].a_w.w_float;
	else { error("define-loudspeakers: dimension NaN"); return; }

	if (d==2 || d==3)
	{
		 x->x_def_ls_dimension= d;
		 x->x_ls_read = 1;
	} 
	else
	{
		x->x_def_ls_dimension= 0;
		error("define-loudspeakers: Dimension has to be 2 or 3!");
		return;
	}
		
	int pointer = 1;
	x->x_def_ls_amount= (ac-1) / (x->x_def_ls_dimension - 1);

	// read loudspeaker direction angles  
    int i;
	for(i=0; i < x->x_def_ls_amount;i++)
	{
		t_float azi = 0;
/*		if(av[pointer].a_type == A_LONG) azi = (float) av[pointer].a_w.w_long;
		else */ if(av[pointer].a_type == A_FLOAT) azi = av[pointer].a_w.w_float;
		else { error("define-loudspeakers: direction angle #%d NaN",i+1); x->x_ls_read = 0; return; }

		x->x_ls[i].azi = azi;
		
		pointer++;

		t_float ele = 0; // in 2d elevation is zero
		if(x->x_def_ls_dimension == 3)
		{  // 3-D 
/*			if(av[pointer].a_type == A_LONG) ele = (float) av[pointer].a_w.w_long;
			else */ if(av[pointer].a_type == A_FLOAT) ele = av[pointer].a_w.w_float;
			else { error("define-loudspeakers: elevation #%d NaN",i+1);  x->x_ls_read = 0; return; }

			pointer++;
		} 
		x->x_ls[i].ele = ele;
	}
	
	if(x->x_ls_read == 1)
	{
        int i;
		for(i=0;i<x->x_def_ls_amount;i++)
		{
			ls_angles_to_cart(&x->x_ls[i]);	
		}
	}
	x->x_triplets_specified=0;
	x->x_ls_set = NULL;
}

static void choose_ls_triplets(t_def_ls *x) 
     /* Selects the loudspeaker triplets, and
      calculates the inversion matrices for each selected triplet.
     A line (connection) is drawn between each loudspeaker. The lines
     denote the sides of the triangles. The triangles should not be 
     intersecting. All crossing connections are searched and the 
     longer connection is erased. This yields non-intesecting triangles,
     which can be used in panning. 
     See theory in paper Pulkki, V. Lokki, T. "Creating Auditory Displays
     with Multiple Loudspeakers Using VBAP: A Case Study with
     DIVA Project" in International Conference on 
     Auditory Displays -98.*/
{
  int i,j,k,l,/*m,li,*/ table_size;
  //int *i_ptr;
  //t_ls vb1,vb2,tmp_vec;
  int connections[MAX_LS_AMOUNT][MAX_LS_AMOUNT];
  //float angles[MAX_LS_AMOUNT];
  //int sorted_angles[MAX_LS_AMOUNT];
  t_float distance_table[((MAX_LS_AMOUNT * (MAX_LS_AMOUNT - 1)) / 2)];
  int distance_table_i[((MAX_LS_AMOUNT * (MAX_LS_AMOUNT - 1)) / 2)];
  int distance_table_j[((MAX_LS_AMOUNT * (MAX_LS_AMOUNT - 1)) / 2)];
  t_float distance;
  t_ls_set *trip_ptr, *prev, *tmp_ptr;
  int ls_amount = x->x_def_ls_amount;
  t_ls *lss = x->x_ls;
  if (ls_amount == 0) { post("define-loudspeakers: Number of loudspeakers is zero"); return; }
 
  for(i=0;i<ls_amount;i++)
    for(j=i+1;j<ls_amount;j++)
      for(k=j+1;k<ls_amount;k++)
			{
        if(vol_p_side_lgth(i,j,k, x->x_ls) > MIN_VOL_P_SIDE_LGTH)
				{
          connections[i][j]=1;
          connections[j][i]=1;
          connections[i][k]=1;
          connections[k][i]=1;
          connections[j][k]=1;
          connections[k][j]=1;
          add_ldsp_triplet(i,j,k,x);
        }
      }
   

  /*calculate distancies between all lss and sorting them*/
  table_size =(((ls_amount - 1) * (ls_amount)) / 2); 
  for(i=0;i<table_size; i++)
    distance_table[i] = 100000.0;
  for(i=0;i<ls_amount;i++)
	{ 
    for(j=(i+1);j<ls_amount; j++)
		{ 
		  if(connections[i][j] == 1) 
		  {
        distance = fabs(vec_angle(lss[i],lss[j]));
        k=0;
        while(distance_table[k] < distance)
          k++;
        for(l=(table_size - 1);l > k ;l--)
				{
          distance_table[l] = distance_table[l-1];
          distance_table_i[l] = distance_table_i[l-1];
          distance_table_j[l] = distance_table_j[l-1];
        }
        distance_table[k] = distance;
        distance_table_i[k] = i;
        distance_table_j[k] = j;
      } 
			else
			{
        table_size--;
			}
    }
  }

  /* disconnecting connections which are crossing shorter ones,
     starting from shortest one and removing all that cross it,
     and proceeding to next shortest */
  for(i=0; i<(table_size); i++)
	{
    int fst_ls = distance_table_i[i];
		int sec_ls = distance_table_j[i];
    if(connections[fst_ls][sec_ls] == 1)
		{
      for(j=0; j<ls_amount ; j++)
			{
        for(k=j+1; k<ls_amount; k++)
				{
          if( (j!=fst_ls) && (k != sec_ls) && (k!=fst_ls) && (j != sec_ls))
					{
            if(lines_intersect(fst_ls, sec_ls, j,k,x->x_ls) == 1)
						{
              connections[j][k] = 0;
              connections[k][j] = 0;
            }
          }
				}
			}
		}
  }

  /* remove triangles which had crossing sides
     with smaller triangles or include loudspeakers*/
  trip_ptr = x->x_ls_set;
  prev = NULL;
  while (trip_ptr != NULL)
	{
    i = trip_ptr->ls_nos[0];
    j = trip_ptr->ls_nos[1];
    k = trip_ptr->ls_nos[2];
    if(connections[i][j] == 0 || 
       connections[i][k] == 0 || 
       connections[j][k] == 0 ||
			 any_ls_inside_triplet(i,j,k,x->x_ls,ls_amount) == 1 )
		{
      if(prev != NULL) 
			{
        prev->next = trip_ptr->next;
        tmp_ptr = trip_ptr;
        trip_ptr = trip_ptr->next;
        freebytes(tmp_ptr, sizeof (struct t_ls_set));
      } 
			else 
			{
        x->x_ls_set = trip_ptr->next;
        tmp_ptr = trip_ptr;
        trip_ptr = trip_ptr->next;
        freebytes(tmp_ptr, sizeof (struct t_ls_set));
      }
    } 
		else 
		{
      prev = trip_ptr;
      trip_ptr = trip_ptr->next;
    }
  }
  x->x_triplets_specified=1;
}


int any_ls_inside_triplet(int a, int b, int c,t_ls lss[MAX_LS_AMOUNT],int ls_amount)
   /* returns 1 if there is loudspeaker(s) inside given ls triplet */
{
  t_float invdet;
  t_ls *lp1, *lp2, *lp3;
  t_float invmx[9];
  int i,j;
  t_float tmp;
  int any_ls_inside, this_inside;

  lp1 =  &(lss[a]);
  lp2 =  &(lss[b]);
  lp3 =  &(lss[c]);

  /* matrix inversion */
  invdet = 1.0 / (  lp1->x * ((lp2->y * lp3->z) - (lp2->z * lp3->y))
                    - lp1->y * ((lp2->x * lp3->z) - (lp2->z * lp3->x))
                    + lp1->z * ((lp2->x * lp3->y) - (lp2->y * lp3->x)));
  
  invmx[0] = ((lp2->y * lp3->z) - (lp2->z * lp3->y)) * invdet;
  invmx[3] = ((lp1->y * lp3->z) - (lp1->z * lp3->y)) * -invdet;
  invmx[6] = ((lp1->y * lp2->z) - (lp1->z * lp2->y)) * invdet;
  invmx[1] = ((lp2->x * lp3->z) - (lp2->z * lp3->x)) * -invdet;
  invmx[4] = ((lp1->x * lp3->z) - (lp1->z * lp3->x)) * invdet;
  invmx[7] = ((lp1->x * lp2->z) - (lp1->z * lp2->x)) * -invdet;
  invmx[2] = ((lp2->x * lp3->y) - (lp2->y * lp3->x)) * invdet;
  invmx[5] = ((lp1->x * lp3->y) - (lp1->y * lp3->x)) * -invdet;
  invmx[8] = ((lp1->x * lp2->y) - (lp1->y * lp2->x)) * invdet;

  any_ls_inside = 0;
  for(i=0; i< ls_amount; i++) 
	{
    if (i != a && i!=b && i != c)
		{
      this_inside = 1;
      for(j=0; j< 3; j++)
			{
        tmp = lss[i].x * invmx[0 + j*3];
        tmp += lss[i].y * invmx[1 + j*3];
        tmp += lss[i].z * invmx[2 + j*3];
        if(tmp < -0.001)
          this_inside = 0;
      }
      if(this_inside == 1)
        any_ls_inside=1;
    }
  }
  return any_ls_inside;
}

static void add_ldsp_triplet(int i, int j, int k, t_def_ls *x)
     /* adds i,j,k triplet to structure*/
{
  struct t_ls_set *trip_ptr, *prev;
  trip_ptr = x->x_ls_set;
  prev = NULL;
  while (trip_ptr != NULL)
	{
    prev = trip_ptr;
    trip_ptr = trip_ptr->next;
  }
  trip_ptr = (struct t_ls_set*) getbytes (sizeof (struct t_ls_set));
  if(prev == NULL)
    x->x_ls_set = trip_ptr;
  else 
    prev->next = trip_ptr;
  trip_ptr->next = NULL;
  trip_ptr->ls_nos[0] = i;
  trip_ptr->ls_nos[1] = j;
	trip_ptr->ls_nos[2] = k;
}




t_float vec_angle(t_ls v1, t_ls v2)
// angle between two loudspeakers
{
  t_float inner= ((v1.x*v2.x + v1.y*v2.y + v1.z*v2.z)/
              (vec_length(v1) * vec_length(v2)));
  if(inner > 1.0)
    inner= 1.0;
  if (inner < -1.0)
    inner = -1.0;
  return fabs( acos( inner));
}

t_float vec_length(t_ls v1)
// length of a vector
{
  return (sqrt(v1.x*v1.x + v1.y*v1.y + v1.z*v1.z));
}

t_float vec_prod(t_ls v1, t_ls v2)
// vector dot product
{
  return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}


t_float vol_p_side_lgth(int i, int j,int k, t_ls  lss[MAX_LS_AMOUNT] )
{
  /* calculate volume of the parallelepiped defined by the loudspeaker
     direction vectors and divide it with total length of the triangle sides. 
     This is used when removing too narrow triangles. */

  t_float volper, lgth;
  t_ls xprod;
  ls_cross_prod(lss[i], lss[j], &xprod);
  volper = fabsf(vec_prod(xprod, lss[k]));
  lgth = (fabsf(vec_angle(lss[i],lss[j])) 
          + fabsf(vec_angle(lss[i],lss[k])) 
          + fabsf(vec_angle(lss[j],lss[k])));
  if(lgth>0.00001)
    return volper / lgth;
  else
    return 0.0;
}

static void ls_cross_prod(t_ls v1,t_ls v2, 
                t_ls *res) 
// vector cross product
{
  t_float length;
  res->x = (v1.y * v2.z ) - (v1.z * v2.y);
  res->y = (v1.z * v2.x ) - (v1.x * v2.z);
  res->z = (v1.x * v2.y ) - (v1.y * v2.x);

  length= vec_length(*res);
  res->x /= length;
  res->y /= length;
  res->z /= length;
}


static int lines_intersect(int i,int j,int k,int l,t_ls  lss[MAX_LS_AMOUNT])
     /* checks if two lines intersect on 3D sphere 
       */
{
 t_ls v1;
  t_ls v2;
  t_ls v3, neg_v3;
  //t_float angle;
  t_float dist_ij,dist_kl,dist_iv3,dist_jv3,dist_inv3,dist_jnv3;
  t_float dist_kv3,dist_lv3,dist_knv3,dist_lnv3;
  // TODO epsilon needs to be updated for 64-bit/double precision
  t_float epsilon = 1e-9;

  ls_cross_prod(lss[i],lss[j],&v1);
  ls_cross_prod(lss[k],lss[l],&v2);
  ls_cross_prod(v1,v2,&v3);

  neg_v3.x= 0.0 - v3.x;
  neg_v3.y= 0.0 - v3.y;
  neg_v3.z= 0.0 - v3.z;

  dist_ij = (vec_angle(lss[i],lss[j]));
  dist_kl = (vec_angle(lss[k],lss[l]));
  dist_iv3 = (vec_angle(lss[i],v3));
  dist_jv3 = (vec_angle(v3,lss[j]));
  dist_inv3 = (vec_angle(lss[i],neg_v3));
  dist_jnv3 = (vec_angle(neg_v3,lss[j]));
  dist_kv3 = (vec_angle(lss[k],v3));
  dist_lv3 = (vec_angle(v3,lss[l]));
  dist_knv3 = (vec_angle(lss[k],neg_v3));
  dist_lnv3 = (vec_angle(neg_v3,lss[l]));

  /* if one of loudspeakers is close to crossing point, don't do anything*/
  if(fabsf(dist_iv3)  <= epsilon || fabsf(dist_jv3)  <= epsilon || 
     fabsf(dist_kv3)  <= epsilon || fabsf(dist_lv3)  <= epsilon ||
     fabsf(dist_inv3) <= epsilon || fabsf(dist_jnv3) <= epsilon || 
     fabsf(dist_knv3) <= epsilon || fabsf(dist_lnv3) <= epsilon )
    return(0);

  // if crossing point is on line between both loudspeakers return 1
  if (((fabsf(dist_ij - (dist_iv3 + dist_jv3))   <= epsilon)  &&
       (fabsf(dist_kl - (dist_kv3 + dist_lv3))   <= epsilon)) ||
      ((fabsf(dist_ij - (dist_inv3 + dist_jnv3)) <= epsilon)  &&
       (fabsf(dist_kl - (dist_knv3 + dist_lnv3)) <= epsilon))) {
    return (1);
  } else {
    return (0);
  }
}

static void  calculate_3x3_matrixes(t_def_ls *x)
     /* Calculates the inverse matrices for 3D */
{  
  t_float invdet;
  t_ls *lp1, *lp2, *lp3;
  t_float *invmx;
  //t_float *ptr;
  struct t_ls_set *tr_ptr = x->x_ls_set;
  int triplet_amount = 0, /*ftable_size,*/i,pointer,list_length=0;
  Atom *at;
  t_ls *lss = x->x_ls;
  
  if (tr_ptr == NULL)
	{
    error("define-loudspeakers: Not valid 3-D configuration\n");
    return;
  }
	
  /* counting triplet amount */
  while(tr_ptr != NULL)
	{
    triplet_amount++;
    tr_ptr = tr_ptr->next;
  }
  tr_ptr = x->x_ls_set;
  list_length= triplet_amount * 21 + 3;
  at= (Atom *) getbytes(list_length*sizeof(Atom));
  
  SETLONG(&at[0], x->x_def_ls_dimension);
  SETLONG(&at[1], x->x_def_ls_amount);
  pointer=2;
  
  while(tr_ptr != NULL){
    lp1 =  &(lss[tr_ptr->ls_nos[0]]);
    lp2 =  &(lss[tr_ptr->ls_nos[1]]);
    lp3 =  &(lss[tr_ptr->ls_nos[2]]);

    /* matrix inversion */
    invmx = tr_ptr->inv_mx;
    invdet = 1.0 / (  lp1->x * ((lp2->y * lp3->z) - (lp2->z * lp3->y))
                    - lp1->y * ((lp2->x * lp3->z) - (lp2->z * lp3->x))
                    + lp1->z * ((lp2->x * lp3->y) - (lp2->y * lp3->x)));

    invmx[0] = ((lp2->y * lp3->z) - (lp2->z * lp3->y)) * invdet;
    invmx[3] = ((lp1->y * lp3->z) - (lp1->z * lp3->y)) * -invdet;
  	invmx[6] = ((lp1->y * lp2->z) - (lp1->z * lp2->y)) * invdet;
    invmx[1] = ((lp2->x * lp3->z) - (lp2->z * lp3->x)) * -invdet;
    invmx[4] = ((lp1->x * lp3->z) - (lp1->z * lp3->x)) * invdet;
    invmx[7] = ((lp1->x * lp2->z) - (lp1->z * lp2->x)) * -invdet;
    invmx[2] = ((lp2->x * lp3->y) - (lp2->y * lp3->x)) * invdet;
    invmx[5] = ((lp1->x * lp3->y) - (lp1->y * lp3->x)) * -invdet;
    invmx[8] = ((lp1->x * lp2->y) - (lp1->y * lp2->x)) * invdet;
    for(i=0;i<3;i++){
    	SETLONG(&at[pointer], tr_ptr->ls_nos[i]+1);
    	pointer++;
    }
    for(i=0;i<9;i++){
    	SETFLOAT(&at[pointer], invmx[i]);
    	pointer++;
    }
    SETFLOAT(&at[pointer], lp1->x); pointer++;
    SETFLOAT(&at[pointer], lp2->x); pointer++;
    SETFLOAT(&at[pointer], lp3->x); pointer++;
    SETFLOAT(&at[pointer], lp1->y); pointer++;
    SETFLOAT(&at[pointer], lp2->y); pointer++;
    SETFLOAT(&at[pointer], lp3->y); pointer++;
    SETFLOAT(&at[pointer], lp1->z); pointer++;
    SETFLOAT(&at[pointer], lp2->z); pointer++;
    SETFLOAT(&at[pointer], lp3->z); pointer++;
 
    tr_ptr = tr_ptr->next;
  }
	sendLoudspeakerMatrices(x,list_length, at);
//  outlet_anything(x->x_outlet0, gensym("loudspeaker-matrices"), list_length, at);
  freebytes(at, list_length*sizeof(Atom));
}



static void choose_ls_tuplets(t_def_ls *x)
     /* selects the loudspeaker pairs, calculates the inversion
        matrices and stores the data to a global array*/
{
  //t_float atorad = (2 * 3.1415927 / 360) ;
  int i,j;
  //t_float w1,w2;
  //t_float p1,p2;
  int sorted_lss[MAX_LS_AMOUNT];
  int exist[MAX_LS_AMOUNT];   
  int amount=0;
  t_float inv_mat[MAX_LS_AMOUNT][4];  // In 2-D ls amount == max amount of LS pairs
  t_float mat[MAX_LS_AMOUNT][4];
  //t_float *ptr;   
  //t_float *ls_table;
  t_ls *lss = x->x_ls;
  long ls_amount=x->x_def_ls_amount;
  long list_length;
  Atom *at;
  long pointer;
  
  for(i=0;i<MAX_LS_AMOUNT;i++){
    exist[i]=0;
  }

  /* sort loudspeakers according their aximuth angle */
  sort_2D_lss(x->x_ls,sorted_lss,ls_amount);

  /* adjacent loudspeakers are the loudspeaker pairs to be used.*/
  for(i=0;i<(ls_amount-1);i++){
    if((lss[sorted_lss[i+1]].azi - 
        lss[sorted_lss[i]].azi) <= (180 - 10)){
      if (calc_2D_inv_tmatrix( lss[sorted_lss[i]].azi, 
                               lss[sorted_lss[i+1]].azi, 
                               inv_mat[i],mat[i]) != 0){
        exist[i]=1;
        amount++;
      }
    }
  }

  if(((360 - lss[sorted_lss[ls_amount-1]].azi) 
      +lss[sorted_lss[0]].azi) <= (180 -  10)) {
    if(calc_2D_inv_tmatrix(lss[sorted_lss[ls_amount-1]].azi, 
                           lss[sorted_lss[0]].azi, 
                           inv_mat[ls_amount-1],mat[ls_amount-1]) != 0) { 
        exist[ls_amount-1]=1;
        amount++;
    } 
  }
  
  
  // Output
  list_length= amount * 10  + 2;
  at= (Atom *) getbytes(list_length*sizeof(Atom));
  
  SETLONG(&at[0], x->x_def_ls_dimension);
  SETLONG(&at[1], x->x_def_ls_amount);
  pointer=2;
  
  for (i=0;i<ls_amount - 1;i++){
    if(exist[i] == 1) {
    	SETLONG(&at[pointer], sorted_lss[i]+1);
    	pointer++;
    	SETLONG(&at[pointer], sorted_lss[i+1]+1);
    	pointer++;
       	for(j=0;j<4;j++) {
       		SETFLOAT(&at[pointer], inv_mat[i][j]);
    		pointer++;
       	}
       for(j=0;j<4;j++) {
       		SETFLOAT(&at[pointer], mat[i][j]);
    		pointer++;
       	}
    }
  }
  if(exist[ls_amount-1] == 1) {
    SETLONG(&at[pointer], sorted_lss[ls_amount-1]+1);
    pointer++;
    SETLONG(&at[pointer], sorted_lss[0]+1);
    pointer++;
    for(j=0;j<4;j++) {
    	SETFLOAT(&at[pointer], inv_mat[ls_amount-1][j]);
    	pointer++;
    }
    for(j=0;j<4;j++) {
    	SETFLOAT(&at[pointer], mat[ls_amount-1][j]);
    	pointer++;
    }
  }
	sendLoudspeakerMatrices(x,list_length, at);
  //outlet_anything(x->x_outlet0, gensym("loudspeaker-matrices"), list_length, at);
  freebytes(at, list_length*sizeof(Atom));
}

void sort_2D_lss(t_ls lss[MAX_LS_AMOUNT], int sorted_lss[MAX_LS_AMOUNT], 
                 int ls_amount)
// sort loudspeakers according to azimuth angle
{
  t_float tmp, tmp_azi;
//  t_float rad2ang = 360.0f / ( 2.0f * M_PI );

  //t_float x,y;
  /* Transforming angles between -180 and 180 */
  int i;
  for (i=0;i<ls_amount;i++) 
	{
    ls_angles_to_cart(&lss[i]);
    lss[i].azi = acos( lss[i].x) * rad2ang;
    if (fabs(lss[i].y) <= 0.001)
        tmp = 1.0;
    else
        tmp = lss[i].y / fabs(lss[i].y);
    lss[i].azi *= tmp;
  }
for (i=0;i<ls_amount;i++)
	{
    tmp = 2000;
		int index = 0;
        int j;
    for (j=0 ; j<ls_amount;j++)
		{
      if (lss[j].azi <= tmp)
			{
        tmp=lss[j].azi;
        index = j;
      }
    }
    sorted_lss[i]=index;
    tmp_azi = (lss[index].azi);
    lss[index].azi = (tmp_azi + (t_float) 4000.0);
  }
  for (i=0;i<ls_amount;i++) 
	{
    tmp_azi = (lss[i].azi);
    lss[i].azi = (tmp_azi - (t_float) 4000.0);
  }
}
  

static int calc_2D_inv_tmatrix(t_float azi1,t_float azi2, t_float inv_mat[4],t_float mat[4])
// calculate inverse 2x2 matrix
{
  t_float x1,x2,x3,x4; /* x1 x3 */
  //t_float y1,y2,y3,y4; /* x2 x4 */
  t_float det;
  
  mat[0]=x1 = cos(azi1 / rad2ang);
  mat[1]=x2 = sin(azi1 / rad2ang);
  mat[2]=x3 = cos(azi2 / rad2ang);
  mat[3]=x4 = sin(azi2 / rad2ang);
  det = (x1 * x4) - ( x3 * x2 );
   if(fabsf(det) <= 0.001) {

    inv_mat[0] = 0.0;
    inv_mat[1] = 0.0;
    inv_mat[2] = 0.0;
    inv_mat[3] = 0.0;
    return 0;
  } else {
    inv_mat[0] =   (x4 / det);
    inv_mat[1] =  (-x3 / det);
    inv_mat[2] =   (-x2 / det);
    inv_mat[3] =    (x1 / det);
    return 1;
  }
}
