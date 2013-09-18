/*
      glm.c
      Nate Robins, 1997, 2000
      nate@pobox.com, http://www.pobox.com/~nate

      Wavefront OBJ model file format reader/writer/manipulator.

      Includes routines for generating smooth normals with
      preservation of edges, welding redundant vertices & texture
      coordinate generation (spheremap and planar projections) + more.

*/

#include "model_loader.h"
/* for post(), error(),... */
#include "m_pd.h"

#define T(x) (model->triangles[(x)])

/* _GLMnode: general purpose node */
typedef struct _GLMnode {
  GLuint         index;
  GLboolean      averaged;
  struct _GLMnode* next;
} GLMnode;


/* glmMax: returns the maximum of two floats */
static GLfloat
_glmMax(GLfloat a, GLfloat b)
{
  if (b > a)
    return b;
  return a;
}

/* glmAbs: returns the absolute value of a float */
static GLfloat
_glmAbs(GLfloat f)
{
  if (f < 0)
    return -f;
  return f;
}

/* glmDot: compute the dot product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 */
static GLfloat
_glmDot(GLfloat* u, GLfloat* v)
{
  if (!(u) || !(v))return 0.f;

  return u[0]*v[0] + u[1]*v[1] + u[2]*v[2];
}

/* glmCross: compute the cross product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 * n - array of 3 GLfloats (GLfloat n[3]) to return the cross product in
 */
static GLvoid
_glmCross(GLfloat* u, GLfloat* v, GLfloat* n)
{
  if (!(u))return; if (!(v))return; if (!(n))return;

  n[0] = u[1]*v[2] - u[2]*v[1];
  n[1] = u[2]*v[0] - u[0]*v[2];
  n[2] = u[0]*v[1] - u[1]*v[0];
}

/* glmNormalize: normalize a vector
 *
 * v - array of 3 GLfloats (GLfloat v[3]) to be normalized
 */
static GLvoid
_glmNormalize(GLfloat* v)
{
  GLfloat l;

  if (!(v))return;

  l = static_cast<GLfloat>(sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]));
  v[0] /= l;
  v[1] /= l;
  v[2] /= l;
}

/* glmEqual: compares two vectors and returns GL_TRUE if they are
 * equal (within a certain threshold) or GL_FALSE if not. An epsilon
 * that works fairly well is 0.000001.
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 */
static GLboolean
_glmEqual(GLfloat* u, GLfloat* v, GLfloat epsilon)
{
  if (_glmAbs(u[0] - v[0]) < epsilon &&
      _glmAbs(u[1] - v[1]) < epsilon &&
      _glmAbs(u[2] - v[2]) < epsilon)
    {
      return GL_TRUE;
    }
  return GL_FALSE;
}

/* glmWeldVectors: eliminate (weld) vectors that are within an
 * epsilon of each other.
 *
 * vectors     - array of GLfloat[3]'s to be welded
 * numvectors - number of GLfloat[3]'s in vectors
 * epsilon     - maximum difference between vectors
 *
 */
GLfloat*
_glmWeldVectors(GLfloat* vectors, GLuint* numvectors, GLfloat epsilon)
{
  GLfloat* copies;
  GLuint   copied;
  GLuint   i, j;

  copies = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (*numvectors + 1));
  memcpy(copies, vectors, (sizeof(GLfloat) * 3 * (*numvectors + 1)));

  copied = 1;
  for (i = 1; i <= *numvectors; i++) {
    for (j = 1; j <= copied; j++) {
      if (_glmEqual(&vectors[3 * i], &copies[3 * j], epsilon)) {
        goto duplicate;
      }
    }

    /* must not be any duplicates -- add to the copies array */
    copies[3 * copied + 0] = vectors[3 * i + 0];
    copies[3 * copied + 1] = vectors[3 * i + 1];
    copies[3 * copied + 2] = vectors[3 * i + 2];
    j = copied;             /* pass this along for below */
    copied++;

  duplicate:
    /* set the first component of this vector to point at the correct
       index into the new copies array */
    vectors[3 * i + 0] = static_cast<GLfloat>(j);
  }

  *numvectors = copied-1;
  return copies;
}

/* glmFindGroup: Find a group in the model */
GLMgroup*
_glmFindGroup(GLMmodel* model, char* name)
{
  GLMgroup* group;

  if (!(model))return NULL;

  group = model->groups;
  while(group) {
    if (!strcmp(name, group->name))
      break;
    group = group->next;
  }

  return group;
}

/* glmAddGroup: Add a group to the model */
GLMgroup*
_glmAddGroup(GLMmodel* model, char* name)
{
  GLMgroup* group;

  group = _glmFindGroup(model, name);
  if (!group) {
    group = (GLMgroup*)malloc(sizeof(GLMgroup));
    group->name = strdup(name);
    group->material = 0;
    group->numtriangles = 0;
    group->triangles = NULL;
    group->next = model->groups;
    model->groups = group;
    model->numgroups++;
  }

  return group;
}

/* glmFindGroup: Find a material in the model */
GLuint
_glmFindMaterial(GLMmodel* model, char* name)
{
  GLuint i;

  /* XXX doing a linear search on a string key'd list is pretty lame,
     but it works and is fast enough for now. */
  for (i = 0; i < model->nummaterials; i++) {
    if (!strcmp(model->materials[i].name, name))
      goto found;
  }

  /* didn't find the name, so print a warning and return the default
     material (0). */
  error("_glmFindMaterial():  can't find material \"%s\".", name);
  i = 0;

 found:
  return i;
}


/* glmDirName: return the directory given a path
 *
 * path - filesystem path
 *
 * NOTE: the return value should be free'd.
 */
static char*
_glmDirName(char* path)
{
  char* dir;
  char* s;

  dir = strdup(path);

  s = strrchr(dir, '/');
  if (s)
    s[1] = '\0';
  else
    dir[0] = '\0';

  return dir;
}


/* glmReadMTL: read a wavefront material library file
 *
 * model - properly initialized GLMmodel structure
 * name  - name of the material library
 */
static GLint
_glmReadMTL(GLMmodel* model, char* name)
{
  FILE* file;
  char* dir;
  char* filename;
  char    buf[128];
  GLuint nummaterials, i;

  dir = _glmDirName(model->pathname);
  filename = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(name) + 1));
  strcpy(filename, dir);
  strcat(filename, name);
  free(dir);

  file = fopen(filename, "r");
  if (!file) {
    error("_glmReadMTL() failed: can't open material file \"%s\".",filename);
    return -1;
  }
  free(filename);

  /* count the number of materials in the file */
  nummaterials = 1;
  while(fscanf(file, "%s", buf) != EOF) {
    switch(buf[0]) {
    case '#':               /* comment */
      /* eat up rest of line */
      if(NULL==fgets(buf, sizeof(buf), file)) {
        logpost(NULL, 5, "_glmReadMTL() failed reading comment"); continue;
      }
      break;
    case 'n':               /* newmtl */
      if(NULL==fgets(buf, sizeof(buf), file)) {
        error("_glmReadMTL() failed reading new material"); goto mtlread_failed;
      }
      nummaterials++;
      sscanf(buf, "%s %s", buf, buf);
      break;
    default:
      /* eat up rest of line */
      if(NULL==fgets(buf, sizeof(buf), file)) {
        logpost(NULL, 5, "_glmReadMTL() failed reading"); continue;
      }
      break;
    }
  }

  rewind(file);

  model->materials = (GLMmaterial*)malloc(sizeof(GLMmaterial) * nummaterials);
  model->nummaterials = nummaterials;

  /* set the default material */
  for (i = 0; i < nummaterials; i++) {
    model->materials[i].name = NULL;
    model->materials[i].shininess = 65.0f;
    model->materials[i].diffuse[0] = 0.8f;
    model->materials[i].diffuse[1] = 0.8f;
    model->materials[i].diffuse[2] = 0.8f;
    model->materials[i].diffuse[3] = 1.0f;
    model->materials[i].ambient[0] = 0.2f;
    model->materials[i].ambient[1] = 0.2f;
    model->materials[i].ambient[2] = 0.2f;
    model->materials[i].ambient[3] = 1.0f;
    model->materials[i].specular[0] = 0.0f;
    model->materials[i].specular[1] = 0.0f;
    model->materials[i].specular[2] = 0.0f;
    model->materials[i].specular[3] = 1.0f;
  }
  model->materials[0].name = strdup("default");

  /* now, read in the data */
  nummaterials = 0;
  while(fscanf(file, "%s", buf) != EOF) {
    switch(buf[0]) {
    case '#':               /* comment */
      /* eat up rest of line */
      if(NULL==fgets(buf, sizeof(buf), file)) {
        logpost(NULL, 5, "_glmReadMTL() really failed reading comment"); continue;
      }
      break;
    case 'n':               /* newmtl */
      if(NULL==fgets(buf, sizeof(buf), file)) {
        error("_glmReadMTL() really failed reading new material"); goto mtlread_failed;
      }
      sscanf(buf, "%s %s", buf, buf);
      nummaterials++;
      model->materials[nummaterials].name = strdup(buf);
      break;
    case 'N':
      fscanf(file, "%f", &model->materials[nummaterials].shininess);
      /* wavefront shininess is from [0, 1000], so scale for OpenGL */
      model->materials[nummaterials].shininess /= 1000.0;
      model->materials[nummaterials].shininess *= 128.0;
      break;
    case 'K':
      switch(buf[1]) {
      case 'd':
        fscanf(file, "%f %f %f",
               &model->materials[nummaterials].diffuse[0],
               &model->materials[nummaterials].diffuse[1],
               &model->materials[nummaterials].diffuse[2]);
        break;
      case 's':
        fscanf(file, "%f %f %f",
               &model->materials[nummaterials].specular[0],
               &model->materials[nummaterials].specular[1],
               &model->materials[nummaterials].specular[2]);
        break;
      case 'a':
        fscanf(file, "%f %f %f",
               &model->materials[nummaterials].ambient[0],
               &model->materials[nummaterials].ambient[1],
               &model->materials[nummaterials].ambient[2]);
        break;
      default:
        /* eat up rest of line */
        if(NULL==fgets(buf, sizeof(buf), file)) {
          logpost(NULL, 5, "_glmReadMTL() really failed reading K"); continue;
        }
        break;
      }
      break;
    default:
      /* eat up rest of line */
      if(NULL==fgets(buf, sizeof(buf), file)) {
        logpost(NULL, 5, "_glmReadMTL() really failed reading"); continue;
      }
      break;
    }
  }
  return 0;
 mtlread_failed:
  if(file)fclose(file);
  return -1;
}

/* glmWriteMTL: write a wavefront material library file
 *
 * model   - properly initialized GLMmodel structure
 * modelpath  - pathname of the model being written
 * mtllibname - name of the material library to be written
 */
static GLboolean
_glmWriteMTL(GLMmodel* model, char* modelpath, char* mtllibname)
{
  FILE* file;
  char* dir;
  char* filename;
  GLMmaterial* material;
  GLuint i;

  dir = _glmDirName(modelpath);
  filename = (char*)malloc(sizeof(char) * (strlen(dir)+strlen(mtllibname)));
  strcpy(filename, dir);
  strcat(filename, mtllibname);
  free(dir);

  /* open the file */
  file = fopen(filename, "w");
  if (!file) {
    error("_glmWriteMTL() failed: can't open file \"%s\".",filename);
    return GL_FALSE;
  }
  free(filename);

  /* spit out a header */
  fprintf(file, "#  \n");
  fprintf(file, "#  Wavefront MTL generated by GLM library\n");
  fprintf(file, "#  \n");
  fprintf(file, "#  GLM library\n");
  fprintf(file, "#  Nate Robins\n");
  fprintf(file, "#  ndr@pobox.com\n");
  fprintf(file, "#  http://www.pobox.com/~ndr\n");
  fprintf(file, "#  \n\n");

  for (i = 0; i < model->nummaterials; i++) {
    material = &model->materials[i];
    fprintf(file, "newmtl %s\n", material->name);
    fprintf(file, "Ka %f %f %f\n",
            material->ambient[0], material->ambient[1], material->ambient[2]);
    fprintf(file, "Kd %f %f %f\n",
            material->diffuse[0], material->diffuse[1], material->diffuse[2]);
    fprintf(file, "Ks %f %f %f\n",
            material->specular[0],material->specular[1],material->specular[2]);
    fprintf(file, "Ns %f\n", material->shininess / 128.0 * 1000.0);
    fprintf(file, "\n");
  }
  return GL_TRUE;
}


/* glmFirstPass: first pass at a Wavefront OBJ file that gets all the
 * statistics of the model (such as #vertices, #normals, etc)
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor
 */
static GLboolean
_glmFirstPass(GLMmodel* model, FILE* file)
{
  GLuint  numvertices;        /* number of vertices in model */
  GLuint  numnormals;         /* number of normals in model */
  GLuint  numtexcoords;       /* number of texcoords in model */
  GLuint  numtriangles;       /* number of triangles in model */
  GLMgroup* group;            /* current group */
  unsigned    v, n, t;
  char        buf[128];

  /* make a default group */
  group = _glmAddGroup(model, (char*)"default");

  numvertices = numnormals = numtexcoords = numtriangles = 0;
  while(fscanf(file, "%s", buf) != EOF) {
    switch(buf[0]) {
    case '#':               /* comment */
      /* eat up rest of line */
      if(NULL==fgets(buf, sizeof(buf), file)) {
        logpost(NULL, 5, "_glmFirstPass() failed reading comment"); continue;
      }
      break;
    case 'v':               /* v, vn, vt */
      switch(buf[1]) {
      case '\0':          /* vertex */
        /* eat up rest of line */
        if(NULL==fgets(buf, sizeof(buf), file)) {
          error("_glmFirstPass() failed reading vertex"); return GL_FALSE;
        }
        numvertices++;
        break;
      case 'n':           /* normal */
        /* eat up rest of line */
        if(NULL==fgets(buf, sizeof(buf), file)) {
          error("_glmFirstPass() failed reading normals"); return GL_FALSE;
        }
        numnormals++;
        break;
      case 't':           /* texcoord */
        /* eat up rest of line */
        if(NULL==fgets(buf, sizeof(buf), file)) {
          error("_glmFirstPass() failed reading texcoords"); return GL_FALSE;
        }
        numtexcoords++;
        break;
      default:
        error("_glmFirstPass(): Unknown token \"%s\".", buf);
        return -1;
        break;
      }
      break;
    case 'm':
      if(NULL==fgets(buf, sizeof(buf), file)) {
        error("_glmFirstPass() failed reading material"); return GL_FALSE;
      }
      sscanf(buf, "%s %s", buf, buf);
      model->mtllibname = strdup(buf);
      _glmReadMTL(model, buf);
      break;
    case 'u':
      /* eat up rest of line */
      if(NULL==fgets(buf, sizeof(buf), file)) {
        logpost(NULL, 5, "_glmFirstPass() failed reading u"); continue;
      }
      break;
    case 'g':               /* group */
      /* eat up rest of line */
      if(NULL==fgets(buf, sizeof(buf), file)) {
        error("_glmFirstPass() failed reading groups"); return GL_FALSE;
      }
#if SINGLE_STRING_GROUP_NAMES
      sscanf(buf, "%s", buf);
#else
      buf[strlen(buf)-1] = '\0';  /* nuke '\n' */
#endif
      group = _glmAddGroup(model, buf);
      break;
    case 'f':               /* face */
      v = n = t = 0;
      fscanf(file, "%s", buf);
      /* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
         if (strstr(buf, "//")) {
           /* v//n */
             sscanf(buf, "%d//%d", &v, &n);
           fscanf(file, "%d//%d", &v, &n);
           fscanf(file, "%d//%d", &v, &n);
           numtriangles++;
           group->numtriangles++;
           while(fscanf(file, "%d//%d", &v, &n) > 0) {
             numtriangles++;
             group->numtriangles++;
           }
         } else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
           /* v/t/n */
           fscanf(file, "%d/%d/%d", &v, &t, &n);
           fscanf(file, "%d/%d/%d", &v, &t, &n);
           numtriangles++;
           group->numtriangles++;
           while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
             numtriangles++;
             group->numtriangles++;
           }
         } else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
           /* v/t */
           fscanf(file, "%d/%d", &v, &t);
           fscanf(file, "%d/%d", &v, &t);
           numtriangles++;
           group->numtriangles++;
           while(fscanf(file, "%d/%d", &v, &t) > 0) {
             numtriangles++;
             group->numtriangles++;
           }
         } else {
           /* v */
           fscanf(file, "%d", &v);
           fscanf(file, "%d", &v);
           numtriangles++;
           group->numtriangles++;
           while(fscanf(file, "%d", &v) > 0) {
             numtriangles++;
             group->numtriangles++;
           }
         }
      break;

    default:
      /* eat up rest of line */
      if(NULL==fgets(buf, sizeof(buf), file)) {
        logpost(NULL, 5, "_glmFirstPass() failed reading"); continue;
      }
      break;
    }
  }

  /* set the stats in the model structure */
  model->numvertices  = numvertices;
  model->numnormals   = numnormals;
  model->numuvtexcoords = numtexcoords;
  model->numtriangles = numtriangles;

  /* allocate memory for the triangles in each group */
  group = model->groups;
  while(group) {
    group->triangles = (GLuint*)malloc(sizeof(GLuint) * group->numtriangles);
    group->numtriangles = 0;
    group = group->next;
  }
  return GL_TRUE;
}

static GLuint fixIndex(GLint current, GLuint baseindex) {
  GLint idx=baseindex;

  /* i'm not sure whether >=0 or >0 is valid...
   * for now, i assume >0 (so if current==0 it resolves to baseindex)
   */

  if(current>0)
    return (GLuint)(current);
 
  idx=(baseindex+current);
  if(idx>0)
    return (GLuint)idx;
  else {
    logpost(NULL, 5, "unable to fix negative index %d @ %d", current, baseindex);
    return baseindex;
  }
}

/* glmSecondPass: second pass at a Wavefront OBJ file that gets all
 * the data.
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor
 */
static GLboolean
_glmSecondPass(GLMmodel* model, FILE* file)
{
  GLuint  numvertices;        /* number of vertices in model */
  GLuint  numnormals;         /* number of normals in model */
  GLuint  numtexcoords;       /* number of texcoords in model */
  GLuint  numtriangles;       /* number of triangles in model */
  GLfloat*    vertices;           /* array of vertices  */
  GLfloat*    normals;            /* array of normals */
  GLfloat*    texcoords;          /* array of texture coordinates */
  GLMgroup* group;            /* current group pointer */
  GLuint  material;           /* current material */
  int  v, n, t;
  char        buf[128];

  /* set the pointer shortcuts */
  vertices       = model->vertices;
  normals    = model->normals;
  texcoords    = model->uvtexcoords;
  group      = model->groups;

  /* on the second pass through the file, read all the data into the
     allocated arrays */
  numvertices = numnormals = numtexcoords = 1;
  numtriangles = 0;
  material = 0;
  while(fscanf(file, "%s", buf) != EOF) {
    switch(buf[0]) {
    case '#':               /* comment */
      /* eat up rest of line */
      if(NULL==fgets(buf, sizeof(buf), file)) {
        logpost(NULL, 5, "_glmSecondPass() failed reading"); continue;
      }
      break;
    case 'v':               /* v, vn, vt */
      switch(buf[1]) {
      case '\0':          /* vertex */
        fscanf(file, "%f %f %f",
               &vertices[3 * numvertices + 0],
               &vertices[3 * numvertices + 1],
               &vertices[3 * numvertices + 2]);
        numvertices++;
        break;
      case 'n':           /* normal */
        fscanf(file, "%f %f %f",
               &normals[3 * numnormals + 0],
               &normals[3 * numnormals + 1],
               &normals[3 * numnormals + 2]);
        numnormals++;
        break;
      case 't':           /* texcoord */
        fscanf(file, "%f %f",
               &texcoords[2 * numtexcoords + 0],
               &texcoords[2 * numtexcoords + 1]);
        numtexcoords++;
        break;
      }
      break;
    case 'u':
      if(NULL==fgets(buf, sizeof(buf), file)) {
        error("_glmSecondPass() failed reading material"); return GL_FALSE;
      }
      sscanf(buf, "%s %s", buf, buf);
      group->material = material = _glmFindMaterial(model, buf);
      break;
    case 'g':               /* group */
      /* eat up rest of line */
      if(NULL==fgets(buf, sizeof(buf), file)) {
        error("_glmSecondPass() failed reading group"); return GL_FALSE;
      }
#if SINGLE_STRING_GROUP_NAMES
      sscanf(buf, "%s", buf);
#else
      buf[strlen(buf)-1] = '\0';  /* nuke '\n' */
#endif
      group = _glmFindGroup(model, buf);
      group->material = material;
      break;
    case 'f':               /* face */
      v = n = t = 0;
      fscanf(file, "%s", buf);
      /* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
         if (strstr(buf, "//")) {
           /* v//n */
             sscanf(buf, "%d//%d", &v, &n);
           T(numtriangles).vindices[0] = fixIndex(v,numvertices);
           T(numtriangles).nindices[0] = fixIndex(n,numnormals);
           fscanf(file, "%d//%d", &v, &n);
           T(numtriangles).vindices[1] = fixIndex(v,numvertices);
           T(numtriangles).nindices[1] = fixIndex(n, numnormals);
           fscanf(file, "%d//%d", &v, &n);
           T(numtriangles).vindices[2] = fixIndex(v,numvertices);
           T(numtriangles).nindices[2] = fixIndex(n, numnormals);
           group->triangles[group->numtriangles++] = numtriangles;
           numtriangles++;
           while(fscanf(file, "%d//%d", &v, &n) > 0) {
             T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
             T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
             T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
             T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
             T(numtriangles).vindices[2] = fixIndex(v,numvertices);
             T(numtriangles).nindices[2] = fixIndex(n, numnormals);
             group->triangles[group->numtriangles++] = numtriangles;
             numtriangles++;
           }
         } else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
           /* v/t/n */
           T(numtriangles).vindices[0] = fixIndex(v,numvertices);
           T(numtriangles).uvtindices[0] = fixIndex(t, numtexcoords);
           T(numtriangles).nindices[0] = fixIndex(n, numnormals);
           fscanf(file, "%d/%d/%d", &v, &t, &n);
           T(numtriangles).vindices[1] = fixIndex(v, numvertices);
           T(numtriangles).uvtindices[1] = fixIndex(t, numtexcoords);
           T(numtriangles).nindices[1] = fixIndex(n, numnormals);
           fscanf(file, "%d/%d/%d", &v, &t, &n);
           T(numtriangles).vindices[2] = fixIndex(v, numvertices);
           T(numtriangles).uvtindices[2] = fixIndex(t, numtexcoords);
           T(numtriangles).nindices[2] = fixIndex(n, numnormals);
           group->triangles[group->numtriangles++] = numtriangles;
           numtriangles++;
           while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
             T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
             T(numtriangles).uvtindices[0] = T(numtriangles-1).uvtindices[0];
             T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
             T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
             T(numtriangles).uvtindices[1] = T(numtriangles-1).uvtindices[2];
             T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
             T(numtriangles).vindices[2] = fixIndex(v, numvertices);
             T(numtriangles).uvtindices[2] = fixIndex(t, numtexcoords);
             T(numtriangles).nindices[2] = fixIndex(n, numnormals);
             group->triangles[group->numtriangles++] = numtriangles;
             numtriangles++;
           }
         } else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
           /* v/t */
           T(numtriangles).vindices[0] = fixIndex(v, numvertices);
           T(numtriangles).uvtindices[0] = fixIndex(t, numtexcoords);
           fscanf(file, "%d/%d", &v, &t);
           T(numtriangles).vindices[1] = fixIndex(v, numvertices);
           T(numtriangles).uvtindices[1] = fixIndex(t, numtexcoords);
           fscanf(file, "%d/%d", &v, &t);
           T(numtriangles).vindices[2] = fixIndex(v, numvertices);
           T(numtriangles).uvtindices[2] = fixIndex(t, numtexcoords);
           group->triangles[group->numtriangles++] = numtriangles;
           numtriangles++;
           while(fscanf(file, "%d/%d", &v, &t) > 0) {
             T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
             T(numtriangles).uvtindices[0] = T(numtriangles-1).uvtindices[0];
             T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
             T(numtriangles).uvtindices[1] = T(numtriangles-1).uvtindices[2];
             T(numtriangles).vindices[2] = fixIndex(v, numvertices);
             T(numtriangles).uvtindices[2] = fixIndex(t, numtexcoords);
             group->triangles[group->numtriangles++] = numtriangles;
             numtriangles++;
           }
         } else {
           /* v */
           sscanf(buf, "%d", &v);
           T(numtriangles).vindices[0] = fixIndex(v, numvertices);
           fscanf(file, "%d", &v);
           T(numtriangles).vindices[1] = fixIndex(v, numvertices);
           fscanf(file, "%d", &v);
           T(numtriangles).vindices[2] = fixIndex(v, numvertices);
           group->triangles[group->numtriangles++] = numtriangles;
           numtriangles++;
           while(fscanf(file, "%d", &v) > 0) {
             T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
             T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
             T(numtriangles).vindices[2] = fixIndex(v, numvertices);
             group->triangles[group->numtriangles++] = numtriangles;
             numtriangles++;
           }
         }
      break;

    default:
      /* eat up rest of line */
      if(NULL==fgets(buf, sizeof(buf), file)) {
        logpost(NULL, 5, "_glmSecondPass() failed reading"); continue;
      }
      break;
    }
  }

  /* announce the memory requirements */
  logpost(NULL, 5, " Memory: %d bytes",
          numvertices  * 3*sizeof(GLfloat) +
          numnormals   * 3*sizeof(GLfloat) * (numnormals ? 1 : 0) +
          numtexcoords * 3*sizeof(GLfloat) * (numtexcoords ? 1 : 0) +
          numtriangles * sizeof(GLMtriangle));

  return GL_TRUE;
}


/* public functions */


/* glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin.   Returns the
 * scalefactor used.
 *
 * model - properly initialized GLMmodel structure
 */
GLfloat
glmUnitize(GLMmodel* model)
{
  GLuint  i;
  GLfloat maxx, minx, maxy, miny, maxz, minz;
  GLfloat cx, cy, cz, w, h, d;
  GLfloat scale;

  if (!(model))return 0.f;
  if (!(model->vertices))return 0.f;

  /* get the max/mins */
  maxx = minx = model->vertices[3 + 0];
  maxy = miny = model->vertices[3 + 1];
  maxz = minz = model->vertices[3 + 2];
  for (i = 1; i <= model->numvertices; i++) {
    if (maxx < model->vertices[3 * i + 0])
      maxx = model->vertices[3 * i + 0];
    if (minx > model->vertices[3 * i + 0])
      minx = model->vertices[3 * i + 0];

    if (maxy < model->vertices[3 * i + 1])
      maxy = model->vertices[3 * i + 1];
    if (miny > model->vertices[3 * i + 1])
      miny = model->vertices[3 * i + 1];

    if (maxz < model->vertices[3 * i + 2])
      maxz = model->vertices[3 * i + 2];
    if (minz > model->vertices[3 * i + 2])
      minz = model->vertices[3 * i + 2];
  }

  /* calculate model width, height, and depth */
  w = _glmAbs(maxx) + _glmAbs(minx);
  h = _glmAbs(maxy) + _glmAbs(miny);
  d = _glmAbs(maxz) + _glmAbs(minz);

  /* calculate center of the model */
  cx = (maxx + minx) / 2.0f;
  cy = (maxy + miny) / 2.0f;
  cz = (maxz + minz) / 2.0f;

  /* calculate unitizing scale factor */
  scale = 2.0f / _glmMax(_glmMax(w, h), d);

  /* translate around center then scale */
  for (i = 1; i <= model->numvertices; i++) {
    model->vertices[3 * i + 0] -= cx;
    model->vertices[3 * i + 1] -= cy;
    model->vertices[3 * i + 2] -= cz;
    model->vertices[3 * i + 0] *= scale;
    model->vertices[3 * i + 1] *= scale;
    model->vertices[3 * i + 2] *= scale;
  }

  return scale;
}

/* glmDimensions: Calculates the dimensions (width, height, depth) of
 * a model.
 *
 * model   - initialized GLMmodel structure
 * dimensions - array of 3 GLfloats (GLfloat dimensions[3])
 */
GLvoid
glmDimensions(GLMmodel* model, GLfloat* dimensions)
{
  GLuint i;
  GLfloat maxx, minx, maxy, miny, maxz, minz;

  if (!(model))return;
  if (!(model->vertices))return;
  if (!(dimensions))return;

  /* get the max/mins */
  maxx = minx = model->vertices[3 + 0];
  maxy = miny = model->vertices[3 + 1];
  maxz = minz = model->vertices[3 + 2];
  for (i = 1; i <= model->numvertices; i++) {
    if (maxx < model->vertices[3 * i + 0])
      maxx = model->vertices[3 * i + 0];
    if (minx > model->vertices[3 * i + 0])
      minx = model->vertices[3 * i + 0];

    if (maxy < model->vertices[3 * i + 1])
      maxy = model->vertices[3 * i + 1];
    if (miny > model->vertices[3 * i + 1])
      miny = model->vertices[3 * i + 1];

    if (maxz < model->vertices[3 * i + 2])
      maxz = model->vertices[3 * i + 2];
    if (minz > model->vertices[3 * i + 2])
      minz = model->vertices[3 * i + 2];
  }

  /* calculate model width, height, and depth */
  dimensions[0] = _glmAbs(maxx) + _glmAbs(minx);
  dimensions[1] = _glmAbs(maxy) + _glmAbs(miny);
  dimensions[2] = _glmAbs(maxz) + _glmAbs(minz);
}

/* glmScale: Scales a model by a given amount.
 *
 * model - properly initialized GLMmodel structure
 * scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
 */
GLvoid
glmScale(GLMmodel* model, GLfloat scale)
{
  GLuint i;

  for (i = 1; i <= model->numvertices; i++) {
    model->vertices[3 * i + 0] *= scale;
    model->vertices[3 * i + 1] *= scale;
    model->vertices[3 * i + 2] *= scale;
  }
}

/* glmReverseWinding: Reverse the polygon winding for all polygons in
 * this model.   Default winding is counter-clockwise.  Also changes
 * the direction of the normals.
 *
 * model - properly initialized GLMmodel structure
 */
GLvoid
glmReverseWinding(GLMmodel* model)
{
  GLuint i, swap;

  if (!(model))return;

  for (i = 0; i < model->numtriangles; i++) {
    swap = T(i).vindices[0];
    T(i).vindices[0] = T(i).vindices[2];
    T(i).vindices[2] = swap;

    if (model->numnormals) {
      swap = T(i).nindices[0];
      T(i).nindices[0] = T(i).nindices[2];
      T(i).nindices[2] = swap;
    }

    if (model->numtexcoords) {
      swap = T(i).tindices[0];
      T(i).tindices[0] = T(i).tindices[2];
      T(i).tindices[2] = swap;
    }
  }

  /* reverse facet normals */
  for (i = 1; i <= model->numfacetnorms; i++) {
    model->facetnorms[3 * i + 0] = -model->facetnorms[3 * i + 0];
    model->facetnorms[3 * i + 1] = -model->facetnorms[3 * i + 1];
    model->facetnorms[3 * i + 2] = -model->facetnorms[3 * i + 2];
  }

  /* reverse vertex normals */
  for (i = 1; i <= model->numnormals; i++) {
    model->normals[3 * i + 0] = -model->normals[3 * i + 0];
    model->normals[3 * i + 1] = -model->normals[3 * i + 1];
    model->normals[3 * i + 2] = -model->normals[3 * i + 2];
  }
}

/* glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle).  Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
GLvoid
glmFacetNormals(GLMmodel* model)
{
  GLuint  i;
  GLfloat u[3];
  GLfloat v[3];

  if (!(model))return;
  if (!(model->vertices))return;

  /* clobber any old facetnormals */
  if (model->facetnorms)
    free(model->facetnorms);

  /* allocate memory for the new facet normals */
  model->numfacetnorms = model->numtriangles;
  model->facetnorms = (GLfloat*)malloc(sizeof(GLfloat) *
                                       3 * (model->numfacetnorms + 1));

  for (i = 0; i < model->numtriangles; i++) {
    model->triangles[i].findex = i+1;

    u[0] = model->vertices[3 * T(i).vindices[1] + 0] -
      model->vertices[3 * T(i).vindices[0] + 0];
    u[1] = model->vertices[3 * T(i).vindices[1] + 1] -
      model->vertices[3 * T(i).vindices[0] + 1];
    u[2] = model->vertices[3 * T(i).vindices[1] + 2] -
      model->vertices[3 * T(i).vindices[0] + 2];

    v[0] = model->vertices[3 * T(i).vindices[2] + 0] -
      model->vertices[3 * T(i).vindices[0] + 0];
    v[1] = model->vertices[3 * T(i).vindices[2] + 1] -
      model->vertices[3 * T(i).vindices[0] + 1];
    v[2] = model->vertices[3 * T(i).vindices[2] + 2] -
      model->vertices[3 * T(i).vindices[0] + 2];

    _glmCross(u, v, &model->facetnorms[3 * (i+1)]);
    _glmNormalize(&model->facetnorms[3 * (i+1)]);
  }
}

/* glmVertexNormals: Generates smooth vertex normals for a model.
 * First builds a list of all the triangles each vertex is in.   Then
 * loops through each vertex in the the list averaging all the facet
 * normals of the triangles each vertex is in.   Finally, sets the
 * normal index in the triangle for the vertex to the generated smooth
 * normal.   If the dot product of a facet normal and the facet normal
 * associated with the first triangle in the list of triangles the
 * current vertex is in is greater than the cosine of the angle
 * parameter to the function, that facet normal is not added into the
 * average normal calculation and the corresponding vertex is given
 * the facet normal.  This tends to preserve hard edges.  The angle to
 * use depends on the model, but 90 degrees is usually a good start.
 *
 * model - initialized GLMmodel structure
 * angle - maximum angle (in degrees) to smooth across
 */
GLvoid
glmVertexNormals(GLMmodel* model, GLfloat angle)
{
  GLMnode*    node;
  GLMnode*    tail;
  GLMnode** members;
  GLfloat*    normals;
  GLuint  numnormals;
  GLfloat average[3];
  GLfloat dot, cos_angle;
  GLuint  i, avg;

  if (!(model))return;
  if (!(model->facetnorms))return;

  /* calculate the cosine of the angle (in degrees) */
  cos_angle = (GLfloat)cos(angle * M_PI / 180.0f);

  /* nuke any previous normals */
  if (model->normals)
    free(model->normals);

  /* allocate space for new normals */
  model->numnormals = model->numtriangles * 3; /* 3 normals per triangle */
  model->normals = (GLfloat*)malloc(sizeof(GLfloat)* 3* (model->numnormals+1));

  /* allocate a structure that will hold a linked list of triangle
     indices for each vertex */
  members = (GLMnode**)malloc(sizeof(GLMnode*) * (model->numvertices + 1));
  for (i = 1; i <= model->numvertices; i++)
    members[i] = NULL;

  /* for every triangle, create a node for each vertex in it */
  for (i = 0; i < model->numtriangles; i++) {
    node = (GLMnode*)malloc(sizeof(GLMnode));
    node->index = i;
    node->next  = members[T(i).vindices[0]];
    members[T(i).vindices[0]] = node;

    node = (GLMnode*)malloc(sizeof(GLMnode));
    node->index = i;
    node->next  = members[T(i).vindices[1]];
    members[T(i).vindices[1]] = node;

    node = (GLMnode*)malloc(sizeof(GLMnode));
    node->index = i;
    node->next  = members[T(i).vindices[2]];
    members[T(i).vindices[2]] = node;
  }

  /* calculate the average normal for each vertex */
  numnormals = 1;
  for (i = 1; i <= model->numvertices; i++) {
    /* calculate an average normal for this vertex by averaging the
       facet normal of every triangle this vertex is in */
    node = members[i];
    if (!node)
      error("glmVertexNormals(): vertex w/o a triangle");
    average[0] = 0.0; average[1] = 0.0; average[2] = 0.0;
    avg = 0;
    while (node) {
      /* only average if the dot product of the angle between the two
         facet normals is greater than the cosine of the threshold
         angle -- or, said another way, the angle between the two
         facet normals is less than (or equal to) the threshold angle */
      dot = _glmDot(&model->facetnorms[3 * T(node->index).findex],
                    &model->facetnorms[3 * T(members[i]->index).findex]);
      if (dot > cos_angle) {
        node->averaged = GL_TRUE;
        average[0] += model->facetnorms[3 * T(node->index).findex + 0];
        average[1] += model->facetnorms[3 * T(node->index).findex + 1];
        average[2] += model->facetnorms[3 * T(node->index).findex + 2];
        avg = 1;            /* we averaged at least one normal! */
      } else {
        node->averaged = GL_FALSE;
      }
      node = node->next;
    }

    if (avg) {
      /* normalize the averaged normal */
      _glmNormalize(average);

      /* add the normal to the vertex normals list */
      model->normals[3 * numnormals + 0] = average[0];
      model->normals[3 * numnormals + 1] = average[1];
      model->normals[3 * numnormals + 2] = average[2];
      avg = numnormals;
      numnormals++;
    }

    /* set the normal of this vertex in each triangle it is in */
    node = members[i];
    while (node) {
      if (node->averaged) {
        /* if this node was averaged, use the average normal */
        if (T(node->index).vindices[0] == i)
          T(node->index).nindices[0] = avg;
        else if (T(node->index).vindices[1] == i)
          T(node->index).nindices[1] = avg;
        else if (T(node->index).vindices[2] == i)
          T(node->index).nindices[2] = avg;
      } else {
        /* if this node wasn't averaged, use the facet normal */
        model->normals[3 * numnormals + 0] =
          model->facetnorms[3 * T(node->index).findex + 0];
        model->normals[3 * numnormals + 1] =
          model->facetnorms[3 * T(node->index).findex + 1];
        model->normals[3 * numnormals + 2] =
          model->facetnorms[3 * T(node->index).findex + 2];
        if (T(node->index).vindices[0] == i)
          T(node->index).nindices[0] = numnormals;
        else if (T(node->index).vindices[1] == i)
          T(node->index).nindices[1] = numnormals;
        else if (T(node->index).vindices[2] == i)
          T(node->index).nindices[2] = numnormals;
        numnormals++;
      }
      node = node->next;
    }
  }

  model->numnormals = numnormals - 1;

  /* free the member information */
  for (i = 1; i <= model->numvertices; i++) {
    node = members[i];
    while (node) {
      tail = node;
      node = node->next;
      free(tail);
    }
  }
  free(members);

  /* pack the normals array (we previously allocated the maximum
     number of normals that could possibly be created (numtriangles *
     3), so get rid of some of them (usually alot unless none of the
     facet normals were averaged)) */
  normals = model->normals;
  model->normals = (GLfloat*)malloc(sizeof(GLfloat)* 3* (model->numnormals+1));
  for (i = 1; i <= model->numnormals; i++) {
    model->normals[3 * i + 0] = normals[3 * i + 0];
    model->normals[3 * i + 1] = normals[3 * i + 1];
    model->normals[3 * i + 2] = normals[3 * i + 2];
  }
  free(normals);
}

/* glmUVTexture: Generates texture coordinates according to a
 * the texture coordinates stored in the .obj file
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid
glmUVTexture(GLMmodel* model, float h, float w)
{
  GLMgroup *group;
  GLuint i;

  if(!model)return;
  if(!(model->uvtexcoords))return;

  if (model->texcoords)
    free(model->texcoords);

  model->numtexcoords = model->numuvtexcoords;
  model->texcoords=(GLfloat*)malloc(sizeof(GLfloat)*2*(model->numuvtexcoords+1));

  /* do the calculations */
  for(i = 1; i <= model->numtexcoords; i++) {
    model->texcoords[2*i+0] = model->uvtexcoords[2*i+0]*w;
    model->texcoords[2*i+1] = model->uvtexcoords[2*i+1]*h;
  }

  /* go through and put texture coordinate indices in all the triangles */
  group = model->groups;
  while(group) {
    for(i = 0; i < group->numtriangles; i++) {
      T(group->triangles[i]).tindices[0] = T(group->triangles[i]).uvtindices[0];
      T(group->triangles[i]).tindices[1] = T(group->triangles[i]).uvtindices[1];
      T(group->triangles[i]).tindices[2] = T(group->triangles[i]).uvtindices[2];
    }
    group = group->next;
  }
  logpost(NULL, 5, "glmUVTexture(): generated %d UV texture coordinates", model->numtexcoords);
}

/* glmLinearTexture: Generates texture coordinates according to a
 * linear projection of the texture map.  It generates these by
 * linearly mapping the vertices onto a square.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid
glmLinearTexture(GLMmodel* model, float h, float w)
{
  GLMgroup *group;
  GLfloat dimensions[3];
  GLfloat x, y, scalefactor;
  GLuint i;

  if (!(model))return;

  if (model->texcoords)
    free(model->texcoords);
  model->numtexcoords = model->numvertices;
  model->texcoords=(GLfloat*)malloc(sizeof(GLfloat)*2*(model->numtexcoords+1));

  glmDimensions(model, dimensions);
  scalefactor = 2.0f /
    _glmAbs(_glmMax(_glmMax(dimensions[0], dimensions[1]), dimensions[2]));

  /* do the calculations */
  for(i = 1; i <= model->numvertices; i++) {
    x = model->vertices[3 * i + 0] * scalefactor;
    y = model->vertices[3 * i + 2] * scalefactor;
    model->texcoords[2 * i + 0] = ((x + 1.0f) / 2.0f) * w;
    model->texcoords[2 * i + 1] = ((y + 1.0f) / 2.0f) * h;
  }

  /* go through and put texture coordinate indices in all the triangles */
  group = model->groups;
  while(group) {
    for(i = 0; i < group->numtriangles; i++) {
      T(group->triangles[i]).tindices[0] = T(group->triangles[i]).vindices[0];
      T(group->triangles[i]).tindices[1] = T(group->triangles[i]).vindices[1];
      T(group->triangles[i]).tindices[2] = T(group->triangles[i]).vindices[2];
    }
    group = group->next;
  }

  logpost(NULL, 5, "glmLinearTexture(): generated %d linear texture coordinates", model->numtexcoords);
}

/* glmSpheremapTexture: Generates texture coordinates according to a
 * spherical projection of the texture map.  Sometimes referred to as
 * spheremap, or reflection map texture coordinates.  It generates
 * these by using the normal to calculate where that vertex would map
 * onto a sphere.  Since it is impossible to map something flat
 * perfectly onto something spherical, there is distortion at the
 * poles.  This particular implementation causes the poles along the X
 * axis to be distorted.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid
glmSpheremapTexture(GLMmodel* model, float h, float w)
{
  GLMgroup* group;
  GLfloat theta, phi, rho, x, y, z, r;
  GLuint i;

  if (!(model))return;
  if (!(model->normals))return;

  if (model->texcoords)
    free(model->texcoords);
  model->numtexcoords = model->numnormals;
  model->texcoords=(GLfloat*)malloc(sizeof(GLfloat)*2*(model->numtexcoords+1));

  for (i = 1; i <= model->numnormals; i++) {
    z = model->normals[3 * i + 0];  /* re-arrange for pole distortion */
    y = model->normals[3 * i + 1];
    x = model->normals[3 * i + 2];
    r = (GLfloat)sqrt((x * x) + (y * y));
    rho = (GLfloat)sqrt((r * r) + (z * z));

    if(r == 0.0) {
      theta = 0.0f;
      phi = 0.0f;
    } else {
      if(z == 0.0)
        phi = M_PI / 2.0f;
      else
        phi = (GLfloat)acos(z / rho);

      if(y == 0.0)
        theta = M_PI / 2.0f;
      else
        theta = (GLfloat)asin(y / r) + (M_PI / 2.0f);
    }

    model->texcoords[2 * i + 0] = w * theta / M_PI;
    model->texcoords[2 * i + 1] = h * phi / M_PI;
  }

  /* go through and put texcoord indices in all the triangles */
  group = model->groups;
  while(group) {
    for (i = 0; i < group->numtriangles; i++) {
      T(group->triangles[i]).tindices[0] = T(group->triangles[i]).nindices[0];
      T(group->triangles[i]).tindices[1] = T(group->triangles[i]).nindices[1];
      T(group->triangles[i]).tindices[2] = T(group->triangles[i]).nindices[2];
    }
    group = group->next;
  }

  logpost(NULL, 5, "glmSpheremapTexture(): generated %d spheremap texture coordinates", model->numtexcoords);
}

GLvoid glmTexture(GLMmodel* model, glmtexture_t typ, float h, float w)
{
  if(!model)return;
  switch(typ) {
  case GLM_TEX_UV: glmUVTexture(model, h, w); break;
  case GLM_TEX_LINEAR: glmLinearTexture(model, h, w); break;
  case GLM_TEX_SPHEREMAP: glmSpheremapTexture(model, h, w); break;
  default:
    if(model->numuvtexcoords)
      glmUVTexture(model, h, w);
    else
      glmLinearTexture(model, h, w);
  }
}

/* glmDelete: Deletes a GLMmodel structure.
 *
 * model - initialized GLMmodel structure
 */
GLvoid
glmDelete(GLMmodel* model)
{
  GLMgroup* group;
  GLuint i;

  if (!(model))return;

  if (model->pathname)     free(model->pathname);
  if (model->mtllibname) free(model->mtllibname);
  if (model->vertices)     free(model->vertices);
  if (model->normals)  free(model->normals);
  if (model->texcoords)  free(model->texcoords);
  if (model->facetnorms) free(model->facetnorms);
  if (model->triangles)  free(model->triangles);
  if (model->materials) {
    for (i = 0; i < model->nummaterials; i++)
      free(model->materials[i].name);
  }
  free(model->materials);
  while(model->groups) {
    group = model->groups;
    model->groups = model->groups->next;
    free(group->name);
    free(group->triangles);
    free(group);
  }

  free(model);
}

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be free'd with
 * glmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.
 */
GLMmodel*
glmReadOBJ(char* filename)
{
  GLMmodel* model;
  FILE*   file;

  /* open the file */
  file = fopen(filename, "r");
  if (!file) {
    error("glmReadOBJ() failed: can't open data file \"%s\".", filename);
    return NULL;
  }

  /* allocate a new model */
  model = (GLMmodel*)malloc(sizeof(GLMmodel));
  model->pathname    = strdup(filename);
  model->mtllibname    = NULL;
  model->numvertices   = 0;
  model->vertices    = NULL;
  model->numnormals    = 0;
  model->normals     = NULL;
  model->numuvtexcoords  = 0;
  model->uvtexcoords       = NULL;
  model->numtexcoords  = 0;
  model->texcoords       = NULL;
  model->numfacetnorms = 0;
  model->facetnorms    = NULL;
  model->numtriangles  = 0;
  model->triangles       = NULL;
  model->nummaterials  = 0;
  model->materials       = NULL;
  model->numgroups       = 0;
  model->groups      = NULL;
  model->position[0]   = 0.0;
  model->position[1]   = 0.0;
  model->position[2]   = 0.0;

  /* make a first pass through the file to get a count of the number
     of vertices, normals, texcoords & triangles */
  if(GL_FALSE==_glmFirstPass(model, file)){
    error("glmReadOBJ() failed: can't parse file \"%s\".", filename);
    goto readobj_failed;
  }

  /* allocate memory */
  model->vertices = (GLfloat*)malloc(sizeof(GLfloat) *
                                     3 * (model->numvertices + 1));
  model->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) *
                                          model->numtriangles);
  if (model->numnormals) {
    model->normals = (GLfloat*)malloc(sizeof(GLfloat) *
                                      3 * (model->numnormals + 1));
  }
  if (model->numuvtexcoords) {
    model->uvtexcoords = (GLfloat*)malloc(sizeof(GLfloat) *
                                          2 * (model->numuvtexcoords + 1));
  }

  /* rewind to beginning of file and read in the data this pass */
  rewind(file);

  if(GL_FALSE==_glmSecondPass(model, file)) {
    error("glmReadOBJ() failed: can't parse file \"%s\".", filename);
    goto readobj_failed;
  }

  /* close the file */
  fclose(file);

  glmTexture(model, GLM_TEX_DEFAULT, 1.0, 1.0);

  return model;

 readobj_failed:
  glmDelete(model);
  if(file)fclose(file);
  return NULL;
}

/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
 * a file.
 *
 * model - initialized GLMmodel structure
 * filename - name of the file to write the Wavefront .OBJ format data to
 * mode  - a bitwise or of values describing what is written to the file
 *             GLM_NONE     -  render with only vertices
 *             GLM_FLAT     -  render with facet normals
 *             GLM_SMOOTH   -  render with vertex normals
 *             GLM_TEXTURE  -  render with texture coords
 *             GLM_COLOR    -  render with colors (color material)
 *             GLM_MATERIAL -  render with materials
 *             GLM_COLOR and GLM_MATERIAL should not both be specified.
 *             GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLint
glmWriteOBJ(GLMmodel* model, char* filename, GLuint mode)
{
  GLuint  i;
  FILE*   file;
  GLMgroup* group;

  if (!(model))return -1;

  /* do a bit of warning */
  if (mode & GLM_FLAT && !model->facetnorms) {
    logpost(NULL, 5,"glmWriteOBJ() warning: flat normal output requested "
            "with no facet normals defined.");
    mode &= ~GLM_FLAT;
  }
  if (mode & GLM_SMOOTH && !model->normals) {
    logpost(NULL, 5,"glmWriteOBJ() warning: smooth normal output requested "
            "with no normals defined.");
    mode &= ~GLM_SMOOTH;
  }
  if (mode & GLM_TEXTURE && !model->texcoords) {
    logpost(NULL, 5,"glmWriteOBJ() warning: texture coordinate output requested "
            "with no texture coordinates defined.");
    mode &= ~GLM_TEXTURE;
  }
  if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
    logpost(NULL, 5,"glmWriteOBJ() warning: flat normal output requested "
            "and smooth normal output requested (using smooth).");
    mode &= ~GLM_FLAT;
  }
  if (mode & GLM_COLOR && !model->materials) {
    logpost(NULL, 5,"glmWriteOBJ() warning: color output requested "
            "with no colors (materials) defined.");
    mode &= ~GLM_COLOR;
  }
  if (mode & GLM_MATERIAL && !model->materials) {
    logpost(NULL, 5,"glmWriteOBJ() warning: material output requested "
            "with no materials defined.");
    mode &= ~GLM_MATERIAL;
  }
  if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
    logpost(NULL, 5,"glmWriteOBJ() warning: color and material output requested "
            "outputting only materials.");
    mode &= ~GLM_COLOR;
  }


  /* open the file */
  file = fopen(filename, "w");
  if (!file) {
    error("glmWriteOBJ() failed: can't open file \"%s\" to write.", filename);
    return -1;
  }

  /* spit out a header */
  fprintf(file, "#  \n");
  fprintf(file, "#  Wavefront OBJ generated by GLM library\n");
  fprintf(file, "#  \n");
  fprintf(file, "#  GLM library\n");
  fprintf(file, "#  Nate Robins\n");
  fprintf(file, "#  ndr@pobox.com\n");
  fprintf(file, "#  http://www.pobox.com/~ndr\n");
  fprintf(file, "#  \n");

  if (mode & GLM_MATERIAL && model->mtllibname) {
    fprintf(file, "\nmtllib %s\n\n", model->mtllibname);
    _glmWriteMTL(model, filename, model->mtllibname);
  }

  /* spit out the vertices */
  fprintf(file, "\n");
  fprintf(file, "# %d vertices\n", model->numvertices);
  for (i = 1; i <= model->numvertices; i++) {
    fprintf(file, "v %f %f %f\n",
            model->vertices[3 * i + 0],
            model->vertices[3 * i + 1],
            model->vertices[3 * i + 2]);
  }

  /* spit out the smooth/flat normals */
  if (mode & GLM_SMOOTH) {
    fprintf(file, "\n");
    fprintf(file, "# %d normals\n", model->numnormals);
    for (i = 1; i <= model->numnormals; i++) {
      fprintf(file, "vn %f %f %f\n",
              model->normals[3 * i + 0],
              model->normals[3 * i + 1],
              model->normals[3 * i + 2]);
    }
  } else if (mode & GLM_FLAT) {
    fprintf(file, "\n");
    fprintf(file, "# %d normals\n", model->numfacetnorms);
    for (i = 1; i <= model->numnormals; i++) {
      fprintf(file, "vn %f %f %f\n",
              model->facetnorms[3 * i + 0],
              model->facetnorms[3 * i + 1],
              model->facetnorms[3 * i + 2]);
    }
  }

  /* spit out the texture coordinates */
  if (mode & GLM_TEXTURE) {
    fprintf(file, "\n");
    fprintf(file, "# %d texcoords\n", model->numtexcoords);
    for (i = 1; i <= model->numtexcoords; i++) {
      fprintf(file, "vt %f %f\n",
              model->texcoords[2 * i + 0],
              model->texcoords[2 * i + 1]);
    }
  }

  fprintf(file, "\n");
  fprintf(file, "# %d groups\n", model->numgroups);
  fprintf(file, "# %d faces (triangles)\n", model->numtriangles);
  fprintf(file, "\n");

  group = model->groups;
  while(group) {
    fprintf(file, "g %s\n", group->name);
    if (mode & GLM_MATERIAL)
      fprintf(file, "usemtl %s\n", model->materials[group->material].name);
    for (i = 0; i < group->numtriangles; i++) {
      if (mode & GLM_SMOOTH && mode & GLM_TEXTURE) {
        fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                T(group->triangles[i]).vindices[0],
                T(group->triangles[i]).nindices[0],
                T(group->triangles[i]).tindices[0],
                T(group->triangles[i]).vindices[1],
                T(group->triangles[i]).nindices[1],
                T(group->triangles[i]).tindices[1],
                T(group->triangles[i]).vindices[2],
                T(group->triangles[i]).nindices[2],
                T(group->triangles[i]).tindices[2]);
      } else if (mode & GLM_FLAT && mode & GLM_TEXTURE) {
        fprintf(file, "f %d/%d %d/%d %d/%d\n",
                T(group->triangles[i]).vindices[0],
                T(group->triangles[i]).findex,
                T(group->triangles[i]).vindices[1],
                T(group->triangles[i]).findex,
                T(group->triangles[i]).vindices[2],
                T(group->triangles[i]).findex);
      } else if (mode & GLM_TEXTURE) {
        fprintf(file, "f %d/%d %d/%d %d/%d\n",
                T(group->triangles[i]).vindices[0],
                T(group->triangles[i]).tindices[0],
                T(group->triangles[i]).vindices[1],
                T(group->triangles[i]).tindices[1],
                T(group->triangles[i]).vindices[2],
                T(group->triangles[i]).tindices[2]);
      } else if (mode & GLM_SMOOTH) {
        fprintf(file, "f %d//%d %d//%d %d//%d\n",
                T(group->triangles[i]).vindices[0],
                T(group->triangles[i]).nindices[0],
                T(group->triangles[i]).vindices[1],
                T(group->triangles[i]).nindices[1],
                T(group->triangles[i]).vindices[2],
                T(group->triangles[i]).nindices[2]);
      } else if (mode & GLM_FLAT) {
        fprintf(file, "f %d//%d %d//%d %d//%d\n",
                T(group->triangles[i]).vindices[0],
                T(group->triangles[i]).findex,
                T(group->triangles[i]).vindices[1],
                T(group->triangles[i]).findex,
                T(group->triangles[i]).vindices[2],
                T(group->triangles[i]).findex);
      } else {
        fprintf(file, "f %d %d %d\n",
                T(group->triangles[i]).vindices[0],
                T(group->triangles[i]).vindices[1],
                T(group->triangles[i]).vindices[2]);
      }
    }
    fprintf(file, "\n");
    group = group->next;
  }

  fclose(file);
  return 0;//?
}

/* glmDraw: Renders the model to the current OpenGL context using the
 * mode specified.
 *
 * model - initialized GLMmodel structure
 * mode  - a bitwise OR of values describing what is to be rendered.
 *             GLM_NONE     -  render with only vertices
 *             GLM_FLAT     -  render with facet normals
 *             GLM_SMOOTH   -  render with vertex normals
 *             GLM_TEXTURE  -  render with texture coords
 *             GLM_COLOR    -  render with colors (color material)
 *             GLM_MATERIAL -  render with materials
 *             GLM_COLOR and GLM_MATERIAL should not both be specified.
 *             GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid
glmDraw(GLMmodel* model, GLuint mode)
{
  static GLuint i;
  static GLMgroup* group;
  static GLMtriangle* triangle;
  static GLMmaterial* material;

  if (!(model))return;
  if (!(model->vertices))return;

  /* do a bit of warning */
  if (mode & GLM_FLAT && !model->facetnorms) {
    logpost(NULL, 5,"glmDraw() warning: flat render mode requested "
            "with no facet normals defined.");
    mode &= ~GLM_FLAT;
  }
  if (mode & GLM_SMOOTH && !model->normals) {
    logpost(NULL, 5,"glmDraw() warning: smooth render mode requested "
            "with no normals defined.");
    mode &= ~GLM_SMOOTH;
  }
  if (mode & GLM_TEXTURE && !model->texcoords) {
    logpost(NULL, 5,"glmDraw() warning: texture render mode requested "
            "with no texture coordinates defined.");
    mode &= ~GLM_TEXTURE;
  }
  if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
    logpost(NULL, 5,"glmDraw() warning: flat render mode requested "
            "and smooth render mode requested (using smooth).");
    mode &= ~GLM_FLAT;
  }
  if (mode & GLM_COLOR && !model->materials) {
    logpost(NULL, 5,"glmDraw() warning: color render mode requested "
            "with no materials defined.");
    mode &= ~GLM_COLOR;
  }
  if (mode & GLM_MATERIAL && !model->materials) {
    logpost(NULL, 5,"glmDraw() warning: material render mode requested "
            "with no materials defined.");
    mode &= ~GLM_MATERIAL;
  }
  if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
    logpost(NULL, 5,"glmDraw() warning: color and material render mode requested "
            "using only material mode.");
    mode &= ~GLM_COLOR;
  }
  if (mode & GLM_COLOR)
    glEnable(GL_COLOR_MATERIAL);
  else if (mode & GLM_MATERIAL)
    glDisable(GL_COLOR_MATERIAL);

  /* perhaps this loop should be unrolled into material, color, flat,
     smooth, etc. loops?  since most cpu's have good branch prediction
     schemes (and these branches will always go one way), probably
     wouldn't gain too much?  */

  group = model->groups;
  while (group) {
    if (mode & GLM_MATERIAL) {
      material = &model->materials[group->material];
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->ambient);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->diffuse);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->specular);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shininess);
    }

    if (mode & GLM_COLOR) {
      glColor3fv(material->diffuse);
    }

    glBegin(GL_TRIANGLES);
    for (i = 0; i < group->numtriangles; i++) {
      triangle = &T(group->triangles[i]);

      if (mode & GLM_FLAT)
        glNormal3fv(&model->facetnorms[3 * triangle->findex]);

      if (mode & GLM_SMOOTH)
        glNormal3fv(&model->normals[3 * triangle->nindices[0]]);
      if (mode & GLM_TEXTURE)
        glTexCoord2fv(&model->texcoords[2 * triangle->tindices[0]]);
      glVertex3fv(&model->vertices[3 * triangle->vindices[0]]);

      if (mode & GLM_SMOOTH)
        glNormal3fv(&model->normals[3 * triangle->nindices[1]]);
      if (mode & GLM_TEXTURE)
        glTexCoord2fv(&model->texcoords[2 * triangle->tindices[1]]);
      glVertex3fv(&model->vertices[3 * triangle->vindices[1]]);

      if (mode & GLM_SMOOTH)
        glNormal3fv(&model->normals[3 * triangle->nindices[2]]);
      if (mode & GLM_TEXTURE)
        glTexCoord2fv(&model->texcoords[2 * triangle->tindices[2]]);
      glVertex3fv(&model->vertices[3 * triangle->vindices[2]]);

    }
    glEnd();

    group = group->next;
  }
}

/* glmList: Generates and returns a display list for the model using
 * the mode specified.
 *
 * model - initialized GLMmodel structure
 * mode  - a bitwise OR of values describing what is to be rendered.
 *             GLM_NONE     -  render with only vertices
 *             GLM_FLAT     -  render with facet normals
 *             GLM_SMOOTH   -  render with vertex normals
 *             GLM_TEXTURE  -  render with texture coords
 *             GLM_COLOR    -  render with colors (color material)
 *             GLM_MATERIAL -  render with materials
 *             GLM_COLOR and GLM_MATERIAL should not both be specified.
 * GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLuint
glmList(GLMmodel* model, GLuint mode)
{
  GLuint modList;

  modList = glGenLists(1);
  glNewList(modList, GL_COMPILE);
  glmDraw(model, mode);
  glEndList();

  return modList;
}
/***********
 * this draws only a single group instead of the entire model
 * added for Leif and Annette
 */

GLvoid
glmDrawGroup(GLMmodel* model, GLuint mode,int groupNumber)
{
  static GLuint i;
  static GLMgroup* group;
  static GLMtriangle* triangle;
  static GLMmaterial* material;

  if (!(model))return;
  if (!(model->vertices))return;

  /* do a bit of warning */
  if (mode & GLM_FLAT && !model->facetnorms) {
    logpost(NULL, 5,"glmDraw() warning: flat render mode requested "
            "with no facet normals defined.");
    mode &= ~GLM_FLAT;
  }
  if (mode & GLM_SMOOTH && !model->normals) {
    logpost(NULL, 5,"glmDraw() warning: smooth render mode requested "
            "with no normals defined.");
    mode &= ~GLM_SMOOTH;
  }
  if (mode & GLM_TEXTURE && !model->texcoords) {
    logpost(NULL, 5,"glmDraw() warning: texture render mode requested "
            "with no texture coordinates defined.");
    mode &= ~GLM_TEXTURE;
  }
  if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
    logpost(NULL, 5,"glmDraw() warning: flat render mode requested "
            "and smooth render mode requested (using smooth).");
    mode &= ~GLM_FLAT;
  }
  if (mode & GLM_COLOR && !model->materials) {
    logpost(NULL, 5,"glmDraw() warning: color render mode requested "
            "with no materials defined.");
    mode &= ~GLM_COLOR;
  }
  if (mode & GLM_MATERIAL && !model->materials) {
    logpost(NULL, 5,"glmDraw() warning: material render mode requested "
            "with no materials defined.");
    mode &= ~GLM_MATERIAL;
  }
  if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
    logpost(NULL, 5,"glmDraw() warning: color and material render mode requested "
            "using only material mode.");
    mode &= ~GLM_COLOR;
  }
  if (mode & GLM_COLOR)
    glEnable(GL_COLOR_MATERIAL);
  else if (mode & GLM_MATERIAL)
    glDisable(GL_COLOR_MATERIAL);

  /* perhaps this loop should be unrolled into material, color, flat,
     smooth, etc. loops?  since most cpu's have good branch prediction
     schemes (and these branches will always go one way), probably
     wouldn't gain too much?  */

  group = model->groups;

  int count = 1;
  int numgroup;

  numgroup = model->numgroups-1;

  logpost(NULL, 5, "number of groups: %d",numgroup);
  //groupNumber-=1;
  if ( (!(groupNumber > numgroup)) && (groupNumber > 0)){
    logpost(NULL, 5, "model group requested is %d number of groups: %d",groupNumber,numgroup);


    while (count < groupNumber) {
      group = group->next;
      count++;
    }


    if (mode & GLM_MATERIAL) {
      material = &model->materials[group->material];
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->ambient);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->diffuse);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->specular);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shininess);
    }

    if (mode & GLM_COLOR) {
      glColor3fv(material->diffuse);
    }

    glBegin(GL_TRIANGLES);
    for (i = 0; i < group->numtriangles; i++) {
      triangle = &T(group->triangles[i]);

      if (mode & GLM_FLAT)
        glNormal3fv(&model->facetnorms[3 * triangle->findex]);

      if (mode & GLM_SMOOTH)
        glNormal3fv(&model->normals[3 * triangle->nindices[0]]);
      if (mode & GLM_TEXTURE)
        glTexCoord2fv(&model->texcoords[2 * triangle->tindices[0]]);
      glVertex3fv(&model->vertices[3 * triangle->vindices[0]]);

      if (mode & GLM_SMOOTH)
        glNormal3fv(&model->normals[3 * triangle->nindices[1]]);
      if (mode & GLM_TEXTURE)
        glTexCoord2fv(&model->texcoords[2 * triangle->tindices[1]]);
      glVertex3fv(&model->vertices[3 * triangle->vindices[1]]);

      if (mode & GLM_SMOOTH)
        glNormal3fv(&model->normals[3 * triangle->nindices[2]]);
      if (mode & GLM_TEXTURE)
        glTexCoord2fv(&model->texcoords[2 * triangle->tindices[2]]);
      glVertex3fv(&model->vertices[3 * triangle->vindices[2]]);

    }
    glEnd();
  }
  // group = group->next;
  //  }
}

GLuint
glmListGroup(GLMmodel* model, GLuint mode, int groupNumber)
{
  GLuint modList;

  modList = glGenLists(1);
  glNewList(modList, GL_COMPILE);
  glmDrawGroup(model, mode,groupNumber);
  glEndList();

  return modList;
}

/* glmWeld: eliminate (weld) vectors that are within an epsilon of
 * each other.
 *
 * model   - initialized GLMmodel structure
 * epsilon     - maximum difference between vertices
 *               ( 0.00001 is a good start for a unitized model)
 *
 */
GLvoid
glmWeld(GLMmodel* model, GLfloat epsilon)
{
  GLfloat* vectors;
  GLfloat* copies;
  GLuint   numvectors;
  GLuint   i;

  /* vertices */
  numvectors = model->numvertices;
  vectors  = model->vertices;
  copies = _glmWeldVectors(vectors, &numvectors, epsilon);

  logpost(NULL, 5, "glmWeld(): %d redundant vertices.", model->numvertices - numvectors - 1);

  for (i = 0; i < model->numtriangles; i++) {
    T(i).vindices[0] = static_cast<GLuint>(vectors[3 * T(i).vindices[0] + 0]);
    T(i).vindices[1] = static_cast<GLuint>(vectors[3 * T(i).vindices[1] + 0]);
    T(i).vindices[2] = static_cast<GLuint>(vectors[3 * T(i).vindices[2] + 0]);
  }

  /* free space for old vertices */
  free(vectors);

  /* allocate space for the new vertices */
  model->numvertices = numvectors;
  model->vertices = (GLfloat*)malloc(sizeof(GLfloat) *
                                     3 * (model->numvertices + 1));

  /* copy the optimized vertices into the actual vertex list */
  for (i = 1; i <= model->numvertices; i++) {
    model->vertices[3 * i + 0] = copies[3 * i + 0];
    model->vertices[3 * i + 1] = copies[3 * i + 1];
    model->vertices[3 * i + 2] = copies[3 * i + 2];
  }

  free(copies);
}

/* glmReadPPM: read a PPM raw (type P6) file.  The PPM file has a header
 * that should look something like:
 *
 *    P6
 *    # comment
 *    width height max_value
 *    rgbrgbrgb...
 *
 * where "P6" is the magic cookie which identifies the file type and
 * should be the only characters on the first line followed by a
 * carriage return.  Any line starting with a # mark will be treated
 * as a comment and discarded.   After the magic cookie, three integer
 * values are expected: width, height of the image and the maximum
 * value for a pixel (max_value must be < 256 for PPM raw files).  The
 * data section consists of width*height rgb triplets (one byte each)
 * in binary format (i.e., such as that written with fwrite() or
 * equivalent).
 *
 * The rgb data is returned as an array of unsigned chars (packed
 * rgb).  The malloc()'d memory should be free()'d by the caller.  If
 * an error occurs, an error message is sent to stdout and NULL is
 * returned.
 *
 * filename   - name of the .ppm file.
 * width      - will contain the width of the image on return.
 * height     - will contain the height of the image on return.
 *
 */
GLubyte*
glmReadPPM(char* filename, int* width, int* height)
{
  FILE* fp;
  int i, w, h, d;
  unsigned char* image;
  char head[70];          /* max line <= 70 in PPM (per spec). */

  fp = fopen(filename, "rb");
  if (!fp) {
    perror(filename);
    return NULL;
  }

  /* grab first two chars of the file and make sure that it has the
     correct magic cookie for a raw PPM file. */
  if(NULL==fgets(head, 70, fp)) {
    error("_glmReadPPM() failed reading header"); return NULL;
  }
  if (strncmp(head, "P6", 2)) {
    error("%s: Not a raw PPM file", filename);
    return NULL;
  }

  /* grab the three elements in the header (width, height, maxval). */
  i = 0;
  while(i < 3) {
    if(NULL==fgets(head, 70, fp)) {
      error("_glmReadPPM() failed header info"); return NULL;
    }
    if (head[0] == '#')     /* skip comments. */
      continue;
    if (i == 0)
      i += sscanf(head, "%d %d %d", &w, &h, &d);
    else if (i == 1)
      i += sscanf(head, "%d %d", &h, &d);
    else if (i == 2)
      i += sscanf(head, "%d", &d);
  }

  /* grab all the image data in one fell swoop. */
  image = (unsigned char*)malloc(sizeof(unsigned char)*w*h*3);
  size_t count = fread(image, sizeof(unsigned char), w*h*3, fp);
  fclose(fp);
  if(count!=static_cast<size_t>(w*h*3)) {
    error("_glmReadPPM failed to read all bytes");
    *width=*height=0;
    free(image);
    image=NULL;
    return NULL;
  }

  *width = w;
  *height = h;
  return image;
}
