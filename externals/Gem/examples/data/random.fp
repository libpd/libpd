!!ARBfp1.0
# Based on an algorithm described by Francois Grieu, sci.crypt, 5th February 2004
#ATTRIB tex0 = fragment.texcoord[0];
ATTRIB tex0 = fragment.position;
ATTRIB col = fragment.color;

#PARAM bounds = program.local[1];
#PARAM seed = program.local[2];
PARAM bounds = 10.0;
PARAM seed = 1234;
PARAM coordsOffset = { -100, 100, 0, 0 };
PARAM cMult = 0.0001002707309736288;
PARAM aSubtract = 0.2727272727272727;
PARAM coordMult0 = { 0.67676, 0.000058758, 0, 0 };
PARAM coordMult1 = { 0.0000696596, 0.797976, 0, 0 };
PARAM coordMult2 = { 0.587976, 0.0000233443, 0, 0 };

TEMP tableCoord, a, b, c, floorA, seedCoords, res;

ADD seedCoords, tex0, coordsOffset;

# gFastRngA = (((currentX*multX)/(currentY*multY))+
MUL tableCoord, seedCoords, coordMult0;
RCP tableCoord.y, tableCoord.y;
MUL a.x, tableCoord.x, tableCoord.y;

# (((height-currentY)*multX2)/((width-currentX)*multY2))+
SUB tableCoord, bounds, seedCoords;
MUL tableCoord, tableCoord, coordMult1;
RCP tableCoord.x, tableCoord.x;
MAD a.x, tableCoord.x, tableCoord.y, a.x;

# (((height-currentX)*multX3)/((width-currentY)*multY3)));
SUB tableCoord.x, bounds.y, seedCoords.x;
SUB tableCoord.y, bounds.x, seedCoords.y;
MUL tableCoord, tableCoord, coordMult2;
RCP tableCoord.y, tableCoord.y;
MAD a.x, tableCoord.x, tableCoord.y, a.x;

# gFastRngA = fmod(gFastRngA,1);
FRC a.x, a.x;
ADD a.x, a.x, seed;

MOV c.x, 0;
MOV b.x, 0;

# (gFastRngA += gFastRngC*(1./9973)+(3./11)-floor(gFastRngA))
FRC floorA.x, a.x;
SUB floorA.x, a.x, floorA.x;
SUB floorA.x, aSubtract.x, floorA.x;
ADD floorA.x, floorA.x, a.x;
MAD a.x, c.x, cMult.x, floorA.x;

# (gFastRngB += (gFastRngA *= gFastRngA))
MUL a.x, a.x, a.x;
ADD b.x, b.x, a.x;

# (gFastRngC += (gFastRngB -= floor(gFastRngB)))
FRC b.x, b.x;
ADD c.x, c.x, b.x;

# (gFastRngC -= floor(gFastRngC))
FRC c.x, c.x;

# (gFastRngA += gFastRngC*(1./9973)+(3./11)-floor(gFastRngA))
FRC floorA.x, a.x;
SUB floorA.x, a.x, floorA.x;
SUB floorA.x, aSubtract.x, floorA.x;
ADD floorA.x, floorA.x, a.x;
MAD a.x, c.x, cMult.x, floorA.x;

# (gFastRngB += (gFastRngA *= gFastRngA))
MUL a.x, a.x, a.x;
ADD b.x, b.x, a.x;

# (gFastRngC += (gFastRngB -= floor(gFastRngB)))
FRC b.x, b.x;
ADD c.x, c.x, b.x;

# (gFastRngC -= floor(gFastRngC))
FRC c.x, c.x;

# (gFastRngA += gFastRngC*(1./9973)+(3./11)-floor(gFastRngA))
FRC floorA.x, a.x;
SUB floorA.x, a.x, floorA.x;
SUB floorA.x, aSubtract.x, floorA.x;
ADD floorA.x, floorA.x, a.x;
MAD a.x, c.x, cMult.x, floorA.x;

# (gFastRngB += (gFastRngA *= gFastRngA))
MUL a.x, a.x, a.x;
ADD b.x, b.x, a.x;

# (gFastRngC += (gFastRngB -= floor(gFastRngB)))
FRC b.x, b.x;
ADD c.x, c.x, b.x;

# (gFastRngC -= floor(gFastRngC))
FRC c.x, c.x;


MOV res, c.x;
MOV res.a, 1;

MUL res, res, -0.5;

ADD res, res, col;

MOV result.color, res;

END
