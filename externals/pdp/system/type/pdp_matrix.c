
#include <string.h>
#include "pdp_matrix.h"
#include "pdp_packet.h"
#include "pdp_symbol.h"
#include "pdp_internals.h" // for pdp_packet_new, which is not a proper constructor
#include "pdp_post.h"
#include "pdp_type.h"

/* the class object */
static t_pdp_class *matrix_class;



/* declare header and subheader variables and exit with errval if invalid */
#define VALID_MATRIX_HEADER(packet, header, subheader, mtype, errval)   \
t_pdp *    header    = pdp_packet_header( packet );			\
t_matrix * subheader = (t_matrix *)pdp_packet_subheader( packet );	\
if (! header ) return errval;						\
if (PDP_MATRIX != header->type) return errval;				\
if (mtype) {if (subheader->type != mtype) return errval;}


int pdp_packet_matrix_isvalid(int p)
{
    VALID_MATRIX_HEADER(p, h, m, 0, 0);
    return 1;
}


void *pdp_packet_matrix_get_gsl_matrix(int p, u32 type)
{
    VALID_MATRIX_HEADER(p, h, m, type, 0);
    return &m->matrix;
}

void *pdp_packet_matrix_get_gsl_vector(int p, u32 type)
{
    VALID_MATRIX_HEADER(p, h, m, type, 0);
    return &m->vector;
}

int pdp_packet_matrix_get_type(int p)
{
    VALID_MATRIX_HEADER(p, h, m, 0, 0);
    return m->type;
}

int pdp_packet_matrix_isvector(int p)
{
    VALID_MATRIX_HEADER(p, h, m, 0, 0);
    return ((m->rows == 1) || (m->columns == 1));
}

int pdp_packet_matrix_ismatrix(int p)
{
    VALID_MATRIX_HEADER(p, h, m, 0, 0);
    return ((m->rows != 1) && (m->columns != 1));
}




/* gsl blas matrix/vector multiplication:

vector.vector

 (const gsl_vector * x, const gsl_vector * y, double * result) 

gsl_blas_sdot
gsl_blas_ddot
gsl_blas_cdot
gsl_blas_zdot
gsl_blas_cdotu
gsl_blas_zdotu

matrix.vector
 (
   CBLAS_TRANSPOSE_t 
   TransA, 
   double alpha, 
   const gsl_matrix * A, 
   const gsl_vector * x, 
   double beta, 
   gsl_vector * y
 )

gsl_blas_sgemv
gsl_blas_dgemv
gsl_blas_cgemv
gsl_blas_zgemv

matrix.matrix
 (
   CBLAS_TRANSPOSE_t TransA, 
   CBLAS_TRANSPOSE_t TransB, 
   double alpha, 
   const gsl_matrix * A, 
   const gsl_matrix * B, 
   double beta, 
   gsl_matrix * C
 )

gsl_blas_sgemm
gsl_blas_dgemm
gsl_blas_cgemm
gsl_blas_zgemm

*/

/* compute the matrix inverse using the LU decomposition */
/* it only works for double real/complex */
int pdp_packet_matrix_LU_to_inverse(int p)
{
    int new_p;
    u32 n;
    int type = pdp_packet_matrix_get_type(p);
    t_matrix *sheader = (t_matrix *)pdp_packet_subheader(p);
    gsl_matrix *m = (gsl_matrix *)pdp_packet_matrix_get_gsl_matrix(p, type);
    gsl_matrix *im;
    if (!m) return -1;
    n = m->gsl_rows; 
    if (n != m->gsl_columns) return -1;
    new_p = pdp_packet_new_matrix(n, n, type);
    if (-1 == new_p) return -1;
    im = (gsl_matrix *)pdp_packet_matrix_get_gsl_matrix(new_p, type);

    switch(type){
    case PDP_MATRIX_TYPE_RDOUBLE:
	gsl_linalg_LU_invert (m, &sheader->perm, im); break;
    case PDP_MATRIX_TYPE_CDOUBLE:
	gsl_linalg_complex_LU_invert ((gsl_matrix_complex *)m, &sheader->perm, (gsl_matrix_complex *)im); break;
    default:
	pdp_packet_mark_unused(new_p);
	new_p = -1;
    }
    return new_p;
}
/* compute the LU decomposition of a square matrix */
/* it only works for double real/complex */
int pdp_packet_matrix_LU(int p)
{
    int p_LU, bytes;
    u32 n;
    int type = pdp_packet_matrix_get_type(p);
    t_matrix *sh_m_LU;
    t_matrix *sh_m = (t_matrix *)pdp_packet_subheader(p);
    gsl_matrix *m = (gsl_matrix *)pdp_packet_matrix_get_gsl_matrix(p, type);
    gsl_matrix *m_LU;
    if (!m) return -1;
    n = m->gsl_rows; 
    if (n != m->gsl_columns) return -1;
    p_LU = pdp_packet_new_matrix(n, n, type);
    if (-1 == p_LU) return -1;
    sh_m_LU  = (t_matrix *)pdp_packet_subheader(p_LU);
    m_LU = (gsl_matrix *)pdp_packet_matrix_get_gsl_matrix(p_LU, type);
    /* copy matrix data: move this to copy method */
    memcpy(pdp_packet_data(p_LU), pdp_packet_data(p), sh_m->block.size); 

    switch(type){
    case PDP_MATRIX_TYPE_RDOUBLE:
	gsl_linalg_LU_decomp (m_LU, &sh_m_LU->perm, &sh_m_LU->signum); break; 
    case PDP_MATRIX_TYPE_CDOUBLE:
	gsl_linalg_complex_LU_decomp ((gsl_matrix_complex *)m_LU, &sh_m_LU->perm, &sh_m_LU->signum); break; 
    default:
	pdp_packet_mark_unused(p_LU);
	p_LU = -1;
    }
    return p_LU;
}

int pdp_packet_matrix_LU_solve(int p_matrix, int p_vector)
{
    int type = pdp_packet_matrix_get_type(p_matrix);
    int p_result_vector = pdp_packet_clone_rw(p_vector);
    gsl_matrix *m = (gsl_matrix *)pdp_packet_matrix_get_gsl_matrix(p_matrix, type);
    gsl_vector *v = (gsl_vector *)pdp_packet_matrix_get_gsl_vector(p_vector, type);
    gsl_vector *rv = (gsl_vector *)pdp_packet_matrix_get_gsl_vector(p_result_vector, type);
    t_matrix *sh_m = (t_matrix *)pdp_packet_subheader(p_matrix);

    if (!(m && v && rv)) goto error;

    switch(type){
    case PDP_MATRIX_TYPE_RDOUBLE:
	if (gsl_linalg_LU_solve (m, &sh_m->perm, v, rv)) goto error; break;
    case PDP_MATRIX_TYPE_CDOUBLE:
	if(gsl_linalg_complex_LU_solve ((gsl_matrix_complex*)m, &sh_m->perm, 
		(gsl_vector_complex *)v, (gsl_vector_complex *)rv)) goto error; break;
    default:
	goto error;
    }
    return p_result_vector;

 error:
    pdp_packet_mark_unused(p_result_vector);
    //post("error");
    return -1;
}

/* matrix matrix mul: C is defining type 
   returns 0 on success */
int pdp_packet_matrix_blas_mm
(
 CBLAS_TRANSPOSE_t TransA,
 CBLAS_TRANSPOSE_t TransB,
 int pA,
 int pB,
 int pC,
 float scale_r,
 float scale_i
)
{
    gsl_complex_float cf_scale = {{scale_r, scale_i}};
    gsl_complex cd_scale = {{(double)scale_r, (double)scale_i}};
    gsl_complex_float cf_one = {{1.0f, 0.0f}};
    gsl_complex cd_one = {{1.0, 0.0}};
    gsl_matrix *mA, *mB, *mC;
    int type;
    type = pdp_packet_matrix_get_type(pC);
    mA = (gsl_matrix *)pdp_packet_matrix_get_gsl_matrix(pA,type);
    mB = (gsl_matrix *)pdp_packet_matrix_get_gsl_matrix(pB,type);
    mC = (gsl_matrix *)pdp_packet_matrix_get_gsl_matrix(pC,type);

    if (!(mA && mB)) return 1;


    switch(type){
    case PDP_MATRIX_TYPE_RFLOAT:
	return gsl_blas_sgemm(TransA, TransB, scale_r, (gsl_matrix_float *)mA, 
			      (gsl_matrix_float *)mB, 1.0f, (gsl_matrix_float *)mC); 
    case PDP_MATRIX_TYPE_RDOUBLE:
	return gsl_blas_dgemm(TransA, TransB, (double)scale_r, (gsl_matrix *)mA, 
			      (gsl_matrix *)mB, 1.0, (gsl_matrix *)mC);
    case PDP_MATRIX_TYPE_CFLOAT:
	return gsl_blas_cgemm(TransA, TransB, cf_scale, (gsl_matrix_complex_float *)mA, 
			      (gsl_matrix_complex_float *)mB, cf_one, (gsl_matrix_complex_float *)mC); 
    case PDP_MATRIX_TYPE_CDOUBLE:
	return gsl_blas_zgemm(TransA, TransB, cd_scale, (gsl_matrix_complex *)mA, 
			      (gsl_matrix_complex *)mB, cd_one, (gsl_matrix_complex *)mC); 
    default:
	return 0;
    }
}

/* matrix vector mul: C is defining type 
   returns 0 on success */
int pdp_packet_matrix_blas_mv
(
 CBLAS_TRANSPOSE_t TransA,
 int pA,
 int pb,
 int pc,
 float scale_r,
 float scale_i
)
{
    gsl_complex_float cf_scale = {{scale_r, scale_i}};
    gsl_complex cd_scale = {{(double)scale_r, (double)scale_i}};
    gsl_complex_float cf_one = {{1.0f, 0.0f}};
    gsl_complex cd_one = {{1.0, 0.0}};
    gsl_matrix *mA;
    gsl_vector *vb, *vc;
    int type;
    type = pdp_packet_matrix_get_type(pA);
    mA = (gsl_matrix *)pdp_packet_matrix_get_gsl_matrix(pA,type);
    vb = (gsl_vector *)pdp_packet_matrix_get_gsl_vector(pb,type);
    vc = (gsl_vector *)pdp_packet_matrix_get_gsl_vector(pc,type);

    if (!(vb && vc)) return 1;


    switch(type){
    case PDP_MATRIX_TYPE_RFLOAT:
	return gsl_blas_sgemv(TransA, scale_r, (gsl_matrix_float *)mA, 
			      (gsl_vector_float *)vb, 1.0f, (gsl_vector_float *)vc); 
    case PDP_MATRIX_TYPE_RDOUBLE:
	return gsl_blas_dgemv(TransA, (double)scale_r, (gsl_matrix *)mA, 
			      (gsl_vector *)vb, 1.0, (gsl_vector *)vc);
    case PDP_MATRIX_TYPE_CFLOAT:
	return gsl_blas_cgemv(TransA, cf_scale, (gsl_matrix_complex_float *)mA, 
			      (gsl_vector_complex_float *)vb, cf_one, (gsl_vector_complex_float *)vc); 
    case PDP_MATRIX_TYPE_CDOUBLE:
	return gsl_blas_zgemv(TransA, cd_scale, (gsl_matrix_complex *)mA, 
			      (gsl_vector_complex *)vb, cd_one, (gsl_vector_complex *)vc); 
    default:
	return 0;
    }
}



t_pdp_symbol *_pdp_matrix_get_description(t_pdp *header)
{
    t_matrix *m = (t_matrix *)(&header->info.raw);
    char description[100];
    char *c = description;
    int encoding;

    if (!header) return pdp_gensym("invalid");
    else if (!header->desc){
	/* if description is not defined, try to reconstruct it (for backwards compat) */
	if (header->type == PDP_MATRIX){

	    c += sprintf(c, "matrix");

	    switch(m->type){
	    case PDP_MATRIX_TYPE_RFLOAT: c += sprintf(c, "/float/real"); break;
	    case PDP_MATRIX_TYPE_CFLOAT: c += sprintf(c, "/float/complex"); break;
	    case PDP_MATRIX_TYPE_RDOUBLE: c += sprintf(c, "/double/real"); break;
	    case PDP_MATRIX_TYPE_CDOUBLE: c += sprintf(c, "/double/complex"); break;
	    default:
		c += sprintf(c, "/unknown"); goto exit;
	    }

	    c += sprintf(c, "/%dx%d", (int)m->rows, (int)m->columns);
	    
	exit:
	    return pdp_gensym(description);
	} 
	else return pdp_gensym("unknown");
    }
    else return header->desc;
}



static void _pdp_matrix_copy(t_pdp *dst, t_pdp *src);
static void _pdp_matrix_clone(t_pdp *dst, t_pdp *src);
static void _pdp_matrix_reinit(t_pdp *dst);
static void _pdp_matrix_cleanup(t_pdp *dst);

static size_t _pdp_matrix_mdata_byte_size(u32 rows, u32 columns, u32 type)
{
    size_t dsize = rows * columns;
    switch (type){
    case PDP_MATRIX_TYPE_RFLOAT:   dsize *= sizeof(float);    break;
    case PDP_MATRIX_TYPE_CFLOAT:   dsize *= 2*sizeof(float);  break;
    case PDP_MATRIX_TYPE_RDOUBLE:  dsize *= sizeof(double);   break;
    case PDP_MATRIX_TYPE_CDOUBLE:  dsize *= 2*sizeof(double); break;
    default: dsize = 0;
    }
    return dsize;
}


static size_t _pdp_matrix_pdata_vector_size(u32 rows, u32 columns)
{
    return (rows>columns ? rows : columns);
}


static void _pdp_matrix_init(t_pdp *dst, u32 rows, u32 columns, u32 type)
{
    int i;
    t_matrix *m = (t_matrix *)(&dst->info.raw);
    void *d = ((void *)dst) + PDP_HEADER_SIZE;
    int matrix_bytesize = _pdp_matrix_mdata_byte_size(rows, columns, type);
    int permsize =  _pdp_matrix_pdata_vector_size(rows, columns);

    /* set meta data */
    m->type = type;
    m->rows = rows;
    m->columns = columns;

    /* set the block data */
    m->block.size = matrix_bytesize;
    m->block.data = (double *)d;

    /* set the vector view */
    m->vector.size = (1==columns) ? rows : columns;
    m->vector.stride = 1;
    m->vector.data = (double *)d;
    m->vector.block = &m->block;
    m->vector.owner = 0;

    /* set the matrix view */
    m->matrix.gsl_rows = rows;
    m->matrix.gsl_columns = columns;
    m->matrix.tda = columns;
    m->matrix.data = (double *)d;
    m->matrix.block = &m->block;
    m->matrix.owner = 0;

    /* set the permutation object & init */
    m->perm.size = permsize;
    m->perm.data = (size_t *)(d + matrix_bytesize);
    for(i=0; i<permsize; i++) m->perm.data[i] = i;
    m->signum = -1;
    
    /* init packet header */
    dst->theclass = matrix_class;
    dst->desc = _pdp_matrix_get_description(dst);

}

static void _pdp_matrix_clone(t_pdp *dst, t_pdp *src)
{
    t_matrix *m = (t_matrix *)(&src->info.raw);
    _pdp_matrix_init(dst, m->rows, m->columns, m->type);

}
static void _pdp_matrix_copy(t_pdp *dst, t_pdp *src)
{
    _pdp_matrix_clone(dst, src);
    memcpy(dst + PDP_HEADER_SIZE, src + PDP_HEADER_SIZE, src->size - PDP_HEADER_SIZE);
    //post("matrix copy successful");
}
static void _pdp_matrix_reinit(t_pdp *dst)
{
    /* nothing to do, assuming data is correct */
}
static void _pdp_matrix_cleanup(t_pdp *dst)
{
    /* no extra memory to free */
}

int pdp_packet_new_matrix(u32 rows, u32 columns, u32 type)
{
    t_pdp *header;
    int dsize, p;

    /* compute the blocksize for the matrix data */
    /* if 0, something went wrong -> return invalid packet */
    if (!(dsize = _pdp_matrix_mdata_byte_size(rows, columns, type))) return -1;
    dsize += sizeof(size_t) * _pdp_matrix_pdata_vector_size(rows, columns);

    p = pdp_packet_new(PDP_MATRIX, dsize);
    if (-1 == p) return -1;
    header = pdp_packet_header(p);
    
    _pdp_matrix_init(header, rows, columns, type);

    return p;
}

int pdp_packet_new_matrix_product_result(CBLAS_TRANSPOSE_t TransA, CBLAS_TRANSPOSE_t TransB, int pA, int pB)
{
    gsl_matrix *mA, *mB;
    u32 type = pdp_packet_matrix_get_type(pA);

    u32 colA, colB, rowA, rowB;
   
    /* check if A is a matrix */
    if (!type) return -1;

    mA = (gsl_matrix *)pdp_packet_matrix_get_gsl_matrix(pA, type);
    mB = (gsl_matrix *)pdp_packet_matrix_get_gsl_matrix(pB, type);

    /* check if A and B are same type */
    if (!mB) return -1;

    /* get dims A */
    if (TransA == CblasNoTrans){
	rowA = mA->gsl_rows;
	colA = mA->gsl_columns;
    }
    else {
	rowA = mA->gsl_columns;
	colA = mA->gsl_rows;
    }

    /* get dims B */
    if (TransB == CblasNoTrans){
	rowB = mB->gsl_rows;
	colB = mB->gsl_columns;
    }
    else {
	rowB = mB->gsl_columns;
	colB = mB->gsl_rows;
    }

    /* check if sizes are compatible */
    if (colA != rowB) return -1;

    /* create new packet */
    return pdp_packet_new_matrix(rowA, colB, type);
}

void pdp_packet_matrix_setzero(int p)
{
    t_matrix *m;
    if (!pdp_packet_matrix_isvalid(p)) return;
    m = (t_matrix *) pdp_packet_subheader(p);
    memset(m->block.data, 0, m->block.size);
}



/* type conversion programs */
static int _pdp_packet_convert_matrix_to_greyimage(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_matrix *matrix = (t_matrix *)pdp_packet_subheader(packet);
    void *data = pdp_packet_data(packet);
    s16 *new_data;
    u32 c,r, nbelements;
    int new_p;
    u32 i;

    c = matrix->columns;
    r = matrix->rows;
    nbelements = c*r;

    new_p = pdp_packet_new_image_grey(c,r);
    if (-1 == new_p) return -1;
    new_data = (s16 *)pdp_packet_data(new_p);

    /* convert first channel */
    switch(matrix->type){
    case PDP_MATRIX_TYPE_RFLOAT:
	for (i=0; i<nbelements; i++) (new_data)[i] = (s16)(((float *)data)[i] * (float)0x8000); break;
    case PDP_MATRIX_TYPE_CFLOAT: //only copy real channel
	for (i=0; i<nbelements; i++) (new_data)[i] = (s16)(((float *)data)[i<<1] * (float)0x8000); break;
    case PDP_MATRIX_TYPE_RDOUBLE:
	for (i=0; i<nbelements; i++) (new_data)[i] = (s16)(((double *)data)[i] * (float)0x8000); break;
    case PDP_MATRIX_TYPE_CDOUBLE: //only copy real channel
	for (i=0; i<nbelements; i++) (new_data)[i] = (s16)(((double *)data)[i<<1] * (float)0x8000); break;
    default:
	pdp_packet_mark_unused(new_p);
	new_p = -1;
    }	
    return new_p;
}

static int _pdp_packet_convert_image_to_matrix(int packet, t_pdp_symbol *dest_template, int type)
{
    t_pdp *header = pdp_packet_header(packet);
    t_image *image = pdp_packet_image_info(packet);
    s16 *data = (s16 *)pdp_packet_data(packet);
    void *new_data;
    u32 w,h, nbelements;
    int new_p;
    int encoding = image->encoding;
    u32 i;

    if (!pdp_packet_image_isvalid(packet)) return -1;
    w = image->width;
    h = image->height;
    nbelements = w*h;

    new_p = pdp_packet_new_matrix(h,w, type);
    if (-1 == new_p) return -1;
    new_data = pdp_packet_data(new_p);

    switch (encoding){
    case PDP_IMAGE_YV12:
    case PDP_IMAGE_GREY:
    case PDP_IMAGE_MCHP:
	/* convert first channel */
	switch(type){
	case PDP_MATRIX_TYPE_RFLOAT:
	    for (i=0; i<nbelements; i++)
		((float *)new_data)[i] = ((float)data[i]) * (1.0f / (float)0x8000); break;
	case PDP_MATRIX_TYPE_RDOUBLE:
	    for (i=0; i<nbelements; i++)
		((double *)new_data)[i] = ((double)data[i]) * (1.0f / (double)0x8000); break;
	case PDP_MATRIX_TYPE_CFLOAT:
	    for (i=0; i<nbelements; i++){
		((float *)new_data)[i*2] = ((float)data[i]) * (1.0f / (float)0x8000); 
		((float *)new_data)[i*2+1] = 0.0f;
	    }
	    break;
	case PDP_MATRIX_TYPE_CDOUBLE:
	    for (i=0; i<nbelements; i++){
		((double *)new_data)[i*2] = ((double)data[i]) * (1.0f / (double)0x8000); 
		((double *)new_data)[i*2+1] = 0.0;
	    }
	    break;
	default:
	    pdp_post("_pdp_packet_convert_image_to_matrix: INTERNAL ERROR");
	}
	break;
    default:
	pdp_packet_mark_unused(new_p);
	new_p = -1;
	break;
    }

    return new_p;

}

static int _pdp_packet_convert_image_to_floatmatrix(int packet, t_pdp_symbol *dest_template){
    return _pdp_packet_convert_image_to_matrix(packet, dest_template, PDP_MATRIX_TYPE_RFLOAT);}
static int _pdp_packet_convert_image_to_doublematrix(int packet, t_pdp_symbol *dest_template){
    return _pdp_packet_convert_image_to_matrix(packet, dest_template, PDP_MATRIX_TYPE_RDOUBLE);}
static int _pdp_packet_convert_image_to_complexfloatmatrix(int packet, t_pdp_symbol *dest_template){
    return _pdp_packet_convert_image_to_matrix(packet, dest_template, PDP_MATRIX_TYPE_CFLOAT);}
static int _pdp_packet_convert_image_to_complexdoublematrix(int packet, t_pdp_symbol *dest_template){
    return _pdp_packet_convert_image_to_matrix(packet, dest_template, PDP_MATRIX_TYPE_CDOUBLE);}


static int _pdp_packet_matrix_convert_fallback(int packet, t_pdp_symbol *dest_template)
{
    pdp_post("can't convert image type %s to %s",
	 pdp_packet_get_description(packet)->s_name, dest_template->s_name);

    return -1;
}


/* this seems like a nice spot to place a dummy gsl signal handler */
static void _gsl_error_handler (const char * reason, 
              const char * file, 
              int line, 
              int gsl_errno)
{
    //pdp_post("gsl error:\nREASON: %s\nFILE:%s\nLINE:%d\nERRNO:%d", reason, file, line, gsl_errno);
}

static int pdp_matrix_factory(t_pdp_symbol *type)
{
    return -1;
}
void pdp_matrix_setup(void)
{
    t_pdp_conversion_program *program;


    /* setup the class */
    matrix_class = pdp_class_new(pdp_gensym("matrix/*/*/*"), pdp_matrix_factory);
    matrix_class->copy = _pdp_matrix_copy;
    //matrix_class->clone = _pdp_matrix_clone; // is now solved through (symbol based) constructor
    matrix_class->wakeup = _pdp_matrix_reinit;
    matrix_class->cleanup = _pdp_matrix_cleanup;
    

    /* image -> matrix: default = double/real (most ops available) */
    program = pdp_conversion_program_new(_pdp_packet_convert_image_to_doublematrix, 0);
    pdp_type_register_conversion(pdp_gensym("image/*/*"), pdp_gensym("matrix/double/real/*"), program);
    program = pdp_conversion_program_new(_pdp_packet_convert_image_to_floatmatrix, 0);
    pdp_type_register_conversion(pdp_gensym("image/*/*"), pdp_gensym("matrix/float/real/*"), program);
    program = pdp_conversion_program_new(_pdp_packet_convert_image_to_complexdoublematrix, 0);
    pdp_type_register_conversion(pdp_gensym("image/*/*"), pdp_gensym("matrix/double/complex/*"), program);
    program = pdp_conversion_program_new(_pdp_packet_convert_image_to_complexfloatmatrix, 0);
    pdp_type_register_conversion(pdp_gensym("image/*/*"), pdp_gensym("matrix/float/complex/*"), program);

    /* matrix -> image */
    program = pdp_conversion_program_new(_pdp_packet_convert_matrix_to_greyimage, 0);
    pdp_type_register_conversion(pdp_gensym("matrix/*/*/*"), pdp_gensym("image/grey/*"), program);

    /* fallbacks */
    program = pdp_conversion_program_new(_pdp_packet_matrix_convert_fallback, 0);
    pdp_type_register_conversion(pdp_gensym("matrix/*/*/*"), pdp_gensym("image/*/*"), program);
    pdp_type_register_conversion(pdp_gensym("matrix/*/*/*"), pdp_gensym("bitmap/*/*"), program);
    pdp_type_register_conversion(pdp_gensym("image/*/*"), pdp_gensym("matrix/*/*/*/*"), program);
    pdp_type_register_conversion(pdp_gensym("bitmap/*/*"), pdp_gensym("matrix/*/*/*/*"), program);

    /* setup gsl handler */
    if(gsl_set_error_handler(_gsl_error_handler)){
	pdp_post("pdp_matrix_setup: WARNING: overriding gsl error handler.");
    }    

}
