#include "pdp_mmx.h"

#define FP(x) ((short int)(((float)(x) * 2 * 256.0f)))

#define nbp 256

    short int a1[4] = {0x0100,0x0100,0x0100,0x0100};
    short int a2[4] = {0x0100,0x0100,0x0100,0x0100};
    short int b0[4] = {0x0100,0x0100,0x0100,0x0100};
    short int b1[4] = {0x0100,0x0100,0x0100,0x0100};
    short int b2[4] = {0x0100,0x0100,0x0100,0x0100};

    short int u1[4] = {0x0100,0x0100,0x0100,0x0100};
    short int u2[4] = {0x0100,0x0100,0x0100,0x0100};

    short int x0[4] = {0x0100,0x0100,0x0100,0x0100};
    short int x1[4] = {0x0100,0x0100,0x0100,0x0100};
    short int x2[4] = {0x0100,0x0100,0x0100,0x0100};
    short int x3[4] = {0x0100,0x0100,0x0100,0x0100};

void print_pixel(unsigned int i)
{
    if (i) printf("x ");
    else printf(". ");
}

void print_line(void)
{
    printf("\n");
}

void print_square(unsigned char *c)
{
    int i,j;

    for(j=7; j>=0; j--){
	for(i=0; i<8; i++) print_pixel(c[j] & (1<<(7-i)));
	printf("\n");
    }
    
}

main()
{
  
    unsigned char src[16]={1,2,3,4,5,6,7,8,-1,-2,-3,-4,-5,-6,-7,-8};
    unsigned char dst[8];

    
    print_square(src);
    print_line();
    print_square(src+8);
    print_line();

    pixel_test_s1(dst,src,1,1);

    print_square(dst);
    print_line();



}
