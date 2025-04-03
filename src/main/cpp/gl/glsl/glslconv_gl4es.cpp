//
// Created by BZLZHH on 2025/4/3.
//

#include "glslconv_gl4es.h"
#include "khash.h"
#include "../log.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <clocale>

#define DEBUG 1

#define MAX_VTX_PROG_ENV_PARAMS 96
#define MAX_VTX_PROG_LOC_PARAMS 96
#define MAX_FRG_PROG_ENV_PARAMS 24
#define MAX_FRG_PROG_LOC_PARAMS 24
#define MAX_TEX 16

const char* gl4es_find_string(const char* pBuffer, const char* S);
char* gl4es_find_string_nc(char* pBuffer, const char* S);
int gl4es_count_string(const char* pBuffer, const char* S);
char* gl4es_resize_if_needed(char* pBuffer, int *size, int addsize);
char* gl4es_inplace_replace(char* pBuffer, int* size, const char* S, const char* D);
char* gl4es_append(char* pBuffer, int* size, const char* S);
char* gl4es_inplace_insert(char* pBuffer, const char* S, char* master, int* size);
char* gl4es_getline(char* pBuffer, int num);
int gl4es_countline(const char* pBuffer);
int gl4es_getline_for(const char* pBuffer, const char* S); // get the line number for 1st occurent of S in pBuffer
char* gl4es_str_next(char *pBuffer, const char* S); // mostly as strstr, but go after the substring if found
//"blank" (space, tab, cr, lf,":", ",", ";", ".", "/")
char* gl4es_next_str(char* pBuffer);   // go to next non "blank"
char* gl4es_prev_str(char* Str, char* pBuffer);    // go to previous non "blank"
char* gl4es_next_blank(char* pBuffer);   // go to next "blank"
char* gl4es_next_line(char* pBuffer);   // go to next new line (crlf not included)

const char* gl4es_get_next_str(char* pBuffer); // get a (static) copy of next str (until next separator), can be a simple number or separator also

// those function don't try to be smart with separators...
int gl4es_countstring_simple(char* pBuffer, const char* S);
char* gl4es_inplace_replace_simple(char* pBuffer, int* size, const char* S, const char* D);


const char* AllSeparators = " \t\n\r.,;()[]{}-<>+*/%&\\\"'^$=!:?";

char* gl4es_resize_if_needed(char* pBuffer, int *size, int addsize);

char* gl4es_inplace_replace(char* pBuffer, int* size, const char* S, const char* D)
{
    int lS = strlen(S), lD = strlen(D);
    pBuffer = gl4es_resize_if_needed(pBuffer, size, (lD-lS)*gl4es_count_string(pBuffer, S));
    char* p = pBuffer;
    while((p = strstr(p, S)))
    {
        // found an occurrence of S
        // check if good to replace, strchr also found '\0' :)
        if(strchr(AllSeparators, p[lS])!=NULL && (p==pBuffer || strchr(AllSeparators, p[-1])!=NULL)) {
            // move out rest of string
            memmove(p+lD, p+lS, strlen(p)-lS+1);
            // replace
            memcpy(p, D, strlen(D));
            // next
            p+=lD;
        } else p+=lS;
    }

    return pBuffer;
}

char* gl4es_inplace_insert(char* pBuffer, const char* S, char* master, int* size)
{
    char* m = gl4es_resize_if_needed(master, size, strlen(S));
    if(m!=master) {
        pBuffer += (m-master);
        master = m;
    }
    char* p = pBuffer;
    int lS = strlen(S), ll = strlen(pBuffer);
    memmove(p+lS, p, ll+1);
    memcpy(p, S, lS);

    return master;
}

char* gl4es_getline(char* pBuffer, int num)
{
    char *p = pBuffer;
    while(num-- && (p=strstr(p, "\n"))) p+=strlen("\n");
    return (p)?p:pBuffer;
}

int gl4es_countline(const char* pBuffer)
{
    int n=0;
    const char* p = pBuffer;
    while((p=strstr(p, "\n"))) {
        p+=strlen("\n");
        n++;
    }
    return n;
}

int gl4es_getline_for(const char* pBuffer, const char* S)
{
    int n=0;
    const char* p = pBuffer;
    const char* end = gl4es_find_string(pBuffer, S);
    if(!end)
        return 0;
    while((p=strstr(p, "\n"))) {
        p+=strlen("\n");
        n++;
        if(p>=end)
            return n;
    }
    return n;
}

int gl4es_count_string(const char* pBuffer, const char* S)
{
    const char* p = pBuffer;
    int lS = strlen(S);
    int n = 0;
    while((p = strstr(p, S)))
    {
        // found an occurrence of S
        // check if good to count, strchr also found '\0' :)
        if(strchr(AllSeparators, p[lS])!=NULL && (p==pBuffer || strchr(AllSeparators, p[-1])!=NULL))
            n++;
        p+=lS;
    }
    return n;
}

const char* gl4es_find_string(const char* pBuffer, const char* S)
{
    const char* p = pBuffer;
    int lS = strlen(S);
    while((p = strstr(p, S)))
    {
        // found an occurrence of S
        // check if good to count, strchr also found '\0' :)
        if(strchr(AllSeparators, p[lS])!=NULL && (p==pBuffer || strchr(AllSeparators, p[-1])!=NULL))
            return p;
        p+=lS;
    }
    return NULL;
}

char* gl4es_find_string_nc(char* pBuffer, const char* S)
{
    char* p = pBuffer;
    int lS = strlen(S);
    while((p = strstr(p, S)))
    {
        // found an occurrence of S
        // check if good to count, strchr also found '\0' :)
        if(strchr(AllSeparators, p[lS])!=NULL && (p==pBuffer || strchr(AllSeparators, p[-1])!=NULL))
            return p;
        p+=lS;
    }
    return NULL;
}

char* gl4es_resize_if_needed(char* pBuffer, int *size, int addsize) {
    char* p = pBuffer;
    int newsize = strlen(pBuffer)+addsize+1;
    if (newsize>*size) {
        newsize += 100;
        p = (char*)realloc(pBuffer, newsize);
        *size=newsize;
    }
    return p;
}

char* gl4es_append(char* pBuffer, int* size, const char* S) {
    char* p =pBuffer;
    p = gl4es_resize_if_needed(pBuffer, size, strlen(S));
    strcat(p, S);
    return p;
}

static int gl4es_is_blank(char c)  {
    switch(c) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
        case ':':
        case ',':
        case ';':
        case '/':
            return 1;
        default:
            return 0;
    }
}
char* gl4es_str_next(char *pBuffer, const char* S) {
    if(!pBuffer) return NULL;
    char *p = strstr(pBuffer, S);
    return (p)?p:(p+strlen(S));
}

char* gl4es_next_str(char* pBuffer) {
    if(!pBuffer) return NULL;
    while(gl4es_is_blank(*pBuffer))
        ++pBuffer;
    return pBuffer;
}

char* gl4es_prev_str(char* Str, char* pBuffer) {
    if(!pBuffer) return NULL;
    if(pBuffer == Str)
        return Str;
    // go to previous non blank
    do {
        --pBuffer;
    } while(gl4es_is_blank(*pBuffer) && (pBuffer!=Str));
    // go to blank
    while((pBuffer!=Str) && !gl4es_is_blank(*(pBuffer-1)))
        --pBuffer;
    return pBuffer;
}

char* gl4es_next_blank(char* pBuffer) {
    if(!pBuffer) return NULL;
    while(!gl4es_is_blank(*pBuffer))
        ++pBuffer;
    return pBuffer;
}

char* gl4es_next_line(char* pBuffer) {
    if(!pBuffer) return NULL;
    while(*pBuffer && *pBuffer!='\n')
        ++pBuffer;
    return pBuffer;
}

const char* gl4es_get_next_str(char* pBuffer) {
    static char buff[100] = {0};
    buff[0] = '\0';
    if(!pBuffer) return NULL;
    char* p1 = gl4es_next_str(pBuffer);
    if(!p1) return buff;
    char* p2 = gl4es_next_blank(p1);
    if(!p2) return buff;
    int i=0;
    while(p1!=p2 && i<99)
        buff[i++] = *(p1++);
    buff[i] = '\0';
    return buff;
}

int gl4es_countstring_simple(char* pBuffer, const char* S)
{
    char* p = pBuffer;
    int lS = strlen(S);
    int n = 0;
    while((p = strstr(p, S)))
    {
        // found an occurrence of S
        n++;
        p+=lS;
    }
    return n;
}

char* gl4es_inplace_replace_simple(char* pBuffer, int* size, const char* S, const char* D)
{
    int lS = strlen(S), lD = strlen(D);
    pBuffer = gl4es_resize_if_needed(pBuffer, size, (lD-lS)*gl4es_countstring_simple(pBuffer, S));
    char* p = pBuffer;
    while((p = strstr(p, S)))
    {
        // found an occurrence of S
        // move out rest of string
        memmove(p+lD, p+lS, strlen(p)-lS+1);
        // replace
        memcpy(p, D, strlen(D));
        // next
        p+=lD;
    }

    return pBuffer;
}

typedef enum {
    ARB_VERTEX = 0,
    //ARB_WEIGHT
    ARB_NORMAL = 2,
    ARB_COLOR,
    ARB_SECONDARY,
    ARB_FOGCOORD,
    // 6 and 7 are nothing specifics
    ARB_MULTITEXCOORD0 = 8,
    ARB_MULTITEXCOORD1,
    ARB_MULTITEXCOORD2,
    ARB_MULTITEXCOORD3,
    ARB_MULTITEXCOORD4,
    ARB_MULTITEXCOORD5,
    ARB_MULTITEXCOORD6,
    ARB_MULTITEXCOORD7,
    ARB_MULTITEXCOORD8,
    ARB_MULTITEXCOORD9,
    ARB_MULTITEXCOORD10,
    ARB_MULTITEXCOORD11,
    ARB_MULTITEXCOORD12,
    ARB_MULTITEXCOORD13,
    ARB_MULTITEXCOORD14,
    ARB_MULTITEXCOORD15,
    //ARB_POINTSIZE,   //this one is supported by GLES hardware
    ARB_MAX
} reserved_attrib_t;


typedef enum {
    COMP_VERTEX = 0,
    COMP_FOGCOORD,
    COMP_NORMAL,
    COMP_COLOR,
    COMP_SECONDARY,
    COMP_MULTITEXCOORD0,
    COMP_MULTITEXCOORD1,
    COMP_MULTITEXCOORD2,
    COMP_MULTITEXCOORD3,
    COMP_MULTITEXCOORD4,
    COMP_MULTITEXCOORD5,
    COMP_MULTITEXCOORD6,
    COMP_MULTITEXCOORD7,
    COMP_MULTITEXCOORD8,
    COMP_MULTITEXCOORD9,
    COMP_MULTITEXCOORD10,
    COMP_MULTITEXCOORD11,
    COMP_MULTITEXCOORD12,
    COMP_MULTITEXCOORD13,
    COMP_MULTITEXCOORD14,
    COMP_MULTITEXCOORD15,
    //COMP_WEIGHT
    //COMP_POINTSIZE,   //this one is supported by GLES hardware
    COMP_MAX
} compressed_attrib_t;

typedef enum {
    MAT_MV = 0,
    MAT_MV_I,
    MAT_MV_T,
    MAT_MV_IT,
    MAT_P,
    MAT_P_I,
    MAT_P_T,
    MAT_P_IT,
    MAT_MVP,
    MAT_MVP_I,
    MAT_MVP_T,
    MAT_MVP_IT,
    MAT_T0,
    MAT_T0_I,
    MAT_T0_T,
    MAT_T0_IT,
    MAT_T1,
    MAT_T1_I,
    MAT_T1_T,
    MAT_T1_IT,
    MAT_T2,
    MAT_T2_I,
    MAT_T2_T,
    MAT_T2_IT,
    MAT_T3,
    MAT_T3_I,
    MAT_T3_T,
    MAT_T3_IT,
    MAT_T4,
    MAT_T4_I,
    MAT_T4_T,
    MAT_T4_IT,
    MAT_T5,
    MAT_T5_I,
    MAT_T5_T,
    MAT_T5_IT,
    MAT_T6,
    MAT_T6_I,
    MAT_T6_T,
    MAT_T6_IT,
    MAT_T7,
    MAT_T7_I,
    MAT_T7_T,
    MAT_T7_IT,
    MAT_T8,
    MAT_T8_I,
    MAT_T8_T,
    MAT_T8_IT,
    MAT_T9,
    MAT_T9_I,
    MAT_T9_T,
    MAT_T9_IT,
    MAT_T10,
    MAT_T10_I,
    MAT_T10_T,
    MAT_T10_IT,
    MAT_T11,
    MAT_T11_I,
    MAT_T11_T,
    MAT_T11_IT,
    MAT_T12,
    MAT_T12_I,
    MAT_T12_T,
    MAT_T12_IT,
    MAT_T13,
    MAT_T13_I,
    MAT_T13_T,
    MAT_T13_IT,
    MAT_T14,
    MAT_T14_I,
    MAT_T14_T,
    MAT_T14_IT,
    MAT_T15,
    MAT_T15_I,
    MAT_T15_T,
    MAT_T15_IT,
    MAT_N,
    MAT_MAX
} reserved_matrix_t;

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#undef STR
#undef STR_HELPER

#define MAX_VATTRIB 32

typedef struct {
    char    name[50];
    int     state;      //0:disable, 1:warn, 1:enable, 2:require
} extension_t;

typedef struct {
    extension_t *ext;
    int         size;
    int         cap;
} extensions_t;

typedef struct hack_s {
    char* sign;
    int   n;
    char* next[5];
} hack_t;

static const hack_t gl4es_hacks[] = {
// this is for Guacamelee (yep, there is a lot of hacks, only int -> float conversions)
// 1
        {"float edgeGlow = step ( 0.2 , pow ( clamp ( ( dot ( vec2 ( 1 * sign ( v_texcoord3 . z ) , 1 ) , normalize ( quadCoord . xy - 0.5 ) ) - 0.4 + depth * 2.0 ) , 0.0  , 1.0  ) , 25 ) ) ;",
         1,
         {"float edgeGlow = step ( 0.2 , pow ( clamp ( ( dot ( vec2 ( 1.0 * sign ( v_texcoord3 . z ) , 1.0 ) , normalize ( quadCoord . xy - 0.5 ) ) - 0.4 + depth * 2.0 ) , 0.0  , 1.0  ) , 25.0 ) ) ;"}},
// 2
        {"float litfire = max ( dot ( normalize ( drops1 . rgb ) , normalize ( vec3 ( - 1 , 0 , pow ( max ( 1.0 - ocoord . x , 0.0 ) , 9 ) ) ) ) , 0 ) ;",
         1,
         {"float litfire = max ( dot ( normalize ( drops1 . rgb ) , normalize ( vec3 ( - 1.0 , 0.0 , pow ( max ( 1.0 - ocoord . x , 0.0 ) , 9.0 ) ) ) ) , 0.0 ) ;"}},
// 3
        {"if ( ( normalizedDepth ) < 0.0  ) discard ; ;\nif ( depth < 0 )",
         1,
         {"if ( ( normalizedDepth ) < 0.0  ) discard ; ;\nif ( depth < 0.0 )"}},
// 4
        {"gl_FragColor . rgba += glowHit ;\nif ( depth < 0 )",
         1,
         {"gl_FragColor . rgba += glowHit ;\nif ( depth < 0.0 )"}},
// 5
        {"gl_FragColor . a *= pow ( clamp ( ( depth + 1 ) , 0.0  , 1.0  ) , 70 ) ;",
         1,
         {"gl_FragColor . a *= pow ( clamp ( ( depth + 1.0 ) , 0.0  , 1.0  ) , 70.0 ) ;"}},
// 6
        {"if ( floor ( in_texcoord0 . y ) != 0 )",
         1,
         {"if ( floor ( in_texcoord0 . y ) != 0.0 )"}},
// 7
        {"if ( in_position0 . y < 0 )",
         1,
         {"if ( in_position0 . y < 0.0 )"}},
// 8
        {"if ( in_position0 . x < 0 )",
         1,
         {"if ( in_position0 . x < 0.0 )"}},
// 9
        {"branchB . y = 0 ;",
         1,
         {"branchB . y = 0.0 ;"}},
// 10
        {"branchB . x = 0 ;",
         1,
         {"branchB . x = 0.0 ;"}},

// this is for Battle Block Theater
// 1
        {"   if(texColor.w == 0)\n       gl_FragColor = texColor;",
         1,
         {"   if(texColor.w == 0.0)\n       gl_FragColor = texColor;"}},
// 2
        {"if(dist1 > 0)       {           float lightVal = (1-dist1) * light1Luminosity;",
         1,
         {"if(dist1 > 0.0)       {           float lightVal = (1.0-dist1) * light1Luminosity;"}},
// 3
        {"float lightVal = 0;",
         1,
         {"float lightVal = 0.0;"}},
// 4
        {"       if(dist1 > 0)\n"
         "       {\n"
         "			if(dist1 > 1)\n"
         "				dist1 = 1;\n",
         1,{
                 "       if(dist1 > 0.0)\n"
                 "       {\n"
                 "			if(dist1 > 1.0)\n"
                 "				dist1 = 1.0;\n"}},
// 5
        {"lightVal += (1-dist1) * light1Luminosity;",
         1,{"lightVal += (1.0-dist1) * light1Luminosity;"}},
// 6
        {"lightVal += (1-dist1) * light2Luminosity;",
         1,{"lightVal += (1.0-dist1) * light2Luminosity;"}},
// 7
        {"lightVal += (1-dist1) * light3Luminosity;",
         1,{"lightVal += (1.0-dist1) * light3Luminosity;"}},
// 8
        {"if(lightVal > 1)\n"
         "			lightVal = 1;",
         1,{
                 "if(lightVal > 1.0)\n"
                 "			lightVal = 1.0;"}},
// 9
        {"if(lightVal > 1)\n"
         "           lightVal = 1;", // space and tabs make a difference...
         1,{
                 "if(lightVal > 1.0)\n"
                 "           lightVal = 1.0;"}},

// For Night of the Zombie / Irrlicht 1.9.0
        {"gl_FragColor = (sample*(1-grayScaleFactor)) + (gray*grayScaleFactor);",
         1,{"gl_FragColor = (sample*(1.0-grayScaleFactor)) + (gray*grayScaleFactor);"}},

// For Knytt Underground
        {"vec2 val = texture_coordinate1+coeff*2*(i/float(iterations-1.0) - 0.5);",
         1,{"vec2 val = texture_coordinate1+coeff*2.0*(float(i)/float(iterations-1) - 0.5);"}},

        {"    b /= iterations;",
         1,{"    b /= float(iterations);"}},

// For Antichamber
        {"attribute vec4 _Un_AttrPosition0;\n"
         "vec4 Un_AttrPosition0 = _Un_AttrPosition0;\n",
         1, {"attribute vec4 _Un_AttrPosition0;\n"
             "#define Un_AttrPosition0 _Un_AttrPosition0\n"}},

        {"attribute vec4 _Un_AttrColor0;\n"
         "vec4 Un_AttrColor0 = _Un_AttrColor0;\n",
         1, {"attribute vec4 _Un_AttrColor0;\n"
             "#define Un_AttrColor0 _Un_AttrColor0\n"}},

        {"attribute vec4 _Un_AttrColor1;\n"
         "vec4 Un_AttrColor1 = _Un_AttrColor1;\n",
         1, {"attribute vec4 _Un_AttrColor1;\n"
             "#define Un_AttrColor1 _Un_AttrColor1\n"}},

        {"attribute vec4 _Un_AttrTangent0;\n"
         "vec4 Un_AttrTangent0 = _Un_AttrTangent0;\n",
         1, {"attribute vec4 _Un_AttrTangent0;\n"
             "#define Un_AttrTangent0 _Un_AttrTangent0\n"}},

        {"attribute vec4 _Un_AttrNormal0;\n"
         "vec4 Un_AttrNormal0 = _Un_AttrNormal0;\n",
         1, {"attribute vec4 _Un_AttrNormal0;\n"
             "#define Un_AttrNormal0 _Un_AttrNormal0\n"}},

        {"attribute vec4 _Un_AttrBlendIndices0;\n"
         "vec4 Un_AttrBlendIndices0 = _Un_AttrBlendIndices0;\n",
         1, {"attribute vec4 _Un_AttrBlendIndices0;\n"
             "#define Un_AttrBlendIndices0 _Un_AttrBlendIndices0\n"}},

        {"attribute vec4 _Un_AttrBlendWeight0;\n"
         "vec4 Un_AttrBlendWeight0 = _Un_AttrBlendWeight0;\n",
         1, {"attribute vec4 _Un_AttrBlendWeight0;\n"
             "#define Un_AttrBlendWeight0 _Un_AttrBlendWeight0\n"}},

        {"attribute vec4 _Un_AttrBinormal0;\n"
         "vec4 Un_AttrBinormal0 = _Un_AttrBinormal0;\n",
         1, {"attribute vec4 _Un_AttrBinormal0;\n"
             "#define Un_AttrBinormal0 _Un_AttrBinormal0\n"}},

        {"attribute vec4 _Un_AttrTexCoord0;\n"
         "vec4 Un_AttrTexCoord0 = _Un_AttrTexCoord0;\n",
         1, {"attribute vec4 _Un_AttrTexCoord0;\n"
             "#define Un_AttrTexCoord0 _Un_AttrTexCoord0\n"}},

        {"attribute vec4 _Un_AttrTexCoord1;\n"
         "vec4 Un_AttrTexCoord1 = _Un_AttrTexCoord1;\n",
         1, {"attribute vec4 _Un_AttrTexCoord1;\n"
             "#define Un_AttrTexCoord1 _Un_AttrTexCoord1\n"}},

        {"attribute vec4 _Un_AttrTexCoord2;\n"
         "vec4 Un_AttrTexCoord2 = _Un_AttrTexCoord2;\n",
         1, {"attribute vec4 _Un_AttrTexCoord2;\n"
             "#define Un_AttrTexCoord2 _Un_AttrTexCoord2\n"}},

        {"attribute vec4 _Un_AttrTexCoord3;\n"
         "vec4 Un_AttrTexCoord3 = _Un_AttrTexCoord3;\n",
         1, {"attribute vec4 _Un_AttrTexCoord3;\n"
             "#define Un_AttrTexCoord3 _Un_AttrTexCoord3\n"}},

        {"attribute vec4 _Un_AttrTexCoord4;\n"
         "vec4 Un_AttrTexCoord4 = _Un_AttrTexCoord4;\n",
         1, {"attribute vec4 _Un_AttrTexCoord4;\n"
             "#define Un_AttrTexCoord4 _Un_AttrTexCoord4\n"}},

        {"attribute vec4 _Un_AttrTexCoord5;\n"
         "vec4 Un_AttrTexCoord5 = _Un_AttrTexCoord5;\n",
         1, {"attribute vec4 _Un_AttrTexCoord5;\n"
             "#define Un_AttrTexCoord5 _Un_AttrTexCoord5\n"}},

        {"attribute vec4 _Un_AttrTexCoord6;\n"
         "vec4 Un_AttrTexCoord6 = _Un_AttrTexCoord6;\n",
         1, {"attribute vec4 _Un_AttrTexCoord6;\n"
             "#define Un_AttrTexCoord6 _Un_AttrTexCoord6\n"}},

        {"attribute vec4 _Un_AttrTexCoord7;\n"
         "vec4 Un_AttrTexCoord7 = _Un_AttrTexCoord7;\n",
         1, {"attribute vec4 _Un_AttrTexCoord7;\n"
             "#define Un_AttrTexCoord7 _Un_AttrTexCoord7\n"}},

// for IcewindDale
        {"uniform highp \tvec2 \t\tuTcScale;",
         1, {"uniform mediump vec2 \t\tuTcScale;"}},

// for OpenMW
        {"uniform bool simpleWater = false;",
         1, {"uniform bool simpleWater;"}},

// for Lethal League
        {"uniform vec4 Color = vec4(1.0, 1.0, 1.0, 1.0);",
         1, {"uniform vec4 Color;"}},

// for ioQuake3
        {"float c[5] = float[5](1.0, 0.9238795325, 0.7071067812, 0.3826834324, 0.0);",
         1, {"float c[5]; c[0]=1.0; c[1]=0.9238795325; c[2]=0.7071067812; c[3]=0.3826834324; c[4]=0.0;"}},

        {"float c[7] = float[7](1.0, 0.9659258263, 0.8660254038, 0.7071067812, 0.5, 0.2588190451, 0.0);",
         1, {"float c[7]; c[0]=1.0; c[1]=0.9659258263; c[2]=0.8660254038; c[3]=0.7071067812; c[4]=0.5; c[5]=0.2588190451; c[6]=0.0;"}},

        {"float scale = 2.0 / r_shadowMapSize;",
         1, {"float scale = 2.0 / float(r_shadowMapSize);"}},

        {"vec2 poissonDisc[9] = vec2[9](\n"
         "vec2(-0.7055767, 0.196515),    vec2(0.3524343, -0.7791386),\n"
         "vec2(0.2391056, 0.9189604),    vec2(-0.07580382, -0.09224417),\n"
         "vec2(0.5784913, -0.002528916), vec2(0.192888, 0.4064181),\n"
         "vec2(-0.6335801, -0.5247476),  vec2(-0.5579782, 0.7491854),\n"
         "vec2(0.7320465, 0.6317794)\n"
         ");\n",
         3, {
                 "vec2 poissonDisc[9];\n",

                 "void main()\n"
                 "{\n",
                 "void main()\n"
                 "{\n"
                 "poissonDisc[0] = vec2(-0.7055767, 0.196515);  poissonDisc[1] = vec2(0.3524343, -0.7791386);\n"
                 "poissonDisc[2] = vec2(0.2391056, 0.9189604);  poissonDisc[3] = vec2(-0.07580382, -0.09224417);\n"
                 "poissonDisc[4] = vec2(0.5784913, -0.002528916);poissonDisc[5]= vec2(0.192888, 0.4064181);\n"
                 "poissonDisc[6] = vec2(-0.6335801, -0.5247476);poissonDisc[7] = vec2(-0.5579782, 0.7491854);\n"
                 "poissonDisc[8] = vec2(0.7320465, 0.6317794);\n"
         }},

        {"float result = 0;",
         1, {"float result = 0.0;"}},

        {"//float gauss[5] = float[5](0.30, 0.23, 0.097, 0.024, 0.0033);\n"
         "float gauss[4] = float[4](0.40, 0.24, 0.054, 0.0044);\n"
         "//float gauss[3] = float[3](0.60, 0.19, 0.0066);\n"
         "#define GAUSS_SIZE 4\n",
         3, {
                 "//float gauss[5] = float[5](0.30, 0.23, 0.097, 0.024, 0.0033);\n"
                 "float gauss[4];\n"
                 "//float gauss[3] = float[3](0.60, 0.19, 0.0066);\n"
                 "#define GAUSS_SIZE 4\n",

                 "void main()\n"
                 "{\n",
                 "void main()\n"
                 "{\n"
                 "        gauss[0]=0.40; gauss[1]=0.24; gauss[2]=0.054; gauss[3]=0.0044;\n"
         }},

        {"vec2 offset = direction * j;",
         1,{"vec2 offset = direction * float(j);"}},

// for Silver
        {"#version 140\r\n"
         "\r\n"
         "out vec2 var_uv;\r\n"
         "in vec4 sg3d_position0;\r\n"
         "void main()\r\n"
         "{\r\n"
         "\tgl_Position = sg3d_position0;\r\n"
         "\tvar_uv=sg3d_position0.xy*0.5+0.5;\r\n"
         "}\r\n",
         1, {
                 "#version 120\n"
                 "\n"
                 "varying vec2 var_uv;\n"
                 "attribute vec4 sg3d_position0;\n"
                 "void main()\n"
                 "{\n"
                 "\tgl_Position = sg3d_position0;\n"
                 "\tvar_uv=sg3d_position0.xy*0.5+0.5;\n"
                 "}\n"
         }},

        {"#version 140\r\n"
         "\r\n"
         "in vec2 var_uv;\r\n"
         "out vec4 var_out;\r\n"
         "uniform vec4 var_param;\r\n"
         "uniform sampler2D SG3D_TEXTURE_DIFFUSE;\r\n"
         "void main()\r\n"
         "{\r\n"
         "\tvar_out = texture(SG3D_TEXTURE_DIFFUSE, var_uv)*var_param;\r\n"
         "}\r\n",
         1, {"#version 120\n"
             "\n"
             "varying vec2 var_uv;\n"
             "#define var_out gl_FragColor\n"
             "uniform vec4 var_param;\n"
             "uniform sampler2D SG3D_TEXTURE_DIFFUSE;\n"
             "void main()\n"
             "{\n"
             "\tvar_out = texture2D(SG3D_TEXTURE_DIFFUSE, var_uv)*var_param;\n"
             "}\n"
         }},

        {"#version 140\r\n"
         "\r\n"
         "in vec2 var_uv;\r\n"
         "out vec4 var_out;\r\n"
         "uniform vec4 var_param;\r\n"
         "uniform sampler2D SG3D_TEXTURE_DIFFUSE;\r\n"
         "void main()\r\n"
         "{\r\n"
         "\tvar_out = (texture(SG3D_TEXTURE_DIFFUSE, var_uv+var_param.xy*-7.0)*55.0+\r\n"
         "\t\t\ttexture(SG3D_TEXTURE_DIFFUSE, var_uv+var_param.xy*-3.0)*330.0+\r\n"
         "\t\t\ttexture(SG3D_TEXTURE_DIFFUSE, var_uv)*252.0+\r\n"
         "\t\t\ttexture(SG3D_TEXTURE_DIFFUSE, var_uv+var_param.xy* 3.0)*330.0+\r\n"
         "\t\t\ttexture(SG3D_TEXTURE_DIFFUSE, var_uv+var_param.xy* 7.0)*55.0)/1022.0;\r\n"
         "}\r\n",
         1, {"#version 120\n"
             "\n"
             "varying vec2 var_uv;\n"
             "#define var_out gl_FragColor\n"
             "uniform vec4 var_param;\n"
             "uniform sampler2D SG3D_TEXTURE_DIFFUSE;\n"
             "void main()\n"
             "{\n"
             "\tvar_out = (texture2D(SG3D_TEXTURE_DIFFUSE, var_uv+var_param.xy*-7.0)*55.0+\n"
             "\t\t\ttexture2D(SG3D_TEXTURE_DIFFUSE, var_uv+var_param.xy*-3.0)*330.0+\n"
             "\t\t\ttexture2D(SG3D_TEXTURE_DIFFUSE, var_uv)*252.0+\n"
             "\t\t\ttexture2D(SG3D_TEXTURE_DIFFUSE, var_uv+var_param.xy* 3.0)*330.0+\n"
             "\t\t\ttexture2D(SG3D_TEXTURE_DIFFUSE, var_uv+var_param.xy* 7.0)*55.0)/1022.0;\n"
             "}\n"
         }},

        {"#version 140\r\n"
         "\r\n"
         "in vec2 var_uv;\r\n"
         "out vec4 var_out;\r\n"
         "uniform vec4 var_param;\r\n"
         "uniform sampler2D SG3D_TEXTURE_DIFFUSE0;\r\n"
         "uniform sampler2D SG3D_TEXTURE_DIFFUSE1;\r\n"
         "uniform sampler2D SG3D_TEXTURE_DIFFUSE2;\r\n"
         "\r\n"
         "const mat3 yuv2rgb =   mat3(1,0,1.596,1,-0.391,-0.813,1,2.018,0);\r\n"
         "\r\n"
         "void main()\r\n"
         "{\r\n"
         "\tfloat y=texture(SG3D_TEXTURE_DIFFUSE0, var_uv).x;\r\n"
         "\tfloat u=texture(SG3D_TEXTURE_DIFFUSE1, var_uv).x;\r\n"
         "\tfloat v=texture(SG3D_TEXTURE_DIFFUSE2, var_uv).x;\r\n"
         "\r\n"
         "\tvec3 rgb=vec3(1.1643*(y-0.0625), u-0.5, v-0.5)*yuv2rgb;\r\n"
         "\tvar_out = vec4(rgb, 1.0)*var_param;\r\n"
         "}\r\n",
         1, {"#version 120\n"
             "\n"
             "varying vec2 var_uv;\n"
             "#define var_out gl_FragColor\n"
             "uniform vec4 var_param;\n"
             "uniform sampler2D SG3D_TEXTURE_DIFFUSE0;\n"
             "uniform sampler2D SG3D_TEXTURE_DIFFUSE1;\n"
             "uniform sampler2D SG3D_TEXTURE_DIFFUSE2;\n"
             "\n"
             "const mat3 yuv2rgb =   mat3(1.0,0.0,1.596,1.0,-0.391,-0.813,1.0,2.018,0.0);\n"
             "\n"
             "void main()\n"
             "{\n"
             "\tfloat y=texture2D(SG3D_TEXTURE_DIFFUSE0, var_uv).x;\n"
             "\tfloat u=texture2D(SG3D_TEXTURE_DIFFUSE1, var_uv).x;\n"
             "\tfloat v=texture2D(SG3D_TEXTURE_DIFFUSE2, var_uv).x;\n"
             "\n"
             "\tvec3 rgb=vec3(1.1643*(y-0.0625), u-0.5, v-0.5)*yuv2rgb;\n"
             "\tvar_out = vec4(rgb, 1.0)*var_param;\n"
             "}\n"
         }},

// for Eldritch
        {"float\tFogOffsetU\t= 0.5f / FogTexSize;\r\n"
         "float\tFogScaleU\t= ( FogTexSize - 1.0f ) / FogTexSize;\r\n",
         3, {"float   FogOffsetU;\r\n"
             "float   FogScaleU;\r\n",
             "void main()\r\n"
             "{\r\n",
             "void main()\r\n"
             "{\r\n"
             "\tFogOffsetU = 0.5 / FogTexSize;\r\n"
             "\tFogScaleU  = ( FogTexSize - 1.0 ) / FogTexSize;\r\n"
         }},
// for KiCAD
        {"        float derivative   = length( dFdx( tex ) ) * u_fontTextureWidth / 4;\n",
         1, {"        float derivative   = length( dFdx( tex ) ) * float(u_fontTextureWidth) / 4.0;\n"}},
        {"        delta = vec4( 0, 2 * pixelR, 0, 0 );\n",
         1, {"        delta = vec4( 0, 2.0 * pixelR, 0, 0 );\n"}},
// for Iconoclasts
// Disable hotspot shaders 1
#ifdef GOA_CLONE
        {
    "void main()\n"
    "{\n"
    "    vec4 p = texture2D(texture, texture_coordinate0);\n"
    "    float factor = (0.5 - p.r) * magnification;\n"
    "    vec2 t = texture_coordinate1;\n"
    "    t.x += (t.x - hotspotX) * factor;\n"
    "    t.y += (t.y - hotspotY) * factor;\n"
    "    gl_FragColor = sample_backtex(background_texture, t) * gl_Color;\n"
    "}",
    1,
    {
    "void main() { discard; }"
    }
},
// Disable hotspot shaders 2
{
    "void main()\n"
    "{\n"
    "    vec4 p = texture2D(texture, texture_coordinate0) * gl_Color;\n"
    "    if (p.a != 0.0) {\n"
    "        float zoomFactor = (0.0 - (p.r + p.g + p.b) / 3.0) * magnification;\n"
    "        vec2 t = texture_coordinate1;\n"
    "        t.x += (t.x - hotspotX + offx / texture_size.x) * zoomFactor;\n"
    "        t.y += (t.y - hotspotY + offy / texture_size.y) * zoomFactor;\n"
    "        gl_FragColor = sample_backtex(background_texture, t);\n"
    "        return;\n"
    "    }\n"
    "    gl_FragColor = p;\n"
    "}",
    1,
    {
    "void main() { discard; }"
    }
},
// Disable hotspot shaders 3
{
    "void main()\n"
    "{\n"
    "    vec4 p = texture2D(texture, texture_coordinate0) * gl_Color;\n"
    "    float factor = (0.0 - (p.r + p.g + p.b) / 3.0) * magnification;\n"
    "    vec2 t = background_offset.xy - texture_coordinate1 * \n"
    "                                    (background_offset.xy * 2.0 - vec2(1.0));\n"
    "    t.x += (t.x - hotspotX) * factor;\n"
    "    t.y += (t.y - hotspotY) * factor;\n"
    "    gl_FragColor = sample_backtex(background_texture, t);\n"
    "}",
    1,
    {"void main() { discard; }"}
}
#endif
};

// For Stellaris
static const char* gl4es_sign_1[] = {
        "if (Data.Type == 1)",
        "if (Data.BlendMode == 0)",
};
static const char* gl4es_hacks_1[] = {
        "if (Data.Type == 1)",
        "if (Data.Type == 1.0)",

        "if (Data.Type == 2)",
        "if (Data.Type == 2.0)",

        "if (Data.Type == 3)",
        "if (Data.Type == 3.0)",

        "if (Data.BlendMode == 0)",
        "if (Data.BlendMode == 0.0)",

        "if (Data.BlendMode == 1)",
        "if (Data.BlendMode == 1.0)",

        "if (Data.BlendMode == 2)",
        "if (Data.BlendMode == 2.0)",

        "Out.vMaskingTexCoord = saturate(v.vTexCoord * 1000);",
        "Out.vMaskingTexCoord = saturate(v.vTexCoord * 1000.0);",

        "float vTime = 0.9 - saturate( (Time - AnimationTime) * 4 );",
        "float vTime = 0.9 - saturate( (Time - AnimationTime) * 4.0 );",

        "float vTime = 0.9 - saturate( (Time - AnimationTime) * 16 );",
        "float vTime = 0.9 - saturate( (Time - AnimationTime) * 16.0 );",
};

// For Psychonauts
static const char* gl4es_sign_2[] = {
        "vec4 ps_t3 = gl_TexCoord[3];",
        "vec4 ps_t2 = gl_TexCoord[2];",
        "vec4 ps_t1 = gl_TexCoord[1];",
        "vec4 ps_t0 = gl_TexCoord[0];",
};

static const char* gl4es_sign_2_main =
        "void main()\n"
        "{\n";


static const char* gl4es_hacks_2_1[] = {
        "vec4 ps_t3;",
        "vec4 ps_t2;",
        "vec4 ps_t1;",
        "vec4 ps_t0;",
};

static const char* gl4es_hacks_2_2[] = {
        "\tps_t3 = gl_TexCoord[3];",
        "\tps_t2 = gl_TexCoord[2];",
        "\tps_t1 = gl_TexCoord[1];",
        "\tps_t0 = gl_TexCoord[0];",
};

static char* ShaderHacks_1(char* shader, char* Tmp, int* tmpsize)
{
    // check for all signature first
    for (int i=0; i<sizeof(gl4es_sign_1)/sizeof(gl4es_sign_1[0]); i++)
        if(!strstr(Tmp, gl4es_sign_1[i]))
            return Tmp;
    // Do the replace
    for (int i=0; i<sizeof(gl4es_hacks_1)/sizeof(gl4es_hacks_1[0]); i+=2)
        if(strstr(Tmp, gl4es_hacks_1[i])) {
            if(Tmp==shader) {Tmp = (char*)malloc(*tmpsize); strcpy(Tmp, shader);}   // hacking!
            Tmp = gl4es_inplace_replace_simple(Tmp, tmpsize, gl4es_hacks_1[i], gl4es_hacks_1[i+1]);
        }
    return Tmp;
}

static char* ShaderHacks_2_1(char* shader, char* Tmp, int* tmpsize, int i)
{
    char* p = strstr(Tmp, gl4es_sign_2[i]);
    if(!p) return Tmp;  // not found
    char* m = strstr(Tmp, gl4es_sign_2_main);
    if(!m) return Tmp;  // main signature not found
    if((uintptr_t)p > (uintptr_t)m) return Tmp; // main is before, aborting...
    // ok, instance found, insert main line...
    if(Tmp==shader) {Tmp = (char*)malloc(*tmpsize); strcpy(Tmp, shader); m = strstr(Tmp, gl4es_sign_2_main);}   // hacking!
    m += strlen(gl4es_sign_2_main);
    Tmp = gl4es_inplace_insert(m, gl4es_hacks_2_2[i], Tmp, tmpsize);
    Tmp = gl4es_inplace_replace_simple(Tmp, tmpsize, gl4es_sign_2[i], gl4es_hacks_2_1[i]);
    return Tmp;
}

static char* ShaderHacks_2(char* shader, char* Tmp, int* tmpsize)
{
    // check for each signature
    for (int i=0; i<sizeof(gl4es_sign_2)/sizeof(gl4es_sign_2[0]); i++)
        Tmp = ShaderHacks_2_1(shader, Tmp, tmpsize, i);
    return Tmp;
}

char* ShaderHacks(char* shader)
{
    char* Tmp = shader;
    int tmpsize = strlen(Tmp)+10;
    // specific hacks
    Tmp = ShaderHacks_1(shader, Tmp, &tmpsize);
    Tmp = ShaderHacks_2(shader, Tmp, &tmpsize);
    // generic
    for (int i=0; i<sizeof(gl4es_hacks)/sizeof(gl4es_hacks[0]); ++i) {
        char* f = gl4es_hacks[i].sign;
        int n = gl4es_hacks[i].n;
        if(strstr(Tmp, f)) {
            if(Tmp==shader) {Tmp = (char*)malloc(tmpsize); strcpy(Tmp, shader);}   // hacking!
            for (int j=0; j<n; j+=2) {
                if(j) f = gl4es_hacks[i].next[j-1];
                Tmp = gl4es_inplace_replace_simple(Tmp, &tmpsize, f, gl4es_hacks[i].next[j]);
            }
        }
    }
    return Tmp;
}

typedef enum _eTokenType {
    TK_NULL = 0,
    TK_SPACE,
    TK_SHARP,
    TK_NEWLINE,
    TK_INT,
    TK_FLOAT,
    TK_PLUS,
    TK_MINUS,
    TK_EQUAL,
    TK_SLASH,
    TK_DOUBLESLASH, //10
    TK_MULTIPLY,
    TK_BACKSLASH,
    TK_OPENBRACE,
    TK_CLOSEBRACE,
    TK_OPENCURLY,
    TK_CLOSECURLY,
    TK_OPENBRAKET,
    TK_CLOSEBRAKET,
    TK_OPENCOMMENT,
    TK_CLOSECOMMENT,    //20
    TK_COLUMN,
    TK_SEMICOLUMN,
    TK_COMMA,
    TK_DOT,
    TK_AMP,
    TK_POW,
    TK_PIPE,
    TK_EXCLAM,
    TK_POINT,   //30
    TK_GREATER,
    TK_LESS,
    TK_DOUBLEEQUAL,
    TK_TILDE,
    TK_TEXT,
    TK_TEXTCOMMENT
} eTokenType;

#define MAXSTR 500

typedef struct _uToken {
    eTokenType type;
    char    str[MAXSTR];
    int     integer;
    float   real;
} uToken;

eTokenType NextTokenComment(char **p, uToken *tok) {
    tok->type = TK_NULL;
    tok->str[0] = 0;
    if(!**p) return tok->type;

    char c = **p;
    (*p)++;
    char nextc = **p;
    int nb = 0;
    int isfloat = 0;
    int isneg = 1;  // multiply by it...
    float fnb = 0.f;
    int cnt = 0;
    float fcnt = 0.f;

    tok->str[0]=c; tok->str[1]=0;

    switch (c) {
        case 10:
            if(nextc==13)
                (*p)++;
            strcpy(tok->str, "\n");
            tok->type = TK_NEWLINE;
            break;
        case 13:
            if(nextc==10)
                (*p)++;
            strcpy(tok->str, "\n");
            tok->type = TK_NEWLINE;
            break;
        case ' ':
        case '\t':
            while(nextc==' ' || nextc=='\t') { int l=strlen(tok->str); tok->str[l]=nextc; tok->str[l+1]=0; (*p)++; nextc=**p; }
            tok->type = TK_SPACE;
            break;
        case '/':
            if(nextc=='/') {
                (*p)++;
                tok->type=TK_DOUBLESLASH;
                strcpy(tok->str, "//");
            } else if(nextc=='*') {
                (*p)++;
                tok->type=TK_OPENCOMMENT;
                strcpy(tok->str, "/*");
            } else {
                tok->type=TK_SLASH;
            }
            break;
        case '*':
            if(nextc=='/') {
                (*p)++;
                tok->type=TK_CLOSECOMMENT;
                strcpy(tok->str, "*/");
            } else {
                tok->type=TK_MULTIPLY;
            }
            break;
        default:
            // all other are plain Ids...
            cnt=1;
            tok->type=TK_TEXTCOMMENT;
            while(cnt!=(MAXSTR-1) && (nextc=='_' || (nextc>='0' && nextc<='9') || (nextc>='A' && nextc<='Z') || (nextc>='a' && nextc<='z'))) {
                tok->str[cnt]=nextc;
                (*p)++; nextc=**p;
                ++cnt;
            }
            tok->str[cnt]=0;
    }

    return tok->type;
}

eTokenType NextToken(char **p, uToken *tok) {
    tok->type = TK_NULL;
    tok->str[0] = 0;
    if(!**p) return tok->type;

    char c = **p;
    (*p)++;
    char nextc = **p;
    int nb = 0;
    int isfloat = 0;
    int isneg = 1;  // multiply by it...
    float fnb = 0.f;
    int cnt = 0;
    float fcnt = 0.f;

    tok->str[0]=c; tok->str[1]=0;

    switch (c) {
        case 10:
            if(nextc==13)
                (*p)++;
            tok->type = TK_NEWLINE;
            strcpy(tok->str, "\n");
            break;
        case 13:
            if(nextc==10)
                (*p)++;
            tok->type = TK_NEWLINE;
            strcpy(tok->str, "\n");
            break;
        case ' ':
        case '\t':
            while(nextc==' ' || nextc=='\t') { int l=strlen(tok->str); tok->str[l]=nextc; tok->str[l+1]=0; (*p)++; nextc=**p; }
            tok->type = TK_SPACE;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            nb=c-'0';
            // we don't know if it's an int or a float at first...
            if(nextc=='x' && nb==0) {
                // hex number, so int...
                (*p)++; nextc=**p;
                while((nextc>='0' && nextc<='9') || (nextc>='a' && nextc<='f') || (nextc>='A' && nextc<='F')) {
                    nb=nb*16;
                    if(nextc>='0' && nextc<='9')
                        nb+=nextc-'0';
                    else if(nextc>='a' && nextc<='f')
                        nb+=nextc-'a'+10;
                    else if(nextc>='A' && nextc<='F')
                        nb+=nextc-'A'+10;
                    (*p)++; nextc=**p;
                }
                tok->type = TK_INT;
                tok->integer = nb;
                sprintf(tok->str, "0x%x", nb);
            } else {
                while(nextc>='0' && nextc<='9') { nb=nb*10+nextc-'0'; (*p)++; nextc=**p;}
                if(nextc=='.' || nextc=='f' || nextc=='e') {
                    isfloat = 1;
                    fnb=nb;
                    if(nextc=='.') {
                        (*p)++; nextc=**p;
                        fcnt = 0.1f;
                        while(nextc>='0' && nextc<='9') {
                            fnb+=(nextc-'0')*fcnt;
                            fcnt/=10.f;
                            (*p)++; nextc=**p;
                        }
                    }
                    if(nextc=='e') {
                        // exponent
                        (*p)++; nextc=**p;
                        nb=0;
                        int expsign = 1;
                        if(nextc=='-' || nextc=='+') {
                            if(nextc=='-') expsign = -1;
                            (*p)++; nextc=**p;
                        }
                        while(nextc>='0' && nextc<='9') { nb=nb*10+nextc-'0'; (*p)++; nextc=**p;}
                        fnb *= powf(10, nb*expsign); // exp10f is a GNU extension
                    }
                    if(nextc=='f') {
                        (*p)++; nextc=**p;
                    }
                    fnb*=isneg;
                    tok->type = TK_FLOAT;
                    tok->real = fnb;
                    sprintf(tok->str, "%#g", fnb);
                } else {
                    tok->type = TK_INT;
                    tok->integer = nb;
                    sprintf(tok->str, "%d", nb);
                }
            }
            break;
        case '/':
            if(nextc=='/') {
                (*p)++;
                tok->type=TK_DOUBLESLASH;
                strcpy(tok->str, "//");
            } else if(nextc=='*') {
                (*p)++;
                tok->type=TK_OPENCOMMENT;
                strcpy(tok->str, "/*");
            } else {
                tok->type=TK_SLASH;
            }
            break;
        case '*':
            if(nextc=='/') {
                (*p)++;
                tok->type=TK_CLOSECOMMENT;
                strcpy(tok->str, "*/");
            } else {
                tok->type=TK_MULTIPLY;
            }
            break;
        case '=': tok->type=TK_EQUAL; break;
        case '~': tok->type=TK_TILDE; break;
        case '#': tok->type=TK_SHARP; break;
        case '(': tok->type=TK_OPENBRACE; break;
        case ')': tok->type=TK_CLOSEBRACE; break;
        case '{': tok->type=TK_OPENCURLY; break;
        case '}': tok->type=TK_CLOSECURLY; break;
        case '[': tok->type=TK_OPENBRAKET; break;
        case ']': tok->type=TK_CLOSEBRAKET; break;
        case '&': tok->type=TK_AMP; break;
        case '^': tok->type=TK_POW; break;
        case '|': tok->type=TK_PIPE; break;
        case '\\': tok->type=TK_BACKSLASH; break;
        case '<': tok->type=TK_LESS; break;
        case '>': tok->type=TK_GREATER; break;
        case '+': tok->type=TK_PLUS; break;
        case '-': tok->type=TK_MINUS; break;
        case ':': tok->type=TK_COLUMN; break;
        case ';': tok->type=TK_SEMICOLUMN; break;
        case ',': tok->type=TK_COMMA; break;
        case '.': tok->type=TK_DOT; break;
        case '!': tok->type=TK_EXCLAM; break;
            // todo: char and string?
        default:
            // all other are plain Ids...
            cnt=1;
            tok->type=TK_TEXT;
            while(cnt!=(MAXSTR-1) && (nextc=='_' || (nextc>='0' && nextc<='9') || (nextc>='A' && nextc<='Z') || (nextc>='a' && nextc<='z'))) {
                tok->str[cnt]=nextc;
                (*p)++; nextc=**p;
                ++cnt;
            }
            tok->str[cnt]=0;
    }

    return tok->type;
}

eTokenType GetToken(char **p, uToken *tok, int incomment) {
    eTokenType ret;
    if (incomment)
        ret = NextTokenComment(p, tok);
    else
        ret = NextToken(p, tok);
    return ret;
}

typedef struct {
    int cap;
    int sz;
    int *ifs;
} stackif_t;

static void push_if(stackif_t *st, int v) {
    if(st->sz == st->cap) {
        st->cap += 16;
        st->ifs = (int*)realloc(st->ifs, sizeof(int)*st->cap);
    }
    st->ifs[st->sz++] = v;
}

static int pop_if(stackif_t *st) {
    if(st->sz)
        return st->ifs[--st->sz];
    return -1;
}

static int top_if(stackif_t *st) {
    if(st->sz)
        return st->ifs[st->sz-1];
    return -1;
}
static void not_top_if(stackif_t *st) {
    if(st->sz && st->ifs[st->sz-1]!=-1)
        st->ifs[st->sz-1] = 1 - st->ifs[st->sz-1];
}

static int result_if(stackif_t *st) {
    for (int i=0; i<st->sz; ++i) {
        if(st->ifs[i] == 1) return 1;
        else if(st->ifs[i] == -1) return -1;
    }
    return 0;
}

KHASH_MAP_INIT_STR(define, int);
KHASH_MAP_INIT_STR(alldefine, char*);

#define MAXSTR 500

char* preproc(const char* code, int keepcomments, int gl_es, extensions_t* exts, char** versionString) {
    LOG_D("Preproc on: =========\n%s\n=================\n", code)

    uToken tok;
    char* p = (char*)code;
    char* oldp = NULL;
    int cap=1000;
    char* ncode = (char*)malloc(1000);
    ncode[0]=0;
    int sz=1;
    int status=0;
    int write=1;
    int incomment=0;
    int indefined=0;
    int newline=0;
    int gettok=0;
    int notok = 0;
    char extname[50];
    khint_t k;
    int ret;
    char *defname;   //used for #define or #if
    int  defval;
    stackif_t stackif = {0};
    int nowrite_ifs = 0;
    int current_if = 0;
    int need_pop = 0;

    char* old_locale = setlocale(LC_ALL, "C");

    khash_t(alldefine) *alldefines = kh_init(alldefine);  // will conatin all defines, even the one without int inside

    khash_t(define) *defines = kh_init(define);
    if(gl_es) {
        k = kh_put(define, defines, "GL_ES", &ret);
        kh_value(defines, k) = 0;
    }
    push_if(&stackif, 0);   // default to OK
    GetToken(&p, &tok, incomment);
    while(tok.type!=TK_NULL) {
        // pop #if / #endif status
        if(need_pop) {
            pop_if(&stackif);
            nowrite_ifs = result_if(&stackif);
            need_pop = 0;

        }
        // pre get token switch
        switch(status) {
            case 110:   // line comment done...
            case 210:   // block comment done
                if(!write && newline) {
                    gettok=0;
                    tok.type=TK_NEWLINE;
                    strcpy(tok.str, "\n");
                }
                write = 1;
                status = (status==210)?1:0;
                incomment=0;
                newline=0;
                break;
        }
        // get token (if needed)
        if (gettok) GetToken(&p, &tok, incomment);
        gettok=1;
        // post get token switch
        if(tok.type!=TK_NULL) {
            switch(status) {
                case 0: // regular...
                case 1:
                    if(tok.type==TK_DOUBLESLASH) {
                        status = 100; // line comment
                        incomment = 1;
                        newline = 1;
                        if(!keepcomments) write=0;
                    } else if(tok.type==TK_OPENCOMMENT) {
                        status = 200; // multi-line comment
                        incomment = 1;
                        if(!keepcomments) write=0;
                    } else if(tok.type==TK_SHARP && !incomment && status==0) {
                        oldp = p-1; // lets buffer the line
                        status = 300;
                    } else if(tok.type==TK_NEWLINE)
                        status = 0;
                    else if(tok.type!=TK_SPACE)
                        status = 1; // everything else but space set status to 1...
                    break;

                    // line comment...
                case 100:
                    if(tok.type==TK_BACKSLASH) {
                        status = 120;   // is it backslash+endline for multiline?
                    } else if(tok.type==TK_NEWLINE) {
                        status = 110;
                    }
                    break;
                case 120:
                    status = 100;   // continue comment, what ever follow (NewLine or anything else)
                    break;

                    // block comment...
                case 200:
                    if(tok.type==TK_NEWLINE)
                        newline=1;
                    if(tok.type==TK_CLOSECOMMENT) {
                        status=210;
                    }
                    break;

                    // # (of ifdef or extension)
                case 300:
                    if(tok.type!=TK_SPACE)
                        if(tok.type==TK_TEXT) {
                            if(!strcmp(tok.str, "ifdef"))
                                status=310;
                            else if(!strcmp(tok.str, "ifndef"))
                                status=320;
                            else if(!strcmp(tok.str, "if")) {
                                // #if defined(GL_ES) not supported for now
                                push_if(&stackif, -1);
                                if(nowrite_ifs==1) {
                                    status = 398; notok = 1;
                                } else {
                                    status = 390;
                                    {
                                        int l = p - oldp;
                                        memcpy(tok.str, oldp, l);
                                        tok.str[l]='\0';
                                        oldp = 0;
                                    }
                                }
                                nowrite_ifs = result_if(&stackif);
                            } else if(!strcmp(tok.str, "else")) {
                                status = 399;
                                // ifs handling
                                {
                                    not_top_if(&stackif);
                                    int v = result_if(&stackif);
                                    if(v!=-1) {
                                        notok = 1;
                                        status = 398;
                                    }
                                    nowrite_ifs = v;
                                }
                            } else if(!strcmp(tok.str, "endif")) {
                                status = 399;
                                {
                                    need_pop = 1;
                                    if(nowrite_ifs!=-1) {
                                        notok = 1;
                                        status = 398;
                                    }
                                }
                            } else if(!strcmp(tok.str, "extension")) {
                                status = 410;
                            } else if(!strcmp(tok.str, "pragma")) {
                                status = 510;
                            } else if(!strcmp(tok.str, "define")) {
                                status = 610;
                            } else if(!strcmp(tok.str, "version")) {
                                status = 810;
                                if(!*versionString)
                                    *versionString = (char*)calloc(1, 51);
                            } else status=399;
                        } else status = 399;  // meh?
                    break;

                    // ifdef
                case 310:
                    if(tok.type==TK_SPACE)
                        status = 310;
                    else if(tok.type==TK_TEXT) {
                        int v = -1;
                        if(gl_es && (strcmp(tok.str, "GL_ES")==0))
                            v = 1;
                        else if(kh_get(alldefine, alldefines, tok.str)!=kh_end(alldefines)) {
                            v = 0;
                        } else if (strncmp(tok.str, "GL_", 3)==0)
                            v = -1;
                        push_if(&stackif, v);
                        nowrite_ifs = result_if(&stackif);
                        if(nowrite_ifs!=-1) {
                            status = 398; notok = 1;
                        } else
                            status = 399;
                    } else {push_if(&stackif, -1); nowrite_ifs = result_if(&stackif);status = 399;}
                    break;

                    // ifndef
                case 320:
                    if(tok.type==TK_SPACE)
                        status = 320;
                    else if(tok.type==TK_TEXT) {
                        int v = -1;
                        if(gl_es && strcmp(tok.str, "GL_ES")==0)
                            v = 0;
                        else if(kh_get(alldefine, alldefines, tok.str)!=kh_end(alldefines)) {
                            v = 1;
                        } else if (strncmp(tok.str, "GL_", 3)==0)
                            v = -1;
                        push_if(&stackif, v);
                        nowrite_ifs = result_if(&stackif);
                        if(nowrite_ifs!=-1) {
                            status = 398; notok = 1;
                        } else
                            status = 399;
                    } else {push_if(&stackif, -1); nowrite_ifs = result_if(&stackif); status = 399;}
                    break;

                    // #if ...
                case 390:
                    if (tok.type == TK_NEWLINE) {
                        status = 0;
                    } else if (tok.type == TK_TEXT) {
                        if(!strcmp(tok.str, "defined"))
                            status = 710;
                    }
                    break;

                    // end of #ifdef GL_ES and variant..
                case 398:
                    if(tok.type==TK_NEWLINE) {
                        oldp = NULL;
                        status = 0;
                    }
                    break;

                    // fallback for #ifdef GL_ES, write the line back...
                case 399:
                    if(oldp)
                    {
                        int l = p - oldp;
                        memcpy(tok.str, oldp, l);
                        tok.str[l]='\0';
                        oldp = NULL;
                    }
                    status = (tok.type==TK_NEWLINE)?0:1;
                    break;
                    // #extension
                case 410:
                    if(tok.type==TK_SPACE) {
                        // nothing...
                    } else if(tok.type==TK_TEXT && strlen(tok.str)<50) {
                        strcpy(extname, tok.str);
                        status = 420;
                    } else {
                        status = 399; // fallback, syntax error...
                    }
                    break;
                    // after the name and before the ':' of #extension
                case 420:
                    if(tok.type==TK_SPACE) {
                        // nothing...
                    } else if (tok.type==TK_COLUMN) {
                        status=430;
                    } else {
                        status = 399; // fallback, syntax error...
                    }
                    break;
                    // after the ':' of #extension
                case 430:
                    if(tok.type==TK_SPACE) {
                        // nothing...
                    } else if(tok.type==TK_TEXT) {
                        int state = -1;
                        if(!strcmp(tok.str, "disable"))
                            state = 0;
                        else if(!strcmp(tok.str, "warn"))
                            state = 1;
                        else if(!strcmp(tok.str, "enable"))
                            state = 1;
                        else if(!strcmp(tok.str, "require"))
                            state = 1;
                        if(state!=-1) {
                            if(exts->size==exts->cap) {
                                exts->cap += 4;
                                exts->ext = (extension_t*)realloc(exts->ext, sizeof(extension_t)*exts->cap);
                            }
                            strcpy(exts->ext[exts->size].name, extname);
                            exts->ext[exts->size].state = state;
                            ++exts->size;
                            status = 398;   // all done
                        } else
                            status = 399; // error, unknown keyword
                    } else {
                        status = 399; // fallback, syntax error...
                    }
                    break;
                    // #pragma
                case 510:
                    if(tok.type==TK_SPACE) {
                        // nothing...
                    } else if(tok.type==TK_TEXT && strlen(tok.str)<50) {
                        if(strcmp(tok.str, "message")==0)
                            status = 398; //pragma message are removed
                        else if(strcmp(tok.str, "parameter")==0)
                            status = 398; //pragma message are removed
                        else
                            status = 399;   // other pragma as left as-is
                    } else {
                        status = 399; // fallback, syntax error...
                    }
                    break;
                    // #define
                case 610:
                    if(tok.type==TK_SPACE) {
                        // nothing...
                    } else if(tok.type==TK_TEXT && strlen(tok.str)<50) {
                        defname = strdup(tok.str);
                        k = kh_put(alldefine, alldefines, defname, &ret);
                        kh_value(alldefines, k) = defname;
                        status = 620; // and now get the value
                    } else {
                        status = 399; // fallback, define name too long...
                    }
                    break;
                case 620:
                    if(tok.type==TK_SPACE) {
                        // nothing...
                    } else if(tok.type==TK_INT) {
                        defval = tok.integer;
                        status = 630; // check if end of line (so it's a simple define)
                    } else if(tok.type==TK_NEWLINE) {
                        {
                            int l = p - oldp;
                            memcpy(tok.str, oldp, l);
                            tok.str[l]='\0';
                            oldp = 0;
                        }
                        status = 0;
                    } else {
                        status = 399; // fallback
                    }
                    break;
                case 630:
                    if(tok.type==TK_SPACE) {
                        // nothing...
                    } else if(tok.type==TK_NEWLINE) {
                        k = kh_put(define, defines, defname, &ret);
                        kh_value(defines, k) = defval;
                        {
                            int l = p - oldp;
                            memcpy(tok.str, oldp, l);
                            tok.str[l]='\0';
                            oldp = 0;
                        }
                        status = 0; // ok, define added to collection, left the line as-is anyway
                    } else {
                        status = 399; // fallback
                    }
                    break;
                    // #defined
                case 710:
                    if(tok.type==TK_SPACE) {
                        // nothing...
                    } else if(tok.type==TK_OPENBRACE) {
                        status = 720; // and now get the value
                        ++indefined;
                    } else if(tok.type==TK_NEWLINE) {
                        if(!indefined) status = 0; // ok... no handling of #defined for now, so just write through the line
                    }/* else {
                        status = 399; // No fallback...
                    }*/
                    break;
                case 720:
                    if(tok.type==TK_SPACE || tok.type==TK_TEXT) {
                        // nothing...
                    } else if (tok.type==TK_CLOSEBRACE) {
                        --indefined;
                        status = 710;
                    } else {
                        indefined = 0;
                        status = 399;
                    }
                    break;
                    // #version
                case 810:
                    if(tok.type==TK_SPACE) {
                        // nothing...
                    } else if(tok.type==TK_TEXT) {
                        strncat(*versionString, tok.str, 50);
                        status = 820;
                    } else if(tok.type==TK_INT) {
                        char buff[20] = {0};
                        sprintf(buff, "%d", tok.integer);
                        strncat(*versionString, buff, 50);
                        status = 820;
                    } else if(tok.type==TK_FLOAT) {
                        char buff[20] = {0};
                        sprintf(buff, "%g", tok.real);
                        strncat(*versionString, buff, 50);
                        status = 820;
                    } else {
                        status = 399; // fallback, syntax error...
                    }
                    break;
                case 820:
                    if(tok.type==TK_SPACE) {
                        strncat(*versionString, " ", 50);
                        status = 830;
                    } else if(tok.type==TK_TEXT) {
                        strncat(*versionString, tok.str, 50);
                    } else if(tok.type==TK_INT) {
                        char buff[20] = {0};
                        sprintf(buff, "%d", tok.integer);
                        strncat(*versionString, buff, 50);
                    } else if(tok.type==TK_FLOAT) {
                        char buff[20] = {0};
                        sprintf(buff, "%g", tok.real);
                        strncat(*versionString, buff, 50);
                    } else if(tok.type==TK_NEWLINE) {
                        oldp = NULL;
                        status = 0;
                    } else {
                        status = 399; // fallback, syntax error...
                    }
                    break;
                case 830:
                    if(tok.type==TK_SPACE) {
                        status = 830;
                    } else if(tok.type==TK_TEXT) {
                        strncat(*versionString, tok.str, 50);
                        status = 820;
                    } else if(tok.type==TK_INT) {
                        char buff[20] = {0};
                        sprintf(buff, "%d", tok.integer);
                        strncat(*versionString, buff, 50);
                        status = 820;
                    } else if(tok.type==TK_FLOAT) {
                        char buff[20] = {0};
                        sprintf(buff, "%g", tok.real);
                        strncat(*versionString, buff, 50);
                        status = 820;
                    } else if(tok.type==TK_NEWLINE) {
                        oldp = NULL;
                        status = 0;
                    } else {
                        status = 399; // fallback, syntax error...
                    }
                    break;
            }
            if(notok)
                notok=0;
            else
            if(write && !oldp && nowrite_ifs!=1) {
                if(!incomment && !indefined && tok.type == TK_TEXT) {
                    k = kh_get(define, defines, tok.str);
                    if(k!=kh_end(defines)) {
                        int v = kh_val(defines, k);
                        sprintf(tok.str, "%d", v); // overide define with defined value
                    }
                }
                int l = strlen(tok.str);
                if(sz+l>=cap) {
                    cap+=2000;
                    ncode = (char*)realloc(ncode, cap);
                }
                strcat(ncode, tok.str);
                sz+=l;
            }
        }
    }

    LOG_D("New code is: ------------\n%s\n------------------\n", ncode)
    kh_destroy(define, defines);
    kh_foreach_value(alldefines, defname,
                     free(defname);
    )
    kh_destroy(alldefine, alldefines);
    if(stackif.ifs)
        free(stackif.ifs);

    setlocale(LC_ALL, old_locale);
    return ncode;
}

typedef struct {
    const char* glname;
    const char* name;
    const char* type;
    const char* prec;
    int attrib;
} builtin_attrib_t;

const builtin_attrib_t builtin_attrib[] = {
        {"gl_Vertex", "_gl4es_Vertex", "vec4", "highp", ARB_VERTEX},
        {"gl_Color", "_gl4es_Color", "vec4", "lowp", ARB_COLOR},
        {"gl_MultiTexCoord0", "_gl4es_MultiTexCoord0", "vec4", "highp", ARB_MULTITEXCOORD0},
        {"gl_MultiTexCoord1", "_gl4es_MultiTexCoord1", "vec4", "highp", ARB_MULTITEXCOORD1},
        {"gl_MultiTexCoord2", "_gl4es_MultiTexCoord2", "vec4", "highp", ARB_MULTITEXCOORD2},
        {"gl_MultiTexCoord3", "_gl4es_MultiTexCoord3", "vec4", "highp", ARB_MULTITEXCOORD3},
        {"gl_MultiTexCoord4", "_gl4es_MultiTexCoord4", "vec4", "highp", ARB_MULTITEXCOORD4},
        {"gl_MultiTexCoord5", "_gl4es_MultiTexCoord5", "vec4", "highp", ARB_MULTITEXCOORD5},
        {"gl_MultiTexCoord6", "_gl4es_MultiTexCoord6", "vec4", "highp", ARB_MULTITEXCOORD6},
        {"gl_MultiTexCoord7", "_gl4es_MultiTexCoord7", "vec4", "highp", ARB_MULTITEXCOORD7},
        {"gl_MultiTexCoord8", "_gl4es_MultiTexCoord8", "vec4", "highp", ARB_MULTITEXCOORD8},
        {"gl_MultiTexCoord9", "_gl4es_MultiTexCoord9", "vec4", "highp", ARB_MULTITEXCOORD9},
        {"gl_MultiTexCoord10", "_gl4es_MultiTexCoord10", "vec4", "highp", ARB_MULTITEXCOORD10},
        {"gl_MultiTexCoord11", "_gl4es_MultiTexCoord11", "vec4", "highp", ARB_MULTITEXCOORD11},
        {"gl_MultiTexCoord12", "_gl4es_MultiTexCoord12", "vec4", "highp", ARB_MULTITEXCOORD12},
        {"gl_MultiTexCoord13", "_gl4es_MultiTexCoord13", "vec4", "highp", ARB_MULTITEXCOORD13},
        {"gl_MultiTexCoord14", "_gl4es_MultiTexCoord14", "vec4", "highp", ARB_MULTITEXCOORD14},
        {"gl_MultiTexCoord15", "_gl4es_MultiTexCoord15", "vec4", "highp", ARB_MULTITEXCOORD15},
        {"gl_SecondaryColor", "_gl4es_SecondaryColor", "vec4", "lowp", ARB_SECONDARY},
        {"gl_Normal", "_gl4es_Normal", "vec3", "highp", ARB_NORMAL},
        {"gl_FogCoord", "_gl4es_FogCoord", "float", "highp", ARB_FOGCOORD}
};

const builtin_attrib_t builtin_attrib_compressed[] = {
        {"gl_Vertex", "_gl4es_Vertex", "vec4", "highp", COMP_VERTEX},
        {"gl_Color", "_gl4es_Color", "vec4", "lowp", COMP_COLOR},
        {"gl_MultiTexCoord0", "_gl4es_MultiTexCoord0", "vec4", "highp", COMP_MULTITEXCOORD0},
        {"gl_MultiTexCoord1", "_gl4es_MultiTexCoord1", "vec4", "highp", COMP_MULTITEXCOORD1},
        {"gl_MultiTexCoord2", "_gl4es_MultiTexCoord2", "vec4", "highp", COMP_MULTITEXCOORD2},
        {"gl_MultiTexCoord3", "_gl4es_MultiTexCoord3", "vec4", "highp", COMP_MULTITEXCOORD3},
        {"gl_MultiTexCoord4", "_gl4es_MultiTexCoord4", "vec4", "highp", COMP_MULTITEXCOORD4},
        {"gl_MultiTexCoord5", "_gl4es_MultiTexCoord5", "vec4", "highp", COMP_MULTITEXCOORD5},
        {"gl_MultiTexCoord6", "_gl4es_MultiTexCoord6", "vec4", "highp", COMP_MULTITEXCOORD6},
        {"gl_MultiTexCoord7", "_gl4es_MultiTexCoord7", "vec4", "highp", COMP_MULTITEXCOORD7},
        {"gl_MultiTexCoord8", "_gl4es_MultiTexCoord8", "vec4", "highp", COMP_MULTITEXCOORD8},
        {"gl_MultiTexCoord9", "_gl4es_MultiTexCoord9", "vec4", "highp", COMP_MULTITEXCOORD9},
        {"gl_MultiTexCoord10", "_gl4es_MultiTexCoord10", "vec4", "highp", COMP_MULTITEXCOORD10},
        {"gl_MultiTexCoord11", "_gl4es_MultiTexCoord11", "vec4", "highp", COMP_MULTITEXCOORD11},
        {"gl_MultiTexCoord12", "_gl4es_MultiTexCoord12", "vec4", "highp", COMP_MULTITEXCOORD12},
        {"gl_MultiTexCoord13", "_gl4es_MultiTexCoord13", "vec4", "highp", COMP_MULTITEXCOORD13},
        {"gl_MultiTexCoord14", "_gl4es_MultiTexCoord14", "vec4", "highp", COMP_MULTITEXCOORD14},
        {"gl_MultiTexCoord15", "_gl4es_MultiTexCoord15", "vec4", "highp", COMP_MULTITEXCOORD15},
        {"gl_SecondaryColor", "_gl4es_SecondaryColor", "vec4", "lowp", COMP_SECONDARY},
        {"gl_Normal", "_gl4es_Normal", "vec3", "highp", COMP_NORMAL},
        {"gl_FogCoord", "_gl4es_FogCoord", "float", "highp", COMP_FOGCOORD}
};

typedef struct {
    const char* glname;
    const char* name;
    const char* type;
    int   texarray;
    reserved_matrix_t matrix;
} builtin_matrix_t;

const builtin_matrix_t builtin_matrix[] = {
        {"gl_ModelViewMatrixInverseTranspose", "_gl4es_ITModelViewMatrix", "mat4", 0, MAT_MV_IT},
        {"gl_ModelViewMatrixInverse", "_gl4es_IModelViewMatrix", "mat4", 0, MAT_MV_I},
        {"gl_ModelViewMatrixTranspose", "_gl4es_TModelViewMatrix", "mat4", 0, MAT_MV_T},
        {"gl_ModelViewMatrix", "_gl4es_ModelViewMatrix", "mat4", 0, MAT_MV},
        {"gl_ProjectionMatrixInverseTranspose", "_gl4es_ITProjectionMatrix", "mat4", 0, MAT_P_IT},
        {"gl_ProjectionMatrixInverse", "_gl4es_IProjectionMatrix", "mat4", 0, MAT_P_I},
        {"gl_ProjectionMatrixTranspose", "_gl4es_TProjectionMatrix", "mat4", 0, MAT_P_T},
        {"gl_ProjectionMatrix", "_gl4es_ProjectionMatrix", "mat4", 0, MAT_P},
        {"gl_ModelViewProjectionMatrixInverseTranspose", "_gl4es_ITModelViewProjectionMatrix", "mat4", 0, MAT_MVP_IT},
        {"gl_ModelViewProjectionMatrixInverse", "_gl4es_IModelViewProjectionMatrix", "mat4", 0, MAT_MVP_I},
        {"gl_ModelViewProjectionMatrixTranspose", "_gl4es_TModelViewProjectionMatrix", "mat4", 0, MAT_MVP_T},
        {"gl_ModelViewProjectionMatrix", "_gl4es_ModelViewProjectionMatrix", "mat4", 0, MAT_MVP},
        // non standard version to avoid useless array of Matrix Uniform (in case the compiler as issue optimising this)
        {"gl_TextureMatrix_0", "_gl4es_TextureMatrix_0", "mat4", 0, MAT_T0},
        {"gl_TextureMatrix_1", "_gl4es_TextureMatrix_1", "mat4", 0, MAT_T1},
        {"gl_TextureMatrix_2", "_gl4es_TextureMatrix_2", "mat4", 0, MAT_T2},
        {"gl_TextureMatrix_3", "_gl4es_TextureMatrix_3", "mat4", 0, MAT_T3},
        {"gl_TextureMatrix_4", "_gl4es_TextureMatrix_4", "mat4", 0, MAT_T4},
        {"gl_TextureMatrix_5", "_gl4es_TextureMatrix_5", "mat4", 0, MAT_T5},
        {"gl_TextureMatrix_6", "_gl4es_TextureMatrix_6", "mat4", 0, MAT_T6},
        {"gl_TextureMatrix_7", "_gl4es_TextureMatrix_7", "mat4", 0, MAT_T7},
        {"gl_TextureMatrix_8", "_gl4es_TextureMatrix_8", "mat4", 0, MAT_T8},
        {"gl_TextureMatrix_9", "_gl4es_TextureMatrix_9", "mat4", 0, MAT_T9},
        {"gl_TextureMatrix_10", "_gl4es_TextureMatrix_10", "mat4", 0, MAT_T10},
        {"gl_TextureMatrix_11", "_gl4es_TextureMatrix_11", "mat4", 0, MAT_T11},
        {"gl_TextureMatrix_12", "_gl4es_TextureMatrix_12", "mat4", 0, MAT_T12},
        {"gl_TextureMatrix_13", "_gl4es_TextureMatrix_13", "mat4", 0, MAT_T13},
        {"gl_TextureMatrix_14", "_gl4es_TextureMatrix_14", "mat4", 0, MAT_T14},
        {"gl_TextureMatrix_15", "_gl4es_TextureMatrix_15", "mat4", 0, MAT_T15},
        // regular texture matrix
        {"gl_TextureMatrixInverseTranspose", "_gl4es_ITTextureMatrix", "mat4", 1, MAT_T0_IT},
        {"gl_TextureMatrixInverse", "_gl4es_ITextureMatrix", "mat4", 1, MAT_T0_I},
        {"gl_TextureMatrixTranspose", "_gl4es_TTextureMatrix", "mat4", 1, MAT_T0_T},
        {"gl_TextureMatrix", "_gl4es_TextureMatrix", "mat4", 1, MAT_T0},
        {"gl_NormalMatrix", "_gl4es_NormalMatrix", "mat3", 0, MAT_N}
};

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
static const char* gl4es_MaxLightsSource =
        "#define _gl4es_MaxLights " STR(MAX_LIGHT) "\n";
static const char* gl4es_MaxClipPlanesSource =
        "#define _gl4es_MaxClipPlanes " STR(MAX_CLIP_PLANES) "\n";
static const char* gl4es_MaxTextureUnitsSource =
        "#define _gl4es_MaxTextureUnits " STR(MAX_TEX) "\n";
static const char* gl4es_MaxTextureCoordsSource =
        "#define _gl4es_MaxTextureCoords " STR(MAX_TEX) "\n";
#undef STR
#undef STR_HELPER

static const char* gl4es_LightSourceParametersSource =
        "struct gl_LightSourceParameters\n"
        "{\n"
        "   vec4 ambient;\n"
        "   vec4 diffuse;\n"
        "   vec4 specular;\n"
        "   vec4 position;\n"
        "   vec4 halfVector;\n"   //halfVector = normalize(normalize(position) + vec3(0,0,1) if vbs==FALSE)
        "   vec3 spotDirection;\n"
        "   float spotExponent;\n"
        "   float spotCutoff;\n"
        "   float spotCosCutoff;\n"
        "   float constantAttenuation;\n"
        "   float linearAttenuation;\n"
        "   float quadraticAttenuation;\n"
        "};\n"
        "uniform gl_LightSourceParameters gl_LightSource[gl_MaxLights];\n";

static const char* gl4es_LightModelParametersSource =
        "struct gl_LightModelParameters {\n"
        "  vec4 ambient;\n"
        "};\n"
        "uniform gl_LightModelParameters gl_LightModel;\n";

static const char* gl4es_MaterialParametersSource =
        "struct gl_MaterialParameters\n"
        "{\n"
        "   vec4 emission;\n"
        "   vec4 ambient;\n"
        "   vec4 diffuse;\n"
        "   vec4 specular;\n"
        "   float shininess;\n"
        "};\n"
        "uniform gl_MaterialParameters gl_FrontMaterial;\n"
        "uniform gl_MaterialParameters gl_BackMaterial;\n";

static const char* gl4es_LightModelProductsSource =
        "struct gl_LightModelProducts\n"
        "{\n"
        "   vec4 sceneColor;\n"
        "};\n"
        "uniform gl_LightModelProducts gl_FrontLightModelProduct;\n"
        "uniform gl_LightModelProducts gl_BackLightModelProduct;\n";

static const char* gl4es_LightProductsSource =
        "struct gl_LightProducts\n"
        "{\n"
        "   vec4 ambient;\n"
        "   vec4 diffuse;\n"
        "   vec4 specular;\n"
        "};\n"
        "uniform gl_LightProducts gl_FrontLightProduct[gl_MaxLights];\n"
        "uniform gl_LightProducts gl_BackLightProduct[gl_MaxLights];\n";

static const char* gl4es_PointSpriteSource =
        "struct gl_PointParameters\n"
        "{\n"
        "   float size;\n"
        "   float sizeMin;\n"
        "   float sizeMax;\n"
        "   float fadeThresholdSize;\n"
        "   float distanceConstantAttenuation;\n"
        "   float distanceLinearAttenuation;\n"
        "   float distanceQuadraticAttenuation;\n"
        "};\n"
        "uniform gl_PointParameters gl_Point;\n";

static const char* gl4es_FogParametersSource =
        "struct gl_FogParameters {\n"
        "    lowp vec4 color;\n"
        "    mediump float density;\n"
        "    mediump float start;\n"
        "    mediump float end;\n"
        "    mediump float scale;\n"   // Derived:   1.0 / (end - start) 
        "};\n"
        "uniform gl_FogParameters gl_Fog;\n";
static const char* gl4es_FogParametersSourceHighp =
        "struct gl_FogParameters {\n"
        "    lowp vec4 color;\n"
        "    mediump float density;\n"
        "    highp   float start;\n"
        "    highp   float end;\n"
        "    highp   float scale;\n"   // Derived:   1.0 / (end - start) 
        "};\n"
        "uniform gl_FogParameters gl_Fog;\n";

static const char* gl4es_texenvcolorSource =
        "uniform vec4 gl_TextureEnvColor[gl_MaxTextureUnits];\n";

static const char* gl4es_texgeneyeSource[4] = {
        "uniform vec4 gl_EyePlaneS[gl_MaxTextureCoords];\n",
        "uniform vec4 gl_EyePlaneT[gl_MaxTextureCoords];\n",
        "uniform vec4 gl_EyePlaneR[gl_MaxTextureCoords];\n",
        "uniform vec4 gl_EyePlaneQ[gl_MaxTextureCoords];\n" };

static const char* gl4es_texgenobjSource[4] = {
        "uniform vec4 gl_ObjectPlaneS[gl_MaxTextureCoords];\n",
        "uniform vec4 gl_ObjectPlaneT[gl_MaxTextureCoords];\n",
        "uniform vec4 gl_ObjectPlaneR[gl_MaxTextureCoords];\n",
        "uniform vec4 gl_ObjectPlaneQ[gl_MaxTextureCoords];\n" };

static const char* gl4es_clipplanesSource =
        "uniform vec4  gl_ClipPlane[gl_MaxClipPlanes];\n";

static const char* gl4es_normalscaleSource =
        "uniform float gl_NormalScale;\n";

static const char* gl4es_instanceID =
        "#define GL_ARB_draw_instanced 1\n"
        "uniform int _gl4es_InstanceID;\n";

static const char* gl4es_frontColorSource =
        "varying lowp vec4 _gl4es_FrontColor;\n";

static const char* gl4es_backColorSource =
        "varying lowp vec4 _gl4es_BackColor;\n";

static const char* gl4es_frontSecondaryColorSource =
        "varying lowp vec4 _gl4es_FrontSecondaryColor;\n";

static const char* gl4es_backSecondaryColorSource =
        "varying lowp vec4 _gl4es_BackSecondaryColor;\n";

static const char* gl4es_texcoordSource =
        "varying mediump vec4 _gl4es_TexCoord[%d];\n";

static const char* gl4es_texcoordSourceAlt =
        "varying mediump vec4 _gl4es_TexCoord_%d;\n";

static const char* gl4es_fogcoordSource =
        "varying mediump float _gl4es_FogFragCoord;\n";

static const char* gl4es_ftransformSource =
        "\n"
        "highp vec4 ftransform() {\n"
        " return gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "}\n";

static const char* gl4es_dummyClipVertex =
        "vec4 dummyClipVertex_%d";

static const char* gl_TexCoordSource = "gl_TexCoord[";

static const char* gl_TexMatrixSources[] = {
        "gl_TextureMatrixInverseTranspose[",
        "gl_TextureMatrixInverse[",
        "gl_TextureMatrixTranspose[",
        "gl_TextureMatrix["
};

static const char* GLESHeader[] = {
        "#version 100\n%sprecision %s float;\nprecision %s int;\n",
        "#version 120\n%sprecision %s float;\nprecision %s int;\n",
        "#version 310 es\n%sprecision %s float;\nprecision %s int;\n",
        "#version 300 es\n%sprecision %s float;\nprecision %s int;\n",
        "#version 320 es\n%sprecision %s float;\nprecision %s int;\n"
};

static const char* gl4es_transpose =
        "mat2 gl4es_transpose(mat2 m) {\n"
        " return mat2(m[0][0], m[0][1],\n"
        "             m[1][0], m[1][1]);\n"
        "}\n"
        "mat3 gl4es_transpose(mat3 m) {\n"
        " return mat3(m[0][0], m[0][1], m[0][2],\n"
        "             m[1][0], m[1][1], m[1][2],\n"
        "             m[2][0], m[2][1], m[2][2]);\n"
        "}\n"
        "mat4 gl4es_transpose(mat4 m) {\n"
        " return mat4(m[0][0], m[0][1], m[0][2], m[0][3],\n"
        "             m[1][0], m[1][1], m[1][2], m[1][3],\n"
        "             m[2][0], m[2][1], m[2][2], m[2][3],\n"
        "             m[3][0], m[3][1], m[3][2], m[3][3]);\n"
        "}\n";

static const char* HackAltPow =
        "float pow(float f, int a) {\n"
        " return pow(f, float(a));\n"
        "}\n";
static const char* HackAltMax =
        "float max(float a, int b) {\n"
        " return max(a, float(b));\n"
        "}\n"
        "float max(int a, float b) {\n"
        " return max(float(a), b);\n"
        "}\n";
static const char* HackAltMin =
        "float min(float a, int b) {\n"
        " return min(a, float(b));\n"
        "}\n"
        "float min(int a, float b) {\n"
        " return min(float(a), b);\n"
        "}\n";
static const char* HackAltClamp =
        "float clamp(float f, int a, int b) {\n"
        " return clamp(f, float(a), float(b));\n"
        "}\n"
        "float clamp(float f, float a, int b) {\n"
        " return clamp(f, a, float(b));\n"
        "}\n"
        "float clamp(float f, int a, float b) {\n"
        " return clamp(f, float(a), b);\n"
        "}\n"
        "vec2 clamp(vec2 f, int a, int b) {\n"
        " return clamp(f, float(a), float(b));\n"
        "}\n"
        "vec2 clamp(vec2 f, float a, int b) {\n"
        " return clamp(f, a, float(b));\n"
        "}\n"
        "vec2 clamp(vec2 f, int a, float b) {\n"
        " return clamp(f, float(a), b);\n"
        "}\n"
        "vec3 clamp(vec3 f, int a, int b) {\n"
        " return clamp(f, float(a), float(b));\n"
        "}\n"
        "vec3 clamp(vec3 f, float a, int b) {\n"
        " return clamp(f, a, float(b));\n"
        "}\n"
        "vec3 clamp(vec3 f, int a, float b) {\n"
        " return clamp(f, float(a), b);\n"
        "}\n"
        "vec4 clamp(vec4 f, int a, int b) {\n"
        " return clamp(f, float(a), float(b));\n"
        "}\n"
        "vec4 clamp(vec4 f, float a, int b) {\n"
        " return clamp(f, a, float(b));\n"
        "}\n"
        "vec4 clamp(vec4 f, int a, float b) {\n"
        " return clamp(f, float(a), b);\n"
        "}\n";


static const char* HackAltMod =
        "float mod(float f, int a) {\n"
        " return mod(f, float(a));\n"
        "}\n"
        "vec2 mod(vec2 f, int a) {\n"
        " return mod(f, float(a));\n"
        "}\n"
        "vec3 mod(vec3 f, int a) {\n"
        " return mod(f, float(a));\n"
        "}\n"
        "vec4 mod(vec4 f, int a) {\n"
        " return mod(f, float(a));\n"
        "}\n";

static const char* texture2DLodAlt =
        "vec4 _gl4es_texture2DLod(sampler2D sampler, vec2 coord, float lod) {\n"
        " return texture2D(sampler, coord);\n"
        "}\n";

static const char* texture2DProjLodAlt =
        "vec4 _gl4es_texture2DProjLod(sampler2D sampler, vec3 coord, float lod) {\n"
        " return texture2DProj(sampler, coord);\n"
        "}\n"
        "vec4 _gl4es_texture2DProjLod(sampler2D sampler, vec4 coord, float lod) {\n"
        " return texture2DProj(sampler, coord);\n"
        "}\n";
static const char* textureCubeLodAlt =
        "vec4 _gl4es_textureCubeLod(samplerCube sampler, vec3 coord, float lod) {\n"
        " return textureCube(sampler, coord);\n"
        "}\n";

static const char* texture2DGradAlt =
        "vec4 _gl4es_texture2DGrad(sampler2D sampler, vec2 coord, vec2 dPdx, vec2 dPdy) {\n"
        " return texture2D(sampler, coord);\n"
        "}\n";

static const char* texture2DProjGradAlt =
        "vec4 _gl4es_texture2DProjGrad(sampler2D sampler, vec3 coord, vec2 dPdx, vec2 dPdy) {\n"
        " return texture2DProj(sampler, coord);\n"
        "}\n"
        "vec4 _gl4es_texture2DProjGrad(sampler2D sampler, vec4 coord, vec2 dPdx, vec2 dPdy) {\n"
        " return texture2DProj(sampler, coord);\n"
        "}\n";
static const char* textureCubeGradAlt =
        "vec4 _gl4es_textureCubeGrad(samplerCube sampler, vec3 coord, vec2 dPdx, vec2 dPdy) {\n"
        " return textureCube(sampler, coord);\n"
        "}\n";


static const char* useEXTDrawBuffers =
        "#extension GL_EXT_draw_buffers : enable\n";

static const char* gl_ProgramEnv  = "gl_ProgramEnv";
static const char* gl_ProgramLocal= "gl_ProgramLocal";

static const char* gl_Samplers1D = "gl_Sampler1D_";
static const char* gl_Samplers2D = "gl_Sampler2D_";
static const char* gl_Samplers3D = "gl_Sampler3D_";
static const char* gl_SamplersCube = "gl_SamplerCube_";
static const char* gl4es_Samplers1D = "_gl4es_Sampler1D_";
static const char* gl4es_Samplers2D = "_gl4es_Sampler2D_";
static const char* gl4es_Samplers3D = "_gl4es_Sampler3D_";
static const char* gl4es_SamplersCube = "_gl4es_SamplerCube_";
static const char* gl4es_Samplers1D_uniform = "uniform sampler2D _gl4es_Sampler1D_";
static const char* gl4es_Samplers2D_uniform = "uniform sampler2D _gl4es_Sampler2D_";
static const char* gl4es_Samplers3D_uniform = "uniform sampler2D _gl4es_Sampler3D_";
static const char* gl4es_SamplersCube_uniform = "uniform samplerCube _gl4es_SamplerCube_";

static const char* gl_VertexAttrib = "gl_VertexAttrib_";
static const char* gl4es_VertexAttrib = "_gl4es_VertexAttrib_";

char gl_VA[MAX_VATTRIB][32] = {0};
char gl4es_VA[MAX_VATTRIB][32] = {0};

int CountString(const char* pBuffer, const char* S)
{
    const char* p = pBuffer;
    int lS = strlen(S);
    int n = 0;
    while((p = strstr(p, S)))
    {
        // found an occurence of S
        // check if good to count, strchr also found '\0' :)
        if(strchr(AllSeparators, p[lS])!=NULL && (p==pBuffer || strchr(AllSeparators, p[-1])!=NULL))
            n++;
        p+=lS;
    }
    return n;
}

char* ResizeIfNeeded(char* pBuffer, int *size, int addsize);

char* InplaceReplace(char* pBuffer, int* size, const char* S, const char* D)
{
    int lS = strlen(S), lD = strlen(D);
    pBuffer = ResizeIfNeeded(pBuffer, size, (lD-lS)*CountString(pBuffer, S));
    char* p = pBuffer;
    while((p = strstr(p, S)))
    {
        // found an occurence of S
        // check if good to replace, strchr also found '\0' :)
        if(strchr(AllSeparators, p[lS])!=NULL && (p==pBuffer || strchr(AllSeparators, p[-1])!=NULL)) {
            // move out rest of string
            memmove(p+lD, p+lS, strlen(p)-lS+1);
            // replace
            memcpy(p, D, strlen(D));
            // next
            p+=lD;
        } else p+=lS;
    }

    return pBuffer;
}

char * InplaceReplaceByIndex(char* pBuffer, int* size, const int startIndex, const int endIndex, const char* replacement)
{
    //SHUT_LOGD("BY INDEX: %s", replacement);
    //SHUT_LOGD("BY INDEX: %i", strlen(replacement));

    int length_difference;
    if(endIndex < startIndex)
        length_difference = strlen(replacement) + (endIndex - startIndex);
    else if(endIndex == startIndex)
        length_difference = strlen(replacement) - 1; // The initial char gets replaced
    else
        length_difference = strlen(replacement) - (endIndex - startIndex); // Can be negative if repl is smaller

    pBuffer = ResizeIfNeeded(pBuffer, size, length_difference);
    //SHUT_LOGD("BEFORE MOVING: \n%s", pBuffer);
    // Move the end of the string
    memmove(pBuffer + startIndex + strlen(replacement) , pBuffer + endIndex + 1, strlen(pBuffer) - endIndex + 1);
    //SHUT_LOGD("AFTER MOVING 1: \n%s", pBuffer);

    // Insert the replacement
    memcpy(pBuffer + startIndex, replacement, strlen(replacement));
    //strncpy(pBuffer + startIndex, replacement, strlen(replacement));
    //SHUT_LOGD("AFTER MOVING 2: \n%s", pBuffer);

    return pBuffer;
}

/**
 * Insert the string at the index, pushing "every chars to the right"
 * @param source The shader as a string
 * @param sourceLength The ALLOCATED length of the shader
 * @param insertPoint The index at which the string is inserted.
 * @param insertedString The string to insert
 * @return The shader as a string, maybe in a different memory location
 */
char * InplaceInsertByIndex(char * source, int *sourceLength, const int insertPoint, const char *insertedString){
    int insertLength = strlen(insertedString);
    source = ResizeIfNeeded(source, sourceLength, insertLength);
    memmove(source + insertPoint + insertLength,  source + insertPoint, strlen(source) - insertPoint + 1);
    memcpy(source + insertPoint, insertedString, insertLength);

    return source;
}

char* InplaceInsert(char* pBuffer, const char* S, char* master, int* size)
{
    char* m = ResizeIfNeeded(master, size, strlen(S));
    if(m!=master) {
        pBuffer += (m-master);
        master = m;
    }
    char* p = pBuffer;
    int lS = strlen(S), ll = strlen(pBuffer);
    memmove(p+lS, p, ll+1);
    memcpy(p, S, lS);

    return master;
}

char* GetLine(char* pBuffer, int num)
{
    char *p = pBuffer;
    while(num-- && (p=strstr(p, "\n"))) p+=strlen("\n");
    return (p)?p:pBuffer;
}

int CountLine(const char* pBuffer)
{
    int n=0;
    const char* p = pBuffer;
    while((p=strstr(p, "\n"))) {
        p+=strlen("\n");
        n++;
    }
    return n;
}

const char* FindString(const char* pBuffer, const char* S)
{
    const char* p = pBuffer;
    int lS = strlen(S);
    while((p = strstr(p, S)))
    {
        // found an occurence of S
        // check if good to count, strchr also found '\0' :)
        if(strchr(AllSeparators, p[lS])!=NULL && (p==pBuffer || strchr(AllSeparators, p[-1])!=NULL))
            return p;
        p+=lS;
    }
    return NULL;
}

int GetLineFor(const char* pBuffer, const char* S)
{
    int n=0;
    const char* p = pBuffer;
    const char* end = FindString(pBuffer, S);
    if(!end)
        return 0;
    while((p=strstr(p, "\n"))) {
        p+=strlen("\n");
        n++;
        if(p>=end)
            return n;
    }
    return n;
}

char* FindStringNC(char* pBuffer, const char* S)
{
    char* p = pBuffer;
    int lS = strlen(S);
    while((p = strstr(p, S)))
    {
        // found an occurence of S
        // check if good to count, strchr also found '\0' :)
        if(strchr(AllSeparators, p[lS])!=NULL && (p==pBuffer || strchr(AllSeparators, p[-1])!=NULL))
            return p;
        p+=lS;
    }
    return NULL;
}

char* ResizeIfNeeded(char* pBuffer, int *size, int addsize) {
    char* p = pBuffer;
    int newsize = strlen(pBuffer)+addsize+1;
    if (newsize>*size) {
        //newsize += 100;
        p = (char*)realloc(pBuffer, newsize);
        *size=newsize;
    }
    return p;
}

char* Append(char* pBuffer, int* size, const char* S) {
    char* p =pBuffer;
    p = ResizeIfNeeded(pBuffer, size, strlen(S));
    strcat(p, S);
    return p;
}

int isBlank(char c)  {
    switch(c) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
        case ':':
        case ',':
        case ';':
        case '/':
            return 1;
        default:
            return 0;
    }
}

int isDigit(char value){
    return (value >= '0' && value <= '9');
}

int isValidFunctionName(char value){
    return ((value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') || (value == '_'));
}

char* StrNext(char *pBuffer, const char* S) {
    if(!pBuffer) return NULL;
    char *p = strstr(pBuffer, S);
    return (p)?p:(p+strlen(S));
}

char* NextStr(char* pBuffer) {
    if(!pBuffer) return NULL;
    while(isBlank(*pBuffer))
        ++pBuffer;
    return pBuffer;
}

char* NextBlank(char* pBuffer) {
    if(!pBuffer) return NULL;
    while(!isBlank(*pBuffer))
        ++pBuffer;
    return pBuffer;
}

char* NextLine(char* pBuffer) {
    if(!pBuffer) return NULL;
    while(*pBuffer && *pBuffer!='\n')
        ++pBuffer;
    return pBuffer;
}

const char* GetNextStr(char* pBuffer) {
    static char buff[100] = {0};
    buff[0] = '\0';
    if(!pBuffer) return NULL;
    char* p1 = NextStr(pBuffer);
    if(!p1) return buff;
    char* p2 = NextBlank(p1);
    if(!p2) return buff;
    int i=0;
    while(p1!=p2 && i<99)
        buff[i++] = *(p1++);
    buff[i] = '\0';
    return buff;
}

int CountStringSimple(char* pBuffer, const char* S)
{
    char* p = pBuffer;
    int lS = strlen(S);
    int n = 0;
    while((p = strstr(p, S)))
    {
        // found an occurence of S
        n++;
        p+=lS;
    }
    return n;
}

char* InplaceReplaceSimple(char* pBuffer, int* size, const char* S, const char* D)
{
    int lS = strlen(S), lD = strlen(D);
    pBuffer = ResizeIfNeeded(pBuffer, size, (lD-lS)*CountStringSimple(pBuffer, S));
    char* p = pBuffer;
    while((p = strstr(p, S)))
    {
        // found an occurence of S
        // move out rest of string
        memmove(p+lD, p+lS, strlen(p)-lS+1);
        // replace
        memcpy(p, D, strlen(D));
        // next
        p+=lD;
    }

    return pBuffer;
}

char* ConvertShader(const char* pEntry, int isVertex, shaderconv_need_t *need, int forwardPort)
{
    if(gl_VA[0][0]=='\0') {
        for (int i=0; i<MAX_VATTRIB; ++i) {
            sprintf(gl_VA[i], "%s%d", gl_VertexAttrib, i);
            sprintf(gl4es_VA[i], "%s%d", gl4es_VertexAttrib, i);
        }
    }
    int fpeShader = 0;
    int maskbefore = 4|(isVertex?1:2);
    int maskafter = 8|(isVertex?1:2);
    
    int comments = 0;

    char* pBuffer = (char*)pEntry;

    int version120 = 0;
    char* versionString = NULL;
    if(!fpeShader) {
        extensions_t exts;  // dummy...
        exts.cap = exts.size = 0;
        exts.ext = NULL;
        // hacks
        char* pHacked = ShaderHacks(pBuffer);
        // preproc first
        pBuffer = preproc(pHacked, comments, 0, &exts, &versionString);
        if(pHacked!=pEntry && pHacked!=pBuffer)
            free(pHacked);
        // now comment all line starting with precision...
        if(strstr(pBuffer, "\nprecision")) {
            int sz = strlen(pBuffer);
            pBuffer = InplaceReplace(pBuffer, &sz, "\nprecision", "\n//precision");
        }
        // should do something with the extension list...
        if(exts.ext)
            free(exts.ext);
    }

    static shaderconv_need_t dummy_need = {0};
    if(!need) {
        need = &dummy_need;
        need->need_texcoord = -1;
        need->need_clean = 1; // no hack, this is a dummy need structure
    }
    int notexarray = need->need_notexarray || fpeShader;

    //const char* GLESUseFragHighp = "#extension GL_OES_fragment_precision_high : enable\n"; // does this one is needed?  
    char GLESFullHeader[512];
    int wanthighp = !fpeShader;
    int versionHeader = 0;
    if(versionString && (strcmp(versionString, "120")==0 || strcmp(versionString, "110")==0 || strstr(versionString, "150") != NULL))
        version120 = forwardPort ? 1 : 0;
    if(version120) {
        /*if(hardext.glsl320es) */versionHeader = 4;
        /*else if(hardext.glsl310es) versionHeader = 2;
        else if(hardext.glsl300es) { versionHeader = 3; }*/ /* location on uniform not supported ! */
        /* else no location or in / out are supported */
    }
    //sprintf(GLESFullHeader, GLESHeader, (wanthighp && 1==1 && !isVertex)?GLESUseFragHighp:"", (wanthighp)?"highp":"mediump", (wanthighp)?"highp":"mediump");
    sprintf(GLESFullHeader, GLESHeader[versionHeader], "", (wanthighp)?"highp":"mediump", (wanthighp)?"highp":"mediump");

    int tmpsize = strlen(pBuffer)*2+strlen(GLESFullHeader)+100;
    char* Tmp = (char*)calloc(1, tmpsize);
    strcpy(Tmp, pBuffer);

    // and now change the version header, and add default precision
    char* newptr;
    newptr=strstr(Tmp, "#version");
    if (!newptr) {
        Tmp = InplaceInsert(Tmp, GLESFullHeader, Tmp, &tmpsize);
    } else {
        while(*newptr!=0x0a) newptr++;
        newptr++;
        memmove(Tmp, newptr, strlen(newptr)+1);
        Tmp = InplaceInsert(Tmp, GLESFullHeader, Tmp, &tmpsize);
    }
    int headline = 3;
    // check if gl_FragDepth is used
    int fragdepth = (strstr(pBuffer, "gl_FragDepth"))?1:0;
    const char* GLESUseFragDepth = "#extension GL_EXT_frag_depth : enable\n";
    const char* GLESFakeFragDepth = "mediump float fakeFragDepth = 0.0;\n";
    if (fragdepth) {
        /* If #extension is used, it should be placed before the second line of the header. */
        if(1)
            Tmp = InplaceInsert(GetLine(Tmp, 1), GLESUseFragDepth, Tmp, &tmpsize);
        else
            Tmp = InplaceInsert(GetLine(Tmp, headline-1), GLESFakeFragDepth, Tmp, &tmpsize);
        headline++;
    }
    // check if the shader uses 3D textures
    int threed_texture = (strstr(pBuffer, "sampler3D"))?1:0;
    const char* GLESUse3DTexture = "#extension GL_OES_texture_3D : enable\n";
    if(threed_texture) {
        Tmp = InplaceInsert(GetLine(Tmp, 1), GLESUse3DTexture, Tmp, &tmpsize); // no fallback possible if device does not support 3D textures
    }

    int derivatives = (strstr(pBuffer, "dFdx(") || strstr(pBuffer, "dFdy(") || strstr(pBuffer, "fwidth("))?1:0;
    const char* GLESUseDerivative = "#extension GL_OES_standard_derivatives : enable\n";
    // complete fake value... A better thing should be use....
    const char* GLESFakeDerivative = "float dFdx(float p) {return 0.0001;}\nvec2 dFdx(vec2 p) {return vec2(0.0001);}\nvec3 dFdx(vec3 p) {return vec3(0.0001);}\n"
                                     "float dFdy(float p) {return 0.0001;}\nvec2 dFdy(vec2 p) {return vec2(0.0001);}\nvec3 dFdy(vec3 p) {return vec3(0.0001);}\n"
                                     "float fwidth(float p) {return abs(dFdx(p))+abs(dFdy(p));}\nvec2 fwidth(vec2 p) {return abs(dFdx(p))+abs(dFdy(p));}\n"
                                     "vec3 fwidth(vec3 p) {return abs(dFdx(p))+abs(dFdy(p));}\n";
    if (derivatives) {
        /* If #extension is used, it should be placed before the second line of the header. */
        if(1)
            Tmp = InplaceInsert(GetLine(Tmp, 1), GLESUseDerivative, Tmp, &tmpsize);
        else
            Tmp = InplaceInsert(GetLine(Tmp, headline-1), GLESFakeDerivative, Tmp, &tmpsize);
        headline++;
    }
    // check if draw_buffers may be used (no fallback here :( )
    if(32>1 && strstr(pBuffer, "gl_FragData[")) {
        Tmp = InplaceInsert(GetLine(Tmp, 1), useEXTDrawBuffers, Tmp, &tmpsize);
    }
    // if some functions are used, add some int/float alternative
    if(!fpeShader && !1) {
        if(strstr(Tmp, "pow(") || strstr(Tmp, "pow (")) {
            Tmp = InplaceInsert(GetLine(Tmp, headline), HackAltPow, Tmp, &tmpsize);
        }
        if(strstr(Tmp, "max(") || strstr(Tmp, "max (")) {
            Tmp = InplaceInsert(GetLine(Tmp, headline), HackAltMax, Tmp, &tmpsize);
        }
        if(strstr(Tmp, "min(") || strstr(Tmp, "min (")) {
            Tmp = InplaceInsert(GetLine(Tmp, headline), HackAltMin, Tmp, &tmpsize);
        }
        if(strstr(Tmp, "clamp(") || strstr(Tmp, "clamp (")) {
            Tmp = InplaceInsert(GetLine(Tmp, headline), HackAltClamp, Tmp, &tmpsize);
        }
        if(strstr(Tmp, "mod(") || strstr(Tmp, "mod (")) {
            Tmp = InplaceInsert(GetLine(Tmp, headline), HackAltMod, Tmp, &tmpsize);
        }
    }
    if(!isVertex && 1 &&
       (FindString(Tmp, "texture2DLod") || FindString(Tmp, "texture2DProjLod")
        || FindString(Tmp, "textureCubeLod")
        || FindString(Tmp, "texture2DGradARB") || FindString(Tmp, "texture2DProjGradARB")|| FindString(Tmp, "textureCubeGradARB")
       )) {
        const char* GLESUseShaderLod = "#extension GL_EXT_shader_texture_lod : enable\n";
        Tmp = InplaceInsert(GetLine(Tmp, 1), GLESUseShaderLod, Tmp, &tmpsize);
    }
    if(!isVertex && (FindString(Tmp, "texture2DLod"))) {
        if(1) {
            Tmp = InplaceReplace(Tmp, &tmpsize, "texture2DLod", "texture2DLodEXT");
        } else {
            Tmp = InplaceReplace(Tmp, &tmpsize, "texture2DLod", "_gl4es_texture2DLod");
            Tmp = InplaceInsert(GetLine(Tmp, headline), texture2DLodAlt, Tmp, &tmpsize);
        }
    }
    if(!isVertex && (FindString(Tmp, "texture2DProjLod"))) {
        if(1) {
            Tmp = InplaceReplace(Tmp, &tmpsize, "texture2DProjLod", "texture2DProjLodEXT");
        } else {
            Tmp = InplaceReplace(Tmp, &tmpsize, "texture2DProjLod", "_gl4es_texture2DProjLod");
            Tmp = InplaceInsert(GetLine(Tmp, headline), texture2DProjLodAlt, Tmp, &tmpsize);
        }
    }
    if(!isVertex && (FindString(Tmp, "textureCubeLod"))) {
        if(1) {
            if(!0)
                Tmp = InplaceReplace(Tmp, &tmpsize, "textureCubeLod", "textureCubeLodEXT");
        } else {
            Tmp = InplaceReplace(Tmp, &tmpsize, "textureCubeLod", "_gl4es_textureCubeLod");
            Tmp = InplaceInsert(GetLine(Tmp, headline), textureCubeLodAlt, Tmp, &tmpsize);
        }
    }
    if(!isVertex && (FindString(Tmp, "texture2DGradARB"))) {
        if(1) {
            Tmp = InplaceReplace(Tmp, &tmpsize, "texture2DGradARB", "texture2DGradEXT");
        } else {
            Tmp = InplaceReplace(Tmp, &tmpsize, "texture2DGradARB", "_gl4es_texture2DGrad");
            Tmp = InplaceInsert(GetLine(Tmp, headline), texture2DGradAlt, Tmp, &tmpsize);
        }
    }
    if(!isVertex && (FindString(Tmp, "texture2DProjGradARB"))) {
        if(1) {
            Tmp = InplaceReplace(Tmp, &tmpsize, "texture2DProjGradARB", "texture2DProjGradEXT");
        } else {
            Tmp = InplaceReplace(Tmp, &tmpsize, "texture2DProjGradARB", "_gl4es_texture2DProjGrad");
            Tmp = InplaceInsert(GetLine(Tmp, headline), texture2DProjGradAlt, Tmp, &tmpsize);
        }
    }
    if(!isVertex && (FindString(Tmp, "textureCubeGradARB"))) {
        if(1) {
            if(!0)
                Tmp = InplaceReplace(Tmp, &tmpsize, "textureCubeGradARB", "textureCubeGradEXT");
        } else {
            Tmp = InplaceReplace(Tmp, &tmpsize, "textureCubeGradARB", "_gl4es_textureCubeGrad");
            Tmp = InplaceInsert(GetLine(Tmp, headline), textureCubeGradAlt, Tmp, &tmpsize);
        }
    }
    // now check to remove trailling "f" after float, as it's not supported too
    newptr = Tmp;
    // simple state machine...
    int state = 0;
    while (*newptr!=0x00) {
        switch(state) {
            case 0:
                if ((*newptr >= '0') && (*newptr <= '9'))
                    state = 1;  // integer part
                else if (*newptr == '.')
                    state = 2;  // fractional part
                else if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
                    state = 0; // separator
                else
                    state = 3; // something else
                break;
            case 1: // integer part
                if ((*newptr >= '0') && (*newptr <= '9'))
                    state = 1;  // integer part
                else if (*newptr == '.')
                    state = 2;  // fractional part
                else if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
                    state = 0; // separator
                else  if (*newptr == 'f' ) {
                    // remove that f
                    memmove(newptr, newptr+1, strlen(newptr+1)+1);
                    newptr--;
                } else
                    state = 3;
                break;
            case 2: // fractionnal part
                if ((*newptr >= '0') && (*newptr <= '9'))
                    state = 2;
                else if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
                    state = 0; // separator
                else  if (*newptr == 'f' ) {
                    // remove that f
                    memmove(newptr, newptr+1, strlen(newptr+1)+1);
                    newptr--;
                } else
                    state = 3;
                break;
            case 3:
                if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
                    state = 0; // separator
                else
                    state = 3;
                break;
        }
        newptr++;
    }
    Tmp = InplaceReplace(Tmp, &tmpsize, "gl_FragDepth", (1)?"gl_FragDepthEXT":"fakeFragDepth");
    // builtin attribs
    if(isVertex) {
        // ANGLE already has ftransform, so skip it
#ifndef __APPLE__
        // check for ftransform function
        if(strstr(Tmp, "ftransform(")) {
            Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_ftransformSource, Tmp, &tmpsize);
            // don't increment headline count, as all variying and attributes should be created before
        }
#endif
        // check for builtin OpenGL attributes...
        int n = sizeof(builtin_attrib)/sizeof(builtin_attrib_t);
        for (int i=0; i<n; i++) {
            if(strstr(Tmp, builtin_attrib[i].glname)) {
                // ok, this attribute is used
                // replace gl_name by _gl4es_ one
                Tmp = InplaceReplace(Tmp, &tmpsize, builtin_attrib[i].glname, builtin_attrib[i].name);
                // insert a declaration of it
                char def[100];
                sprintf(def, "attribute %s %s %s;\n", builtin_attrib[i].prec, builtin_attrib[i].type, builtin_attrib[i].name);
                Tmp = InplaceInsert(GetLine(Tmp, headline++), def, Tmp, &tmpsize);
            }
        }
        if(strstr(Tmp, gl_VertexAttrib)) {
            // Generic VA from Old Programs
            for (int i=0; i<MAX_VATTRIB; ++i) {
                char A[100];
                if(FindString(Tmp, gl_VA[i])) {
                    sprintf(A, "attribute highp vec4 %s%d;\n", gl4es_VertexAttrib, i);
                    Tmp = InplaceReplace(Tmp, &tmpsize, gl_VA[i], gl4es_VA[i]);
                    Tmp = InplaceInsert(GetLine(Tmp, headline++), A, Tmp, &tmpsize);
                }
            }
        }
    }
    // builtin varying
    int nvarying = 0;
    if(strstr(Tmp, "gl_Color") || need->need_color) {
        if(need->need_color<1) need->need_color = 1;
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_Color", (need->need_color==1)?"gl_FrontColor":"(gl_FrontFacing?gl_FrontColor:gl_BackColor)");
    }
    if(strstr(Tmp, "gl_FrontColor") || need->need_color) {
        if(need->need_color<1) need->need_color = 1;
        nvarying+=1;
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_frontColorSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_frontColorSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_FrontColor", "_gl4es_FrontColor");
    }
    if(strstr(Tmp, "gl_BackColor") || (need->need_color==2)) {
        need->need_color = 2;
        nvarying+=1;
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_backColorSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_backColorSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_BackColor", "_gl4es_BackColor");
    }
    if(strstr(Tmp, "gl_SecondaryColor") || need->need_secondary) {
        if(need->need_secondary<1) need->need_secondary = 1;
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_SecondaryColor", (need->need_secondary==1)?"gl_FrontSecondaryColor":"(gl_FrontFacing?gl_FrontSecondaryColor:gl_BackSecondaryColor)");
    }
    if(strstr(Tmp, "gl_FrontSecondaryColor") || need->need_secondary) {
        if(need->need_secondary<1) need->need_secondary = 1;
        nvarying+=1;
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_frontSecondaryColorSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_frontSecondaryColorSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_FrontSecondaryColor", "_gl4es_FrontSecondaryColor");
    }
    if(strstr(Tmp, "gl_BackSecondaryColor") || (need->need_secondary==2)) {
        need->need_secondary = 2;
        nvarying+=1;
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_backSecondaryColorSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_backSecondaryColorSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_BackSecondaryColor", "_gl4es_BackSecondaryColor");
    }
    if(strstr(Tmp, "gl_FogFragCoord") || need->need_fogcoord) {
        need->need_fogcoord = 1;
        nvarying+=1;
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_fogcoordSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_fogcoordSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_FogFragCoord", "_gl4es_FogFragCoord");
    }
    // Get the max_texunit and the calc notexarray
    if(strstr(Tmp, "gl_TexCoord") || need->need_texcoord!=-1) {
        int ntex = need->need_texcoord;
        // Try to determine max gl_TexCoord used
        char* p = Tmp;
        int notexarray_ok = 1;
        while((p=strstr(p, gl_TexCoordSource))) {
            p+=strlen(gl_TexCoordSource);
            if(*p>='0' && *p<='9') {
                int n = (*p) - '0';
                if(p[1]>='0' && p[1]<='9')
                    n = n*10 + (p[1] - '0');
                if (ntex<n) ntex = n;
            } else
                notexarray_ok=0;
        }
        // if failed to determine, take max...
        if (ntex==-1) ntex = 32;
        // check constraint, and switch to notexarray if needed
        if (!notexarray && ntex+nvarying>24 && !need->need_clean && notexarray_ok) {
            notexarray = 1;
            need->need_notexarray = 1;
        }
        // prefer notexarray...
        if(!isVertex && notexarray_ok && !need->need_clean) {
            notexarray = 1;
            need->need_notexarray = 1;
        }
        // check constaints
        if (!notexarray && ntex+nvarying>24) ntex = 24 - nvarying;
        need->need_texcoord = ntex;
        char d[100];
        if(notexarray) {
            for (int k=0; k<ntex+1; k++) {
                char d2[100];
                sprintf(d2, "gl_TexCoord[%d]", k);
                if(strstr(Tmp, d2)) {
                    sprintf(d, gl4es_texcoordSourceAlt, k);
                    Tmp = InplaceInsert(GetLine(Tmp, headline), d, Tmp, &tmpsize);
                    headline+=CountLine(d);
                    sprintf(d, "_gl4es_TexCoord_%d", k);
                    Tmp = InplaceReplace(Tmp, &tmpsize, d2, d);
                }
                // check if texture is there
                sprintf(d2, "_gl4es_TexCoord_%d", k);
                if(strstr(Tmp, d2))
                    need->need_texs |= (1<<k);
            }
        } else {
            sprintf(d, gl4es_texcoordSource, ntex+1);
            Tmp = InplaceInsert(GetLine(Tmp, headline), d, Tmp, &tmpsize);
            headline+=CountLine(d);
            Tmp = InplaceReplace(Tmp, &tmpsize, "gl_TexCoord", "_gl4es_TexCoord");
            // set textures as all ntex used
            for (int k=0; k<ntex+1; k++)
                need->need_texs |= (1<<k);
        }
    }

    // builtin matrices work
    {
        if(strstr(Tmp, "transpose(") || strstr(Tmp, "transpose ") || strstr(Tmp, "transpose\t")) {
            Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_transpose, Tmp, &tmpsize);
            InplaceReplace(Tmp, &tmpsize, "transpose", "gl4es_transpose");
            // don't increment headline count, as all variying and attributes should be created before
        }
        // check for builtin matrix uniform...
        {
            // first check number of texture matrices used
            int ntex = -1;
            // Try to determine max Texture matrice used, for each transposed inverse or regular...
            for(int i=0; i<4; ++i) {
                char* p = Tmp;
                while((p=strstr(p, gl_TexMatrixSources[i]))) {
                    p+=strlen(gl_TexMatrixSources[i]);
                    if(*p>='0' && *p<='9') {
                        int n = 0;
                        while(*p>='0' && *p<='9')
                            n = n*10 + (*(p++) - '0');

                        if (ntex<n) ntex = n;
                    }
                }
            }

            // if failed to determine, take max...
            if (ntex==-1) ntex = need->need_texcoord; else ++ntex;
            // change gl_TextureMatrix[X] to gl_TextureMatrix_X if possible
            int change_textmat = notexarray;
            if(!change_textmat) {
                change_textmat = 1;
                char* p = Tmp;
                while(change_textmat && (p=strstr(p, "gl_TextureMatrix["))) {
                    p += strlen("gl_TextureMatrix[");
                    while((*p)>='0' && (*p)<='9') ++p;
                    if((*p)!=']')
                        change_textmat = 0;
                }
            }
            if(change_textmat) {
                for (int k=0; k<ntex+1; k++) {
                    char d[100];
                    char d2[100];
                    sprintf(d2, "gl_TextureMatrix[%d]", k);
                    if(strstr(Tmp, d2)) {
                        sprintf(d, "gl_TextureMatrix_%d", k);
                        Tmp = InplaceReplace(Tmp, &tmpsize, d2, d);
                    }
                }
            }

            int n = sizeof(builtin_matrix)/sizeof(builtin_matrix_t);
            for (int i=0; i<n; i++) {
                if(strstr(Tmp, builtin_matrix[i].glname)) {
                    // ok, this matrix is used
                    // replace gl_name by _gl4es_ one
                    Tmp = InplaceReplace(Tmp, &tmpsize, builtin_matrix[i].glname, builtin_matrix[i].name);
                    // insert a declaration of it
                    char def[100];
                    int ishighp = (isVertex || 1)?1:0;
                    if(builtin_matrix[i].matrix == MAT_N) {
                        if(need->need_normalmatrix && !1)
                            ishighp = 0;
                        if(!1 && !isVertex)
                            need->need_normalmatrix = 1;
                    }
                    if(builtin_matrix[i].matrix == MAT_MV) {
                        if(need->need_mvmatrix && !1)
                            ishighp = 0;
                        if(!1 && !isVertex)
                            need->need_mvmatrix = 1;
                    }
                    if(builtin_matrix[i].matrix == MAT_MVP) {
                        if(need->need_mvpmatrix && !1)
                            ishighp = 0;
                        if(!1 && !isVertex)
                            need->need_mvpmatrix = 1;
                    }
                    if(builtin_matrix[i].texarray)
                        sprintf(def, "uniform %s%s %s[%d];\n", (ishighp)?"highp ":"mediump ", builtin_matrix[i].type, builtin_matrix[i].name, ntex);
                    else
                        sprintf(def, "uniform %s%s %s;\n", (ishighp)?"highp ":"mediump ", builtin_matrix[i].type, builtin_matrix[i].name);
                    Tmp = InplaceInsert(GetLine(Tmp, headline++), def, Tmp, &tmpsize);
                }
            }
        }
    }
    // Handling of gl_LightSource[x].halfVector => normalize(gl_LightSource[x].position - gl_Vertex), but what if in the FragShader ?
/*  if(strstr(Tmp, "halfVector"))
  {
    char *p = Tmp;
    while((p=strstr(p, "gl_LightSource["))) {
      char *p2 = strchr(p, ']');
      if (p2 && !strncmp(p2, "].halfVector", strlen("].halfVector"))) {
        // found an occurence, lets change
        char p3[500];
        strncpy(p3,p, (p2-p)+1); p3[(p2-p)+1]='\0';
        char p4[500], p5[500];
        sprintf(p4, "%s.halfVector", p3);
        sprintf(p5, "normalize(normalize(%s.position.xyz) + vec3(0., 0., 1.))", p3);
        Tmp = InplaceReplace(Tmp, &tmpsize, p4, p5);
        p = Tmp;
      } else
        ++p;
    }
  }*/
    // cleaning up the "centroid" keyword...
    if(strstr(Tmp, "centroid"))
    {
        char *p = Tmp;
        while((p=strstr(p, "centroid"))!=NULL)
        {
            if(p[8]==' ' || p[8]=='\t') { // what next...
                const char* p2 = GetNextStr(p+8);
                if(strcmp(p2, "uniform")==0 || strcmp(p2, "varying")==0) {
                    memset(p, ' ', 8);  // erase the keyword...
                }
            }
            p+=8;
        }
    }

    // check for builtin OpenGL gl_LightSource & friends
    if(strstr(Tmp, "gl_LightSourceParameters") || strstr(Tmp, "gl_LightSource"))
    {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_LightSourceParametersSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_LightSourceParametersSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_LightSourceParameters", "_gl4es_LightSourceParameters");
    }
    if(strstr(Tmp, "gl_LightModelParameters") || strstr(Tmp, "gl_LightModel"))
    {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_LightModelParametersSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_LightModelParametersSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_LightModelParameters", "_gl4es_LightModelParameters");
    }
    if(strstr(Tmp, "gl_LightModelProducts") || strstr(Tmp, "gl_FrontLightModelProduct") || strstr(Tmp, "gl_BackLightModelProduct"))
    {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_LightModelProductsSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_LightModelProductsSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_LightModelProducts", "_gl4es_LightModelProducts");
    }
    if(strstr(Tmp, "gl_LightProducts") || strstr(Tmp, "gl_FrontLightProduct") || strstr(Tmp, "gl_BackLightProduct"))
    {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_LightProductsSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_LightProductsSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_LightProducts", "_gl4es_LightProducts");
    }
    if(strstr(Tmp, "gl_MaterialParameters ") || (strstr(Tmp, "gl_FrontMaterial")) || strstr(Tmp, "gl_BackMaterial"))
    {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_MaterialParametersSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_MaterialParametersSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_MaterialParameters", "_gl4es_MaterialParameters");
    }
    if(strstr(Tmp, "gl_LightSource")) {
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_LightSource", "_gl4es_LightSource");
    }
    if(strstr(Tmp, "gl_LightModel"))
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_LightModel", "_gl4es_LightModel");
    if(strstr(Tmp, "gl_FrontLightModelProduct"))
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_FrontLightModelProduct", "_gl4es_FrontLightModelProduct");
    if(strstr(Tmp, "gl_BackLightModelProduct"))
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_BackLightModelProduct", "_gl4es_BackLightModelProduct");
    if(strstr(Tmp, "gl_FrontLightProduct"))
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_FrontLightProduct", "_gl4es_FrontLightProduct");
    if(strstr(Tmp, "gl_BackLightProduct"))
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_BackLightProduct", "_gl4es_BackLightProduct");
    if(strstr(Tmp, "gl_FrontMaterial"))
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_FrontMaterial", "_gl4es_FrontMaterial");
    if(strstr(Tmp, "gl_BackMaterial"))
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_BackMaterial", "_gl4es_BackMaterial");
    if(strstr(Tmp, "gl_MaxLights"))
    {
        Tmp = InplaceInsert(GetLine(Tmp, 2), gl4es_MaxLightsSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_MaxLightsSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_MaxLights", "_gl4es_MaxLights");
    }
    if(strstr(Tmp, "gl_NormalScale")) {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_normalscaleSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_normalscaleSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_NormalScale", "_gl4es_NormalScale");
    }
    if(strstr(Tmp, "gl_InstanceID") || strstr(Tmp, "gl_InstanceIDARB")) {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_instanceID, Tmp, &tmpsize);
        headline+=CountLine(gl4es_instanceID);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_InstanceIDARB", "_gl4es_InstanceID");
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_InstanceID", "_gl4es_InstanceID");
    }
    if(strstr(Tmp, "gl_ClipPlane")) {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_clipplanesSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_clipplanesSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_ClipPlane", "_gl4es_ClipPlane");
    }
    if(strstr(Tmp, "gl_MaxClipPlanes")) {
        Tmp = InplaceInsert(GetLine(Tmp, 2), gl4es_MaxClipPlanesSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_MaxClipPlanesSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_MaxClipPlanes", "_gl4es_MaxClipPlanes");
    }

    if(strstr(Tmp, "gl_PointParameters") || strstr(Tmp, "gl_Point"))
    {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_PointSpriteSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_PointSpriteSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_PointParameters", "_gl4es_PointParameters");
    }
    if(strstr(Tmp, "gl_Point"))
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_Point", "_gl4es_Point");
    if(strstr(Tmp, "gl_FogParameters") || strstr(Tmp, "gl_Fog"))
    {
        Tmp = InplaceInsert(GetLine(Tmp, headline), 1?gl4es_FogParametersSourceHighp:gl4es_FogParametersSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_FogParametersSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_FogParameters", "_gl4es_FogParameters");
    }
    if(strstr(Tmp, "gl_Fog"))
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_Fog", "_gl4es_Fog");
    if(strstr(Tmp, "gl_TextureEnvColor")) {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_texenvcolorSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_texenvcolorSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_TextureEnvColor", "_gl4es_TextureEnvColor");
    }
    if(strstr(Tmp, "gl_EyePlaneS")) {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_texgeneyeSource[0], Tmp, &tmpsize);
        headline+=CountLine(gl4es_texgeneyeSource[0]);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_EyePlaneS", "_gl4es_EyePlaneS");
    }
    if(strstr(Tmp, "gl_EyePlaneT")) {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_texgeneyeSource[1], Tmp, &tmpsize);
        headline+=CountLine(gl4es_texgeneyeSource[1]);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_EyePlaneT", "_gl4es_EyePlaneT");
    }
    if(strstr(Tmp, "gl_EyePlaneR")) {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_texgeneyeSource[2], Tmp, &tmpsize);
        headline+=CountLine(gl4es_texgeneyeSource[2]);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_EyePlaneR", "_gl4es_EyePlaneR");
    }
    if(strstr(Tmp, "gl_EyePlaneQ")) {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_texgeneyeSource[3], Tmp, &tmpsize);
        headline+=CountLine(gl4es_texgeneyeSource[3]);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_EyePlaneQ", "_gl4es_EyePlaneQ");
    }
    if(strstr(Tmp, "gl_ObjectPlaneS")) {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_texgenobjSource[0], Tmp, &tmpsize);
        headline+=CountLine(gl4es_texgenobjSource[0]);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_ObjectPlaneS", "_gl4es_ObjectPlaneS");
    }
    if(strstr(Tmp, "gl_ObjectPlaneT")) {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_texgenobjSource[1], Tmp, &tmpsize);
        headline+=CountLine(gl4es_texgenobjSource[1]);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_ObjectPlaneT", "_gl4es_ObjectPlaneT");
    }
    if(strstr(Tmp, "gl_ObjectPlaneR")) {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_texgenobjSource[2], Tmp, &tmpsize);
        headline+=CountLine(gl4es_texgenobjSource[2]);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_ObjectPlaneR", "_gl4es_ObjectPlaneR");
    }
    if(strstr(Tmp, "gl_ObjectPlaneQ")) {
        Tmp = InplaceInsert(GetLine(Tmp, headline), gl4es_texgenobjSource[3], Tmp, &tmpsize);
        headline+=CountLine(gl4es_texgenobjSource[3]);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_ObjectPlaneQ", "_gl4es_ObjectPlaneQ");
    }

    if(strstr(Tmp, "gl_MaxTextureUnits")) {
        Tmp = InplaceInsert(GetLine(Tmp, 2), gl4es_MaxTextureUnitsSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_MaxTextureUnitsSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_MaxTextureUnits", "_gl4es_MaxTextureUnits");
    }
    if(strstr(Tmp, "gl_MaxTextureCoords")) {
        Tmp = InplaceInsert(GetLine(Tmp, 2), gl4es_MaxTextureCoordsSource, Tmp, &tmpsize);
        headline+=CountLine(gl4es_MaxTextureCoordsSource);
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_MaxTextureCoords", "_gl4es_MaxTextureCoords");
    }
    if(strstr(Tmp, "gl_ClipVertex")) {
        // gl_ClipVertex is not handled for now
        // Proper way would be to copy handling from fpe_shader, but then, need to use gl_ClipPlane...
        static int ncv = 0;
        char CV[60];
        sprintf(CV, gl4es_dummyClipVertex, ncv);
        ++ncv;
        Tmp = InplaceReplace(Tmp, &tmpsize, "gl_ClipVertex", CV);
    }
    //oldprogram uniforms...
    if(FindString(Tmp, gl_ProgramEnv)) {
        // check if array can be removed
        int maxind = -1;
        int noarray_ok = 1;
        char* p = Tmp;
        while(noarray_ok && (p=FindStringNC(p, gl_ProgramEnv))) {
            p+=strlen(gl_ProgramEnv);
            if(*p=='[') {
                ++p;
                if(*p>='0' && *p<='9') {
                    int n = (*p) - '0';
                    if(p[1]>='0' && p[1]<='9')
                        n = n*10 + (p[1] - '0');
                    if (maxind<n) maxind = n;
                } else
                    noarray_ok=0;
            } else
                noarray_ok=0;
        }
        if(noarray_ok) {
            // ok, so change array to single...
            char F[60], T[60], U[300];
            for(int i=0; i<=maxind; ++i) {
                sprintf(F, "%s[%d]", gl_ProgramEnv, i);
                sprintf(T, "_gl4es_%s_ProgramEnv_%d", isVertex?"Vertex":"Fragment", i);
                Tmp = InplaceReplace(Tmp, &tmpsize, F, T);
                if(FindString(Tmp, T)) {
                    // add the uniform declaration if needed
                    sprintf(U, "uniform vec4 %s;\n", T);
                    Tmp = InplaceInsert(GetLine(Tmp, headline), U, Tmp, &tmpsize);
                    headline += 1;
                }
            }
        } else {
            // need the full array...
            char T[60], U[300];
            sprintf(T, "_gl4es_%s_ProgramEnv", isVertex?"Vertex":"Fragment");
            sprintf(U, "uniform vec4 %s[%d];\n", T, isVertex?MAX_VTX_PROG_ENV_PARAMS:MAX_FRG_PROG_ENV_PARAMS);
            Tmp = InplaceInsert(GetLine(Tmp, headline), U, Tmp, &tmpsize);
            headline += 1;
            Tmp = InplaceReplace(Tmp, &tmpsize, gl_ProgramEnv, T);
        }
    }
    if(FindString(Tmp, gl_ProgramLocal)) {
        // check if array can be removed
        int maxind = -1;
        int noarray_ok = 1;
        char* p = Tmp;
        while(noarray_ok && (p=FindStringNC(p, gl_ProgramLocal))) {
            p+=strlen(gl_ProgramLocal);
            if(*p=='[') {
                ++p;
                if(*p>='0' && *p<='9') {
                    int n = (*p) - '0';
                    if(p[1]>='0' && p[1]<='9')
                        n = n*10 + (p[1] - '0');
                    if (maxind<n) maxind = n;
                } else
                    noarray_ok=0;
            } else
                noarray_ok=0;
        }
        if(noarray_ok) {
            // ok, so change array to single...
            char F[60], T[60], U[300];
            for(int i=0; i<=maxind; ++i) {
                sprintf(F, "%s[%d]", gl_ProgramLocal, i);
                sprintf(T, "_gl4es_%s_ProgramLocal_%d", isVertex?"Vertex":"Fragment", i);
                Tmp = InplaceReplace(Tmp, &tmpsize, F, T);
                if(FindString(Tmp, T)) {
                    // add the uniform declaration if needed
                    sprintf(U, "uniform vec4 %s;\n", T);
                    Tmp = InplaceInsert(GetLine(Tmp, headline), U, Tmp, &tmpsize);
                    headline += 1;
                }
            }
        } else {
            // need the full array...
            char T[60], U[300];
            sprintf(T, "_gl4es_%s_ProgramLocal", isVertex?"Vertex":"Fragment");
            sprintf(U, "uniform vec4 %s[%d];\n", T, isVertex?MAX_VTX_PROG_LOC_PARAMS:MAX_FRG_PROG_LOC_PARAMS);
            Tmp = InplaceInsert(GetLine(Tmp, headline), U, Tmp, &tmpsize);
            headline += 1;
            Tmp = InplaceReplace(Tmp, &tmpsize, gl_ProgramLocal, T);
        }
    }
#define GO(A) \
  if(strstr(Tmp, gl_Samplers ## A)) {                                   \
    char S[60], D[60], U[60];                                           \
    for(int i=0; i<MAX_TEX; ++i) {                                      \
      sprintf(S, "%s%d", gl_Samplers ## A, i);                          \
      if(FindString(Tmp, S)) {                                          \
        sprintf(D, "%s%d", gl4es_Samplers ## A, i);                     \
        sprintf(U, "%s%d;\n", gl4es_Samplers ## A ## _uniform, i);      \
        Tmp = InplaceReplace(Tmp, &tmpsize, S, D);                      \
        Tmp = InplaceInsert(GetLine(Tmp, headline), U, Tmp, &tmpsize);  \
        headline += 1;                                                  \
      }                                                                 \
    }                                                                   \
  }
    GO(1D)
    GO(2D)
    GO(3D)
    GO(Cube)
#undef GO

    // non-square matrix handling
    // the square one first
    if(strstr(Tmp, "mat2x2")) {
        // better to use #define ?
        Tmp = InplaceReplace(Tmp, &tmpsize, "mat2x2", "mat2");
    }
    if(strstr(Tmp, "mat3x3")) {
        // better to use #define ?
        Tmp = InplaceReplace(Tmp, &tmpsize, "mat3x3", "mat3");
    }

    if (versionHeader > 1) {
        const char* GLESBackport = "#define texture2D texture\n#define attribute in\n#define varying out\n";
        Tmp = InplaceInsert(GetLine(Tmp, 1), GLESBackport, Tmp, &tmpsize);
    }else {
        const char* GLESForwardPort = "#define texture texture2D\n #define textureProj texture2DProj\n #define mod(a,b) (int(a) - int(b) * int(a/b))\n";
        Tmp = InplaceInsert(GetLine(Tmp, 1), GLESForwardPort, Tmp, &tmpsize);
    }

    // finish
    
    // clean preproc'd source
    if(pEntry!=pBuffer)
        free(pBuffer);
    return Tmp;
}

int isBuiltinAttrib(const char* name) {
    int n = sizeof(builtin_attrib)/sizeof(builtin_attrib_t);
    for (int i=0; i<n; i++) {
        if (strcmp(builtin_attrib[i].name, name)==0)
            return builtin_attrib[i].attrib;
    }
    return -1;
}

int isBuiltinMatrix(const char* name) {
    int ret = -1;
    int n = sizeof(builtin_matrix)/sizeof(builtin_matrix_t);
    for (int i=0; i<n && ret==-1; i++) {
        if (strncmp(builtin_matrix[i].name, name, strlen(builtin_matrix[i].name))==0) {
            int l=strlen(builtin_matrix[i].name);
            if(strlen(name)==l
               || (strlen(name)==l+3 && name[l]=='[' && builtin_matrix[i].texarray)
               || (strlen(name)==l+4 && name[l]=='[' && builtin_matrix[i].texarray)
                    ) {
                ret=builtin_matrix[i].matrix;
                if(builtin_matrix[i].texarray) {
                    int n = name[l+1] - '0';
                    if(name[l+2]>='0' && name[l+2]<='9')
                        n = n*10 + name[l+2]-'0';
                    ret+=n*4;
                }
            }
        }
    }
    return ret;
}

const char* hasBuiltinAttrib(const char* vertexShader, int Att) {
    if(!vertexShader) // can happens (like if the shader is a pure GLES2 one)
        return NULL;
    // first search for the string
    const char* ret = NULL;
    if(24>8) {
        int n = sizeof(builtin_attrib)/sizeof(builtin_attrib_t);
        for (int i=0; i<n && !ret; i++) {
            if (builtin_attrib[i].attrib == Att)
                ret = builtin_attrib[i].name;
        }
    } else {
        int n = sizeof(builtin_attrib_compressed)/sizeof(builtin_attrib_t);
        for (int i=0; i<n && !ret; i++) {
            if (builtin_attrib_compressed[i].attrib == Att)
                ret = builtin_attrib_compressed[i].name;
        }
    }
    if (!ret)
        return NULL;
    if(strstr(vertexShader, ret)) // it's here!
        return ret;
    // check for old program generic vertex attribs
    if(strstr(vertexShader, gl4es_VA[Att]))
        return gl4es_VA[Att];
    // nope
    return NULL;
}

const char* builtinAttribGLName(const char* name) {
    // no need to check for compressed array here...
    int n = sizeof(builtin_attrib)/sizeof(builtin_attrib_t);
    for(int i=0; i<n; ++i)
        if(!strcmp(name, builtin_attrib[i].name))
            return builtin_attrib[i].glname;
    if(!strncmp(name, gl4es_VertexAttrib, strlen(gl4es_VertexAttrib))) {
        int l = strlen(gl4es_VertexAttrib);
        int n = 0;
        while(name[l]>='0' && name[l]<='9')
            n = n*10 + name[l++]-'0';
        return gl_VA[n];
    }
    return name;
}

const char* builtinAttribInternalName(const char* name) {
    // no need to check for compressed array here...
    int n = sizeof(builtin_attrib)/sizeof(builtin_attrib_t);
    for(int i=0; i<n; ++i)
        if(!strcmp(name, builtin_attrib[i].glname))
            return builtin_attrib[i].name;
    if(!strncmp(name, gl_VertexAttrib, strlen(gl_VertexAttrib))) {
        int l = strlen(gl_VertexAttrib);
        int n = 0;
        while(name[l]>='0' && name[l]<='9')
            n = n*10 + name[l++]-'0';
        return gl4es_VA[n];
    }
    return name;
}

int testGenericShader(GLenum type, char* glsl) {
    // check the current shader is valid for compilation

    GLuint shad = GLES.glCreateShader(type);
    GLES.glShaderSource(shad, 1, (const GLchar *const *)(&glsl), NULL);
    GLES.glCompileShader(shad);
    GLint compiled;
    GLES.glGetShaderiv(shad, GL_COMPILE_STATUS, &compiled);
    /*
    if(!compiled) {
        LOAD_GLES2(glGetShaderInfoLog)
        char buff[500];
        GLES.glGetShaderInfoLog(shad, 500, NULL, buff);
        printf("LIBGL: \"%s\" failed, message:\n%s\n", version, buff);
    }
    */
    GLES.glDeleteShader(shad);

    return compiled;
}


char * GLSLHeader(char* source);
char * RemoveConstInsideBlocks(char* source, int * sourceLength);
char * ForceIntegerArrayAccess(char* source, int * sourceLength);
char * CoerceIntToFloat(char * source, int * sourceLength);
char * ReplaceModOperator(char * source, int * sourceLength);
char * WrapIvecFunctions(char * source, int * sourceLength);
char * WrapFunction(char * source, int * sourceLength, char * functionName, char * wrapperFunctionName, char * wrapperFunction);
int FindPositionAfterDirectives(char * source);
int FindPositionAfterVersion(char * source);
char * ReplaceGLFragData(char * source, int * sourceLength);
char * ReplaceGLFragColor(char * source, int * sourceLength);
char * ReplaceVariableName(char * source, int * sourceLength, char * initialName, char* newName);
char * ReplaceFunctionName(char * source, int * sourceLength, char * initialName, char * finalName);
char * RemoveUnsupportedExtensions(char * source);
int doesShaderVersionContainsES(const char * source);
char *ReplacePrecisionQualifiers(char *source, int *sourceLength, int isVertex);
char * GetClosingTokens(char openingToken);
int isOpeningToken(char openingToken);
int GetClosingTokenPosition(const char * source, int initialTokenPosition);
int GetClosingTokenPositionTokenOverride(const char * source, int initialTokenPosition, char initialToken);
int GetNextTokenPosition(const char * source, int initialPosition, char token, const char * acceptedChars);
void GetNextWord(char *source, int startPoint, int * startWord, int * endWord);
unsigned long strstrPos(const char * haystack, const char * needle);
char * insertIntAtFunctionCall(char * source, int * sourceSize, const char * functionName, int argumentPosition);
char * InsertExtension(char * source, int * sourceLength, int insertPoint, const char * extension);
char * InsertExtensions(char *source, int *sourceLength);
int GetShaderVersion(const char * source);
char * ReplaceFragmentOut(char * source, int *sourceLength);
char * BackportConstArrays(char *source, int * sourceLength);
char * ExtractString(char * source, int startString, int endString);
char* ProcessSwitchCases(char* source, int* length);


char* GetOperandFromOperator(char* source, int operatorIndex, int rightOperand, int * limit);

/** Convert the shader through multiple steps
 * @param source The start of the shader as a string*/
char * ConvertShaderVgpu(char *shader_source, GLenum type){

    // Get the shader source
    char * source = shader_source;
    int sourceLength = strlen(source) + 1;
#define vgpu_backport 1
    // For now, skip stuff
    if(FindString(source, "#version 100")){
        if(vgpu_backport){
            if (type == GL_VERTEX_SHADER){
                source = ReplaceVariableName(source, &sourceLength, "in", "attribute");
                source = ReplaceVariableName(source, &sourceLength, "out", "varying");
            }else{
                source = ReplaceVariableName(source, &sourceLength, "in", "varying");
                source = ReplaceFragmentOut(source, &sourceLength);
            }

            // Well, we don't have gl_VertexID on OPENGL 1
            source = ReplaceVariableName(source, &sourceLength, "gl_VertexID", "0");
            source = InplaceReplaceSimple(source, &sourceLength, "ivec", "vec");
            source = InplaceReplaceSimple(source, &sourceLength, "bvec", "vec");
            source = InplaceReplaceSimple(source, &sourceLength, "flat ", "");

            source = BackportConstArrays(source, &sourceLength);
            int insertPoint = FindPositionAfterVersion(source);
            source = InplaceInsertByIndex(source, &sourceLength, insertPoint + 1, "#define texelFetch(a, b, c) vec4(1.0,1.0,1.0,1.0) \n");

            source = ReplaceModOperator(source, &sourceLength);

            return source;
        }

        // Else, skip the conversion
        
        return source;
    }


    // Remove 'const' storage qualifier
    //printf("REMOVING CONST qualifiers");
    //source = RemoveConstInsideBlocks(source, &sourceLength);
    //source = ReplaceVariableName(source, &sourceLength, "const", " ");




    // Avoid keyword clash with gl4es #define blocks
    //printf("REPLACING KEYWORDS");
    source = InplaceReplaceSimple(source, &sourceLength, "#define texture2D texture\n", "");
    source = ReplaceVariableName(source, &sourceLength, "sample", "vgpu_Sample");
    source = ReplaceVariableName(source, &sourceLength, "texture", "vgpu_texture");

    source = ReplaceFunctionName(source, &sourceLength, "texture2D", "texture");
    source = ReplaceFunctionName(source, &sourceLength, "texture3D", "texture");
    source = ReplaceFunctionName(source, &sourceLength, "texture2DLod", "textureLod");


    //printf("REMOVING \" CHARS ");
    // " not really supported here
    source = InplaceReplaceSimple(source, &sourceLength, "\"", "");

    // For now let's hope no extensions are used
    // TODO deal with extensions but properly
    //printf("REMOVING EXTENSIONS");
    //source = RemoveUnsupportedExtensions(source);

    // OpenGL natively supports non const global initializers, not OPENGL ES except if we add an extension
    //printf("ADDING EXTENSIONS\n");
    source = InsertExtensions(source, &sourceLength);

    //printf("REPLACING mod OPERATORS");
    // No support for % operator, so we replace it
    source = ReplaceModOperator(source, &sourceLength);

    //printf("COERCING INT TO FLOATS");
    // Hey we don't want to deal with implicit type stuff
    source = CoerceIntToFloat(source, &sourceLength);

    //printf("FIXING ARRAY ACCESS");
    // Avoid any weird type trying to be an index for an array
    source = ForceIntegerArrayAccess(source, &sourceLength);

    //printf("WRAPPING FUNCTION");
    // Since everything is a float, we need to overload WAY TOO MANY functions
    source = WrapIvecFunctions(source, &sourceLength);

    //printf("REMOVING DUBIOUS DEFINES");
    source = InplaceReplaceSimple(source, &sourceLength, "#define texture texture2D\n", "");
    source = InplaceReplaceSimple(source, &sourceLength, "#define attribute in\n", "");
    source = InplaceReplaceSimple(source, &sourceLength, "#define varying out\n", "");

    if (type == GL_VERTEX_SHADER){
        source = ReplaceVariableName(source, &sourceLength, "attribute", "in");
        source = ReplaceVariableName(source, &sourceLength, "varying", "out");
    }else{
        source = ReplaceVariableName(source, &sourceLength, "varying", "in");
    }

    // Draw buffers aren't dealt the same on OPEN GL|ES
    if(type == GL_FRAGMENT_SHADER && doesShaderVersionContainsES(source) ){
        //printf("REPLACING FRAG DATA");
        source = ReplaceGLFragData(source, &sourceLength);
        //printf("REPLACING FRAG COLOR");
        source = ReplaceGLFragColor(source, &sourceLength);
    }

    //printf("FUCKING UP PRECISION");
    source = ReplacePrecisionQualifiers(source, &sourceLength, type == GL_VERTEX_SHADER);

    source = ProcessSwitchCases(source, &sourceLength);


    return source;
}

static const char* switch_template = " switch (%n%[^)] { ";
static const char* case_template = " case %n%[^:] ";
static const char* declaration_template = " const float %s = %s ;";

#define VARIABLE_SIZE 1024
#define MODE_SWITCH 0
#define MODE_CASE 1


/**
 * Convert switches or cases to be usable with the current int to float conversion
 * @param source The shader as a string
 * @param sourceLength The shader allocated length
 * @param mode Whether to scan and fix switches or cases
 * @return The shader as a string, converted appropriately, maybe in a different memory location
*/
char* FindAndCorrect(char* source, int* length, int mode) {
    const char*     template_ = mode == MODE_SWITCH ? switch_template : mode == MODE_CASE ? case_template : NULL;
    char*           scan_source = source;
    char            template_string[VARIABLE_SIZE];
    size_t          string_offset;
    size_t          offset = 0;
    unsigned char   rewind = 0;
    while(1) {
        int scan_result = sscanf(scan_source, template_, &string_offset, &template_string);
        if(scan_result == 0) {
            scan_source++;
            continue;
        }else if(scan_result == EOF) {
            break;
        }
        offset = string_offset + (strstr(scan_source, mode == MODE_SWITCH ? "{" : mode == MODE_CASE ? ":" : 0) - scan_source); // find it by hand cause sscanf has trouble with two %n operators
        string_offset += (scan_source - source); // convert it from relative to scan to relative to base
        if(mode == MODE_SWITCH && !strstr(template_string, "int(") ) { // already cast to int, skip
            size_t insert_end_offset = string_offset + strlen(template_string);
            source = InplaceInsertByIndex(source, length, insert_end_offset, ")");
            source = InplaceInsertByIndex(source, length, string_offset, "int(");
            rewind = 1;
        }
        if(mode == MODE_CASE) {
            if(!isdigit(template_string[0])) { // cant have a number without the first digit, and the standard doesnt permit variable names starting with numbers
                char   decltemplate_formatted[VARIABLE_SIZE];
                float  declared_value = 99;
                snprintf(decltemplate_formatted, VARIABLE_SIZE, declaration_template, template_string, "%f");
                printf("Scanning with template %s\n", decltemplate_formatted);
                char* scanbase = source;
                while(1) {
                    int result = sscanf(scanbase, decltemplate_formatted, &declared_value);
                    if(result == 0) {
                        scanbase++;
                        continue;
                    }else if(result == EOF) {
                        printf("Scanned the whole shader and didn't find declaration for %s with template \"%s\"\n", template_string, decltemplate_formatted);
                        abort();
                    }
                    break;
                }
                char   integer[VARIABLE_SIZE];
                snprintf(integer, VARIABLE_SIZE, "%i", (int)declared_value);
                size_t replace_end_offset = string_offset + strlen(template_string)-1;
                source = InplaceReplaceByIndex(source, length, string_offset, replace_end_offset, integer);
                rewind = 1;
            }
        }
        if(rewind) {
            scan_source = source; // since inplace replacement operations are destructive, the scan will be rewound after doing them
            rewind = 0;
        }else scan_source += offset;
    }
    return source;
}

/**
 *Convert switches and cases in the shader to be usable with the current int to float coercion
 * @param source The shader as a string
 * @param sourceLength The shader allocated length
 * @return The shader as a string, converted appropriately, maybe in a different memory location
*/

char* ProcessSwitchCases(char* source, int* length) {
    source = FindAndCorrect(source, length, MODE_SWITCH);
    source = FindAndCorrect(source, length, MODE_CASE);
    return source;
}

/**
 * Turn const arrays and its accesses into a function and function calls
 * @param source The shader as a string
 * @param sourceLength The shader allocated length
 * @return The shader as a string, maybe in a different memory location
 */
char * BackportConstArrays(char *source, int * sourceLength){
    unsigned long startPoint = strstrPos(source, "const");
    if(startPoint == 0){
        return source;
    }
    int constStart, constStop;
    GetNextWord(source, startPoint, &constStart, &constStop); // Catch the "const"

    int typeStart, typeStop;
    GetNextWord(source, constStop, &typeStart, &typeStop); // Catch the type, without []

    int variableNameStart, variableNameStop;
    GetNextWord(source, typeStop, &variableNameStart, &variableNameStop); // Catch the var name
    char * variableName = ExtractString(source, variableNameStart, variableNameStop);

    //Now, verify the data type is actually an array
    char * tokenStart = strstr(source + typeStop, "[");
    if( tokenStart != NULL && (tokenStart - source) < variableNameStart){
        // We've found an array. So we need to get to the starting parenthesis and isolate each member
        int startArray = GetNextTokenPosition(source, variableNameStop, '(', "");
        int endArray = GetClosingTokenPosition(source, startArray);

        // First pass, to count the amount of entries in the array
        int arrayEntryCount = -1;
        int currentPoint = startArray;
        while (currentPoint < endArray){
            ++arrayEntryCount;
            currentPoint = GetClosingTokenPositionTokenOverride(source, currentPoint, ',');
        }
        if(currentPoint == endArray){
            ++arrayEntryCount;
        }

        // Now we know how many entries we have, we can copy data
        int entryStart = startArray + 1;
        int entryEnd;
        for(int j=0; j<arrayEntryCount; ++j){
            // First, isolate the array entry
            entryEnd = GetClosingTokenPositionTokenOverride(source, entryStart, ',');

            // Replace the entry and jump to the end of the replacement
            source = InplaceReplaceByIndex(source, sourceLength, entryEnd , entryEnd +1, ";}"); // +2 - 1
            // Build the string to insert
            int indexStringLength = (j == 0 ? 1 : (int)(log10(j)+1));
            char * replacementString = (char *)malloc(19 + indexStringLength + 1);
            replacementString[19 + indexStringLength + 1] = '\0';
            memcpy(replacementString, "if(index==", 10);
            sprintf(replacementString + 10, "%d", j);
            strcpy(replacementString + 10 + indexStringLength, "){return ");

            // Insert the correct index in the replacement string
            source = InplaceInsertByIndex(source, sourceLength, entryStart, replacementString);

            entryStart = entryEnd + 19 + indexStringLength + 2;
            free(replacementString);
        }

        // The replacement string is not needed anymore


        // Add The last "}" to close the function
        source = InplaceInsertByIndex(source, sourceLength, entryStart, "}");
        // Add the argument section of the function
        source = InplaceReplaceByIndex(source, sourceLength, variableNameStop, startArray , "(int index){");
        // Remove the []
        source = InplaceReplaceByIndex(source, sourceLength, typeStop, variableNameStart - 1, " ");
        // Finally, remove the "const" keyword
        source = InplaceReplaceByIndex(source, sourceLength, startPoint, startPoint + 5, " ");

        // Now, we have to turn every array access to a function call
        // TODO change the start position to be more accurate to the end of the function !
        for(int k = strstrPos(source + endArray, variableName) + endArray; k < strlen(source); ){
            int startAccess = GetNextTokenPosition(source, k, '[', "");
            int endAccess = GetClosingTokenPosition(source, startAccess);
            source = InplaceReplaceByIndex(source, sourceLength, endAccess, endAccess, ")");
            source = InplaceReplaceByIndex(source, sourceLength, startAccess, startAccess, "(");

            int nextPos = strstrPos(source + k, variableName) + k;
            if(nextPos == k) break;
            k = nextPos;
        }

        free(variableName);

    }else{
        // Nothing, go to the next loop iteration
    }

    return source;
}

/**
 * Extract a substring from the provided string
 * @param source The shader as a string
 * @param startString The start of the substring
 * @param endString The end of the substring
 * @return A newly allocated substring. Don't forget to free() it !
 */
char * ExtractString(char * source, int startString, int endString){
    char * subString = (char *)malloc((endString - startString) +1);
    subString[(endString - startString) +1] = '\0';
    memcpy(subString, source + startString, (endString - startString));
    return subString;
}

/**
 * Replace the out vec4 from a fragment shader by the gl_FragColor constant
 * @param source The shader as a string
 * @return The shader, maybe in a different memory location
 */
char * ReplaceFragmentOut(char * source, int *sourceLength){
    int startPosition = strstrPos(source, "out");
    if(startPosition == 0) return source; // No "out" keyword
    int t1, t2;
    GetNextWord(source, startPosition, &t1, &t2); // Catches "out"
    GetNextWord(source, t2, &t1, &t2); // Catches "vec4"
    GetNextWord(source, t2, &t1, &t2); // Catches the variableName

    // Load the variable inside another string
    char * variableName = (char *)malloc(t2 - t1 + 1);
    variableName[t2 - t1 + 1] = '\0';
    memcpy(variableName, source + t1, t2 - t1);

    // Removing the declaration
    source = InplaceReplaceByIndex(source, sourceLength, startPosition, t2 + 1, "");

    // Replacing occurrences of the variable
    source = ReplaceVariableName(source, sourceLength, variableName, "gl_FragColor");

    free(variableName);

    return source;
}

/**
 * Get to the start, then end of the next of current word.
 * @param source The shader as a string
 * @param startPoint The start point to look at
 * @param startWord Will point to the start of the word
 * @param endWord Will point to the end of the word
 */
void GetNextWord(char *source, int startPoint, int * startWord, int * endWord){
    // Step 1: Find the real start point
    int start = 0;
    while(!start){
        if(isValidFunctionName(source[startPoint] ) || isDigit(source[startPoint])){
            start = startPoint;
            break;
        }
        ++startPoint;
    }

    // Step 2: Find the end of a word
    int end = 0;
    while (!end){
        if(!isValidFunctionName(source[startPoint] ) && !isDigit(source[startPoint])){
            end = startPoint;
            break;
        }
        ++startPoint;
    }

    // Then return values
    *startWord = start;
    *endWord = end;
}

char * InsertExtensions(char *source, int *sourceLength){
    int insertPoint = FindPositionAfterDirectives(source);
    //printf("INSERT POINT: %i\n", insertPoint);

    source = InsertExtension(source, sourceLength, insertPoint+1, "GL_EXT_shader_non_constant_global_initializers");
    source = InsertExtension(source, sourceLength, insertPoint+1, "GL_EXT_texture_cube_map_array");
    source = InsertExtension(source, sourceLength, insertPoint+1, "GL_EXT_texture_buffer");
    source = InsertExtension(source, sourceLength, insertPoint+1, "GL_OES_texture_storage_multisample_2d_array");
    return source;
}

char * InsertExtension(char * source, int * sourceLength, const int insertPoint, const char * extension){
    // First, insert the model, then the extension
    source = InplaceInsertByIndex(source, sourceLength, insertPoint, "#ifdef __EXT__ \n#extension __EXT__ : enable\n#endif\n");
    source = InplaceReplaceSimple(source, sourceLength, "__EXT__", extension);
    return source;
}

int doesShaderVersionContainsES(const char * source){
    return GetShaderVersion(source) >= 300;
}

char * WrapIvecFunctions(char * source, int * sourceLength){
    source = WrapFunction(source, sourceLength, "texelFetch", "vgpu_texelFetch", "\nvec4 vgpu_texelFetch(sampler2D sampler, vec2 P, float lod){return texelFetch(sampler, ivec2(int(P.x), int(P.y)), int(lod));}\n"
                                                                                 "vec4 vgpu_texelFetch(sampler3D sampler, vec3 P, float lod){return texelFetch(sampler, ivec3(int(P.x), int(P.y), int(P.z)), int(lod));}\n"
                                                                                 "vec4 vgpu_texelFetch(sampler2DArray sampler, vec3 P, float lod){return texelFetch(sampler, ivec3(int(P.x), int(P.y), int(P.z)), int(lod));}\n"
                                                                                 "#ifdef GL_EXT_texture_buffer\n"
                                                                                 "vec4 vgpu_texelFetch(samplerBuffer sampler, float P){return texelFetch(sampler, int(P));}\n"
                                                                                 "#endif\n"
                                                                                 "#ifdef GL_OES_texture_storage_multisample_2d_array\n"
                                                                                 "vec4 vgpu_texelFetch(sampler2DMS sampler, vec2 P, float _sample){return texelFetch(sampler, ivec2(int(P.x), int(P.y)), int(_sample));}\n"
                                                                                 "vec4 vgpu_texelFetch(sampler2DMSArray sampler, vec3 P, float _sample){return texelFetch(sampler, ivec3(int(P.x), int(P.y), int(P.z)), int(_sample));}\n"
                                                                                 "#endif\n");

    source = WrapFunction(source, sourceLength, "textureSize", "vgpu_textureSize", "\nvec2 vgpu_textureSize(sampler2D sampler, float lod){ivec2 size = textureSize(sampler, int(lod));return vec2(size.x, size.y);}\n"
                                                                                   "vec3 vgpu_textureSize(sampler3D sampler, float lod){ivec3 size = textureSize(sampler, int(lod));return vec3(size.x, size.y, size.z);}\n"
                                                                                   "vec2 vgpu_textureSize(samplerCube sampler, float lod){ivec2 size = textureSize(sampler, int(lod));return vec2(size.x, size.y);}\n"
                                                                                   "vec2 vgpu_textureSize(sampler2DShadow sampler, float lod){ivec2 size = textureSize(sampler, int(lod));return vec2(size.x, size.y);}\n"
                                                                                   "vec2 vgpu_textureSize(samplerCubeShadow sampler, float lod){ivec2 size = textureSize(sampler, int(lod));return vec2(size.x, size.y);}\n"
                                                                                   "#ifdef GL_EXT_texture_cube_map_array\n"
                                                                                   "vec3 vgpu_textureSize(samplerCubeArray sampler, float lod){ivec3 size = textureSize(sampler, int(lod));return vec3(size.x, size.y, size.z);}\n"
                                                                                   "vec3 vgpu_textureSize(samplerCubeArrayShadow sampler, float lod){ivec3 size = textureSize(sampler, int(lod));return vec3(size.x, size.y, size.z);}\n"
                                                                                   "#endif\n"
                                                                                   "vec3 vgpu_textureSize(sampler2DArray sampler, float lod){ivec3 size = textureSize(sampler, int(lod));return vec3(size.x, size.y, size.z);}\n"
                                                                                   "vec3 vgpu_textureSize(sampler2DArrayShadow sampler, float lod){ivec3 size = textureSize(sampler, int(lod));return vec3(size.x, size.y, size.z);}\n"
                                                                                   "#ifdef GL_EXT_texture_buffer\n"
                                                                                   "float vgpu_textureSize(samplerBuffer sampler){return float(textureSize(sampler));}\n"
                                                                                   "#endif\n"
                                                                                   "#ifdef GL_OES_texture_storage_multisample_2d_array\n"
                                                                                   "vec2 vgpu_textureSize(sampler2DMS sampler){ivec2 size = textureSize(sampler);return vec2(size.x, size.y);}\n"
                                                                                   "vec3 vgpu_textureSize(sampler2DMSArray sampler){ivec3 size = textureSize(sampler);return vec3(size.x, size.y, size.z);}\n"
                                                                                   "#endif\n");

    source = WrapFunction(source, sourceLength, "textureOffset", "vgpu_textureOffset", "\nvec4 vgpu_textureOffset(sampler2D tex, vec2 P, vec2 offset, float bias){ivec2 Size = textureSize(tex, 0);return texture(tex, P+offset/vec2(float(Size.x), float(Size.y)), bias);}\n"
                                                                                       "vec4 vgpu_textureOffset(sampler2D tex, vec2 P, vec2 offset){return vgpu_textureOffset(tex, P, offset, 0.0);}\n"
                                                                                       "vec4 vgpu_textureOffset(sampler3D tex, vec3 P, vec3 offset, float bias){ivec3 Size = textureSize(tex, 0);return texture(tex, P+offset/vec3(float(Size.x), float(Size.y), float(Size.z)), bias);}\n"
                                                                                       "vec4 vgpu_textureOffset(sampler3D tex, vec3 P, vec3 offset){return vgpu_textureOffset(tex, P, offset, 0.0);}\n"
                                                                                       "float vgpu_textureOffset(sampler2DShadow tex, vec3 P, vec2 offset, float bias){ivec2 Size = textureSize(tex, 0);return texture(tex, P+vec3(offset.x, offset.y, 0)/vec3(float(Size.x), float(Size.y), 1.0), bias);}\n"
                                                                                       "float vgpu_textureOffset(sampler2DShadow tex, vec3 P, vec2 offset){return vgpu_textureOffset(tex, P, offset, 0.0);}\n"
                                                                                       "vec4 vgpu_textureOffset(sampler2DArray tex, vec3 P, vec2 offset, float bias){ivec3 Size = textureSize(tex, 0);return texture(tex, P+vec3(offset.x, offset.y, 0)/vec3(float(Size.x), float(Size.y), float(Size.z)), bias);}\n"
                                                                                       "vec4 vgpu_textureOffset(sampler2DArray tex, vec3 P, vec2 offset){return vgpu_textureOffset(tex, P, offset, 0.0);}\n");

    source = WrapFunction(source, sourceLength, "shadow2D", "vgpu_shadow2D", "\nvec4 vgpu_shadow2D(sampler2DShadow shadow, vec3 coord){return vec4(texture(shadow, coord), 0.0, 0.0, 0.0);}\n"
                                                                             "vec4 vgpu_shadow2D(sampler2DShadow shadow, vec3 coord, float bias){return vec4(texture(shadow, coord, bias), 0.0, 0.0, 0.0);}\n");
    return source;
}

/**
 * Replace a function and its calls by a wrapper version, only if needed
 * @param source The shader code as a string
 * @param functionName The function to be replaced
 * @param wrapperFunctionName The replacing function name
 * @param function The wrapper function itself
 * @return The shader as a string, maybe in a different memory location
 */
char * WrapFunction(char * source, int * sourceLength, char * functionName, char * wrapperFunctionName, char * wrapperFunction){
    int originalSize = strlen(source);
    source = ReplaceFunctionName(source, sourceLength, functionName, wrapperFunctionName);
    // If some calls got replaced, add the wrapper
    if(originalSize != strlen(source)){
        int insertPoint = FindPositionAfterDirectives(source);
        source = InplaceInsertByIndex(source, sourceLength, insertPoint + 1, wrapperFunction);
    }

    return source;
}

/**
 * Replace the % operator with a mathematical equivalent (x - y * floor(x/y))
 * @param source The shader as a string
 * @return The shader as a string, maybe in a different memory location
 */
char * ReplaceModOperator(char * source, int * sourceLength){
    char * modelString = " mod(x, y) ";
    int startIndex, endIndex = 0;
    int * startPtr = &startIndex, *endPtr = &endIndex;

    for(int i=0;i<*sourceLength; ++i){
        if(source[i] != '%') continue;
        // A mod operator is found !
        char * leftOperand = GetOperandFromOperator(source, i, 0, startPtr);
        char * rightOperand = GetOperandFromOperator(source,  i, 1, endPtr);

        // Generate a model string to be inserted
        char * replacementString = (char *)malloc(strlen(modelString) + 1);
        strcpy(replacementString, modelString);
        int replacementSize = strlen(replacementString);
        replacementString = InplaceReplace(replacementString, &replacementSize, "x", leftOperand);
        replacementString = InplaceReplace(replacementString, &replacementSize, "y", rightOperand);

        // Insert the new string
        source = InplaceReplaceByIndex(source, sourceLength, startIndex, endIndex, replacementString);

        // Free all the temporary strings
        free(leftOperand);
        free(rightOperand);
        free(replacementString);
    }

    return source;
}

/**
 * Change all (u)ints to floats.
 * This is a hack to avoid dealing with implicit conversions on common operators
 * @param source The shader as a string
 * @return The shader as a string, maybe in a new memory location
 * @see ForceIntegerArrayAccess
 */
char * CoerceIntToFloat(char * source, int * sourceLength){
    // Let's go the "freestyle way"

    // Step 1 is to translate keywords
    // Attempt and loop unrolling -> worked well, time to fix my shit I guess
    source = ReplaceVariableName(source, sourceLength, "int", "float");
    source = WrapFunction(source, sourceLength, "int", "float", "\n ");
    source = ReplaceVariableName(source, sourceLength, "uint", "float");
    source = WrapFunction(source, sourceLength, "uint", "float", "\n ");

    // TODO Yes I could just do the same as above but I'm lazy at times
    source = InplaceReplaceSimple(source, sourceLength, "ivec", "vec");
    source = InplaceReplaceSimple(source, sourceLength, "uvec", "vec");

    source = InplaceReplaceSimple(source, sourceLength, "isampleBuffer", "sampleBuffer");
    source = InplaceReplaceSimple(source, sourceLength, "usampleBuffer", "sampleBuffer");

    source = InplaceReplaceSimple(source, sourceLength, "isampler", "sampler");
    source = InplaceReplaceSimple(source, sourceLength, "usampler", "sampler");


    // Step 3 is slower.
    // We need to parse hardcoded values like 1 and turn it into 1.(0)
    for(int i=0; i<*sourceLength; ++i){

        // Avoid version/line directives
        if(source[i] == '#' && (source[i + 1] == 'v' || source[i + 1] == 'l') ){
            // Look for the next line
            while (source[i] != '\n' && source[i] != '\0'){
                i++;
            }
        }

        if(!isDigit(source[i])){ continue; }
        // So there is a few situations that we have to distinguish:
        // functionName1 (      ----- meaning there is SOMETHING on its left side that is related to the number
        // function(1,          ----- there is something, and it ISN'T related to the number
        // float test=3;        ----- something on both sides, not related to the number.
        // float test=X.2       ----- There is a dot, so it is part of a float already
        // float test = 0.00000 ----- I have to backtrack to find the dot

        if(source[i-1] == '.' || source[i+1] == '.') continue;// Number part of a float
        if(isValidFunctionName(source[i - 1])) continue; // Char attached to something related
        if(isDigit(source[i+1])) continue; // End of number not reached
        if(isDigit(source[i-1])){
            // Backtrack to check if the number is floating point
            int shouldBeCoerced = 0;
            for(int j=1; 1; ++j){
                if(isDigit(source[i-j])) continue;
                if(isValidFunctionName(source[i-j])) break; // Function or variable name, don't coerce
                if(source[i-j] == '.' || ((source[i-j] == '+' || source[i-j] == '-') && (source[i-j-1] == 'e'|| source[i-j-1] == 'E') )) break; // No coercion, float or scientific notation already
                // Nothing found, should be coerced then
                shouldBeCoerced = 1;
                break;
            }

            if(!shouldBeCoerced) continue;
        }

        // Now we know there is nothing related to the digit, turn it into a float
        source = InplaceInsertByIndex(source, sourceLength, i+1, ".0");
    }

    // TODO Hacks for special built in values and typecasts ?
    source = InplaceReplaceSimple(source, sourceLength, "gl_VertexID", "float(gl_VertexID)");
    source = InplaceReplaceSimple(source, sourceLength, "gl_InstanceID", "float(gl_InstanceID)");

    return source;
}

/** Force all array accesses to use integers by adding an explicit typecast
 * @param source The shader as a string
 * @return The shader as a string, maybe at a new memory location */
char * ForceIntegerArrayAccess(char* source, int * sourceLength){
    char * markerStart = "$";
    char * markerEnd = "`";

    // Step 1, we need to mark all [] that are empty and must not be changed
    int leftCharIndex = 0;
    for(int i=0; i< *sourceLength; ++i){
        if(source[i] == '['){
            leftCharIndex = i;
            continue;
        }
        // If a start has been found
        if(leftCharIndex){
            if(source[i] == ' ' || source[i] == '\n'){
                continue;
            }
            // We find the other side and mark both ends
            if(source[i] == ']'){
                source[leftCharIndex] = *markerStart;
                source[i] = *markerEnd;
            }
        }
        //Something else is there, abort the marking phase for this one
        leftCharIndex = 0;
    }

    // Step 2, replace the array accesses with a forced typecast version
    source = InplaceReplaceSimple(source, sourceLength, "]", ")]");
    source = InplaceReplaceSimple(source, sourceLength, "[", "[int(");

    // Step 3, restore all marked empty []
    source = InplaceReplaceSimple(source, sourceLength, markerStart, "[");
    source = InplaceReplaceSimple(source, sourceLength, markerEnd, "]");

    return source;
}

#define NO_OPERATOR_VALUE 9999

/** Small helper to help evaluate whether to continue or not I guess
 * Values over 9900 are not for real operators, more like stop indicators*/
int GetOperatorValue(char operator_){
    if(operator_ == ',' || operator_ == ';') return 9998;
    if(operator_ == '=') return 9997;
    if(operator_ == '+' || operator_ == '-') return 3;
    if(operator_ == '*' || operator_ == '/' || operator_ == '%') return 2;
    return NO_OPERATOR_VALUE; // Meaning no value;
}

/** Get the left or right operand, given the last index of the operator
 * It bases its ability to get operands by evaluating the priority of operators.
 * @param source The shader as a string
 * @param operatorIndex The index the operator is found
 * @param rightOperand Whether we get the right or left operator
 * @param limit The left or right index of the operand
 * @return newly allocated string with the operand
 */
char* GetOperandFromOperator(char* source, int operatorIndex, int rightOperand, int * limit){
    int parserState = 0;
    int parserDirection = rightOperand ? 1 : -1;
    int operandStartIndex = 0, operandEndIndex = 0;
    int parenthesesLeft = 0, hasFoundParentheses = 0;
    int operatorValue = GetOperatorValue(source[operatorIndex]);
    int lastOperator = 0; // Used to determine priority for unary operators

    char parenthesesStart = rightOperand ? '(' : ')';
    char parenthesesEnd = rightOperand ? ')' : '(';
    int stringIndex = operatorIndex;

    // Get to the operand
    while (parserState == 0){
        stringIndex += parserDirection;
        if(source[stringIndex] != ' '){
            parserState = 1;
            // Place the mark
            if(rightOperand){
                operandStartIndex = stringIndex;
            }else{
                operandEndIndex = stringIndex;
            }

            // Special case for unary operator when parsing to the right
            if(GetOperatorValue(source[stringIndex]) == 3 ){ // 3 is +- operators
                stringIndex += parserDirection;
            }
        }
    }

    // Get to the other side of the operand, the twist is here.
    while (parenthesesLeft > 0 || parserState == 1){

        // Look for parentheses
        if(source[stringIndex] == parenthesesStart){
            hasFoundParentheses = 1;
            parenthesesLeft += 1;
            stringIndex += parserDirection;
            continue;
        }

        if(source[stringIndex] == parenthesesEnd){
            hasFoundParentheses = 1;
            parenthesesLeft -= 1;

            // Likely to happen in a function call
            if(parenthesesLeft < 0){
                parserState = 3;
                if(rightOperand){
                    operandEndIndex = stringIndex - 1;
                }else{
                    operandStartIndex = stringIndex + 1;
                }
                continue;
            }
            stringIndex += parserDirection;
            continue;
        }

        // Small optimisation
        if(parenthesesLeft > 0){
            stringIndex += parserDirection;
            continue;
        }

        // So by now the following assumptions are made
        // 1 - We aren't between parentheses
        // 2 - No implicit multiplications are present
        // 3 - No fuckery with operators like "test = +-+-+-+-+-+-+-+-3;" although I attempt to support them

        // Higher value operators have less priority
        int currentValue = GetOperatorValue(source[stringIndex]);


        // The condition is different due to the evaluation order which is left to right, aside from the unary operators
        if((rightOperand ? currentValue >= operatorValue: currentValue > operatorValue)){
            if(currentValue == NO_OPERATOR_VALUE){
                if(source[stringIndex] == ' '){
                    stringIndex += parserDirection;
                    continue;
                }

                // Found an operand, so reset the operator eval for unary
                if(rightOperand) lastOperator = NO_OPERATOR_VALUE;

                // maybe it is the start of a function ?
                if(hasFoundParentheses){
                    parserState = 2;
                    continue;
                }
                // If no full () set is found, assume we didn't fully travel the operand
                stringIndex += parserDirection;
                continue;
            }

            // Special case when parsing unary operator to the right
            if(rightOperand && operatorValue == 3 && lastOperator < currentValue){
                stringIndex += parserDirection;
                continue;
            }

            // Stop, we found an operator of same worth.
            parserState = 3;
            if(rightOperand){
                operandEndIndex = stringIndex - 1;
            }else{
                operandStartIndex = stringIndex + 1;
            }
        }

        // Special case for unary operators from the right
        if(rightOperand && operatorValue == 3) { // 3 is + - operators
            lastOperator = currentValue;
        } // Special case for unary operators from the left
        if(!rightOperand && operatorValue < 3 && currentValue == 3){
            lastOperator = NO_OPERATOR_VALUE;
            for(int j=1; 1; ++j){
                int subCurrentValue = GetOperatorValue(source[stringIndex - j]);
                if(subCurrentValue != NO_OPERATOR_VALUE){
                    lastOperator = subCurrentValue;
                    continue;
                }

                // No operator value, can be almost anything
                if(source[stringIndex - j] == ' ') continue;
                // Else we found something. Did we found a high priority operator ?
                if(lastOperator <= operatorValue){ // If so, we allow continuing and going out of the loop
                    stringIndex -= j;
                    parserState = 1;
                    break;
                }
                // No other operator found
                operandStartIndex = stringIndex;
                parserState = 3;
                break;
            }
        }
        stringIndex += parserDirection;
    }

    // Status when we get the name of a function and nothing else.
    while (parserState == 2){
        if(source[stringIndex] != ' '){
            stringIndex += parserDirection;
            continue;
        }
        if(rightOperand){
            operandEndIndex = stringIndex - 1;
        }else{
            operandStartIndex = stringIndex + 1;
        }
        parserState = 3;
    }

    // At this point, we know both the start and end point of our operand, let's copy it
    char * operand = (char *)malloc(operandEndIndex - operandStartIndex + 2);
    memcpy(operand, source+operandStartIndex, operandEndIndex - operandStartIndex + 1);
    // Make sure the string is null terminated
    operand[operandEndIndex - operandStartIndex + 1] = '\0';

    // Send back the limitIndex
    *limit = rightOperand ? operandEndIndex : operandStartIndex;

    return operand;
}

/**
 * Replace any gl_FragData[n] reference by creating an out variable with the manual layout binding
 * @param source  The shader source as a string
 * @return The shader as a string, maybe at a different memory location
 */
char * ReplaceGLFragData(char * source, int * sourceLength){

    // 10 is arbitrary, but I don't expect the shader to use so many
    // TODO I guess the array could be accessed with one or more spaces :think:
    // TODO wait they can access via a variable !
    for (int i = 0; i < 10; ++i) {
        // Check for 2 forms on the glFragData and take the first one found
        char needle[30];
        sprintf(needle, "gl_FragData[%i]", i);

        // Skip if the draw buffer isn't used at this index
        char * useFragData = strstr(source, &needle[0]);
        if(useFragData == NULL){
            sprintf(needle, "gl_FragData[int(%i.0)]", i);
            useFragData = strstr(source, &needle[0]);
            if(useFragData == NULL) continue;
        }

        // Construct replacement string
        char replacement[20];
        char replacementLine[70];
        sprintf(replacement, "vgpu_FragData%i", i);
        sprintf(replacementLine, "\nlayout(location = %i) out mediump vec4 %s;\n", i, replacement);
        int insertPoint = FindPositionAfterDirectives(source);

        // And place them into the shader
        source = InplaceReplaceSimple(source, sourceLength, &needle[0], &replacement[0]);
        source = InplaceInsertByIndex(source, sourceLength, insertPoint + 1, &replacementLine[0]);
    }
    return source;
}

/**
 * Replace the gl_FragColor
 * @param source The shader as a string
 * @return The shader a a string, maybe in a different memory location
 */
char * ReplaceGLFragColor(char * source, int * sourceLength){
    if(strstr(source, "gl_FragColor")){
        source = InplaceReplaceSimple(source, sourceLength, "gl_FragColor", "vgpu_FragColor");
        int insertPoint = FindPositionAfterDirectives(source);
        source = InplaceInsertByIndex(source, sourceLength, insertPoint + 1, "out mediump vec4 vgpu_FragColor;\n");
    }
    return source;
}

/**
 * Remove all extensions right now by replacing them with spaces
 * @param source The shader as a string
 * @return The shader as a string, maybe in a different memory location
 */
char * RemoveUnsupportedExtensions(char * source){
    //TODO remove only specific extensions ?
    for(char * extensionPtr = strstr(source, "#extension "); extensionPtr; extensionPtr = strstr(source, "#extension ")){
        int i = 0;
        while(extensionPtr[i] != '\n'){
            extensionPtr[i] = ' ';
            ++i;
        }
    }
    return source;
}

/**
 * Replace the variable name in a shader, mostly used to avoid keyword clashing
 * @param source The shader as a string
 * @param initialName The initial name for the variable
 * @param newName The new name for the variable
 * @return The shader as a string, maybe in a different memory location
 */
char * ReplaceVariableName(char * source, int * sourceLength, char * initialName, char* newName) {

    char * toReplace = (char *)malloc(strlen(initialName) + 3);
    char * replacement = (char *)malloc(strlen(newName) + 3);
    char * charBefore = "{}([];+-*/~!%<>,&| \n\t";
    char * charAfter = ")[];+-*/%<>;,|&. \n\t";

    // Prepare the fixed part of the strings
    strcpy(toReplace+1, initialName);
    toReplace[strlen(initialName)+2] = '\0';

    strcpy(replacement+1, newName);
    replacement[strlen(newName)+2] = '\0';

    for (int i = 0; i < strlen(charBefore); ++i) {
        for (int j = 0; j < strlen(charAfter); ++j) {
            // Prepare the string to replace
            toReplace[0] = charBefore[i];
            toReplace[strlen(initialName)+1] = charAfter[j];

            // Prepare the replacement string
            replacement[0] = charBefore[i];
            replacement[strlen(newName)+1] = charAfter[j];

            source = InplaceReplaceSimple(source, sourceLength, toReplace, replacement);
        }
    }

    free(toReplace);
    free(replacement);

    return source;
}

/**
 * Replace a function definition and calls to the function to another name
 * @param source The shader as a string
 * @param sourceLength The shader length
 * @param initialName The name be to changed
 * @param finalName The name to use instead
 * @return The shader as a string, maybe in a different memory location
 */
char * ReplaceFunctionName(char * source, int * sourceLength, char * initialName, char * finalName){
    for(unsigned long currentPosition = 0; 1; currentPosition += strlen(initialName)){
        unsigned long newPosition = strstrPos(source + currentPosition, initialName);
        if(newPosition == 0) // No more calls
            break;

        // Check if that is indeed a function call on the right side
        if (source[GetNextTokenPosition(source, currentPosition + newPosition + strlen(initialName), '(', " \n\t")] != '('){
            currentPosition += newPosition;
            continue; // Skip to the next potential call
        }

        // Check the naming on the left side
        if (isValidFunctionName(source[currentPosition + newPosition - 1])){
            currentPosition += newPosition;
            continue; // Skip to the next potential call
        }

        // This is a valid function call/definition, replace it
        source = InplaceReplaceByIndex(source, sourceLength, currentPosition + newPosition, currentPosition + newPosition + strlen(initialName) - 1, finalName);
        currentPosition += newPosition;
    }
    return source;
}

/** Remove 'const ' storage qualifier from variables inside {..} blocks
 * @param source The pointer to the start of the shader */
char * RemoveConstInsideBlocks(char* source, int * sourceLength){
    int insideBlock = 0;
    char * keyword = "const \0";
    int keywordLength = strlen(keyword);


    for(int i=0; i< *sourceLength+1; ++i){
        // Step 1, look for a block
        if(source[i] == '{'){
            insideBlock += 1;
            continue;
        }
        if(!insideBlock) continue;

        // A block has been found, look for the keyword
        if(source[i] == keyword[0]){
            int keywordMatch = 1;
            int j = 1;
            while (j < keywordLength){
                if (source[i+j] != keyword[j]){
                    keywordMatch = 0;
                    break;
                }
                j +=1;
            }
            // A match is found, remove it
            if(keywordMatch){
                source = InplaceReplaceByIndex(source, sourceLength, i, i+j - 1, " ");
                continue;
            }
        }

        // Look for an exit
        if(source[i] == '}'){
            insideBlock -= 1;
            continue;
        }
    }
    return source;
}

/** Find the first point which is right after most shader directives
 * @param source The shader as a string
 * @return The index position after the #version line, start of the shader if not found
 */
int FindPositionAfterDirectives(char * source){
    const char * position = FindString(source, "#version");
    if (position == NULL) return 0;
    for(int i=7; 1; ++i){
        if(position[i] == '\n'){
            if(position[i+1] == '#') continue; // a directive is present right after, skip
            return i;
        }
    }
}

int FindPositionAfterVersion(char * source){
    const char * position = FindString(source, "#version");
    if (position == NULL) return 0;
    for(int i=7; 1; ++i){
        if(position[i] == '\n'){
            return i;
        }
    }
}

/**
 * Replace and inserts precision qualifiers as necessary, see LIBGL_VGPU_PRECISION
 * @param source The shader as a string
 * @param sourceLength The length of the string
 * @return The shader as a string, maybe in a different memory location
 */
char * ReplacePrecisionQualifiers(char * source, int * sourceLength, int isVertex){

    if(!doesShaderVersionContainsES(source)){
        LOG_D("\nSKIPPING the replacement qualifiers step\n");
        
        return source;
    }

    // Step 1 is to remove any "precision" qualifiers
    for(unsigned long currentPosition=strstrPos(source, "precision "); currentPosition>0;currentPosition=strstrPos(source, "precision ")){
        // Once a qualifier is found, get to the end of the instruction and replace
        int endPosition = GetNextTokenPosition(source, currentPosition, ';', "");
        source = InplaceReplaceByIndex(source, sourceLength, currentPosition, endPosition,"");
    }

    // Step 2 is to insert precision qualifiers, even the ones we think are defaults, since there are defaults only for some types

    int insertPoint = FindPositionAfterDirectives(source);
    source = InplaceInsertByIndex(source, sourceLength, insertPoint,
                                  "\nprecision lowp sampler2D;\n"
                                  "precision lowp sampler3D;\n"
                                  "precision lowp sampler2DShadow;\n"
                                  "precision lowp samplerCubeShadow;\n"
                                  "precision lowp sampler2DArray;\n"
                                  "precision lowp sampler2DArrayShadow;\n"
                                  "precision lowp samplerCube;\n"
                                  "#ifdef GL_EXT_texture_buffer\n"
                                  "precision lowp samplerBuffer;\n"
                                  "precision lowp imageBuffer;\n"
                                  "#endif\n"
                                  "#ifdef GL_EXT_texture_cube_map_array\n"
                                  "precision lowp imageCubeArray;\n"
                                  "precision lowp samplerCubeArray;\n"
                                  "precision lowp samplerCubeArrayShadow;\n"
                                  "#endif\n"
                                  "#ifdef GL_OES_texture_storage_multisample_2d_array\n"
                                  "precision lowp sampler2DMS;\n"
                                  "precision lowp sampler2DMSArray;\n"
                                  "#endif\n");

    if(GetShaderVersion(source) > 300){
        source = InplaceInsertByIndex(source, sourceLength,insertPoint,
                                      "\nprecision lowp image2D;\n"
                                      "precision lowp image2DArray;\n"
                                      "precision lowp image3D;\n"
                                      "precision lowp imageCube;\n");
    }
    int supportHighp = 1;
    source = InplaceInsertByIndex(source, sourceLength, insertPoint, supportHighp ? "\nprecision highp float;\n" : "\nprecision medium float;\n");

#define vgpu_precision 1
    if (vgpu_precision != 0){
        char * target_precision;
        switch (vgpu_precision) {
            case 1: target_precision = "highp"; break;
            case 2: target_precision = "mediump"; break;
            case 3: target_precision = "lowp"; break;
            default: target_precision = "highp";
        }
        source = ReplaceVariableName(source, sourceLength, "highp", target_precision);
        source = ReplaceVariableName(source, sourceLength, "mediump", target_precision);
        source = ReplaceVariableName(source, sourceLength, "lowp", target_precision);
    }

    return source;
}

/**
 * @param openingToken The opening token
 * @return All closing tokens, if available
 */
char * GetClosingTokens(char openingToken){
    switch (openingToken) {
        case '(': return ")";
        case '[': return "]";
        case ',': return ",)";
        case '{': return "}";
        case ';': return ";";

        default: return "";
    }
}

/**
 * @param openingToken The opening token
 * @return Whether the token is an opening token
 */
int isOpeningToken(char openingToken){
    return openingToken != ',' && strlen(GetClosingTokens(openingToken)) != 0;
}

int GetClosingTokenPosition(const char * source, int initialTokenPosition){
    return GetClosingTokenPositionTokenOverride(source, initialTokenPosition, source[initialTokenPosition]);
}

/**
 * Get the index of the closing token within a string, same as initialTokenPosition if not found
 * @param source The string to look into
 * @param initialTokenPosition The opening token position
 * @return The closing token position
 */
int GetClosingTokenPositionTokenOverride(const char * source, int initialTokenPosition, char initialToken){
    // Step 1: Determine the closing token
    char openingToken = initialToken;
    char * closingTokens = GetClosingTokens(openingToken);
    printf("Closing tokens: %s", closingTokens);
    if (strlen(closingTokens) == 0){
        printf("No closing tokens, somehow \n");
        return initialTokenPosition;
    }

    // Step 2: Go through the string to find what we want
    for(int i=initialTokenPosition+1; i<strlen(source); ++i){
        // Loop though all the available closing tokens first, since opening/closing tokens can be identical
        for(int j=0; j<strlen(closingTokens); ++j){
            if (source[i] == closingTokens[j]){
                return i;
            }
        }

        if (isOpeningToken(source[i])){
            i = GetClosingTokenPosition(source, i);
            continue;
        }
    }
    printf("No closing tokens 2 , somehow \n");
    return initialTokenPosition; // Nothing found
}



/**
 * Return the position of the first token corresponding to what we want
 * @param source The source string
 * @param initialPosition The starting position to look from
 * @param token The token you want to find
 * @param acceptedChars All chars we can go over without tripping. Empty means all chars are allowed.
 * @return
 */
int GetNextTokenPosition(const char * source, int initialPosition, const char token, const char * acceptedChars){
    for(int i=initialPosition+1; i< strlen(source); ++i){
        // Tripping check
        if(strlen(acceptedChars) > 0){
            for(int j=0; j< strlen(acceptedChars); ++j){
                if (source[i] == acceptedChars[j]) break; // No tripping, continue
            }
            return initialPosition; // Tripped, meaning the token is not found
        }

        if (source[i] == token){
            return i;
        }
    }
    return initialPosition;
}

/**
 * @param haystack
 * @param needle
 * @return The position of the first occurence of the needle in the haystack
 */
unsigned long strstrPos(const char * haystack, const char * needle){
    const char *substr = strstr(haystack, needle);
    if (substr == NULL) return 0;
    return (substr - haystack);
}

/**
 * Inserts int(...) on a specific argument from each call to the function
 * @param source The shader as a string
 * @param functionName The name of the function to manipulate
 * @param argumentPosition The position of the argument to manipulate, from 0. If not found, no changes are made.
 * @return The shader as a string, maybe in a different memory location
 */
char * insertIntAtFunctionCall(char * source, int * sourceSize, const char * functionName, int argumentPosition){
    //TODO a less naive function for edge-cases ?
    unsigned long functionCallPosition = strstrPos(source, functionName);
    while(functionCallPosition != 0){
        int openingTokenPosition = GetNextTokenPosition(source, functionCallPosition + strlen(functionName), '(', " \n\r\t");
        if (source[openingTokenPosition] == '('){
            // Function call found, determine the start and end of the argument
            int endArgPos = openingTokenPosition;
            int startArgPos = openingTokenPosition;

            // Note the additional check to see we aren't at the end of a function
            for(int argCount=0; argCount<=argumentPosition && source[startArgPos] != ')'; ++argCount){
                endArgPos = GetClosingTokenPositionTokenOverride(source, endArgPos, ',');
                if (argCount == argumentPosition){
                    // Argument found, insert the int(...)
                    source = InplaceInsertByIndex(source, sourceSize, endArgPos+1, ")");
                    source = InplaceInsertByIndex(source, sourceSize, startArgPos+1, "int(");
                    break;
                }
                // Not the arg we want, got to the next one
                startArgPos = endArgPos;
            }
        }

        // Get the next function call
        unsigned long offset = strstrPos(source + functionCallPosition + strlen(functionName), functionName);
        if (offset == 0) break; // No more function calls
        functionCallPosition += offset + strlen(functionName);
    }
    return source;
}

/**
 * @param source The shader as a string
 * @return The shader version: eg. 310 for #version 310 es
 */
int GetShaderVersion(const char * source){
    // Oh yeah, I won't care much about this function
    if(FindString(source, "#version 320 es")){return 320;}
    if(FindString(source, "#version 310 es")){return 310;}
    if(FindString(source, "#version 300 es")){return 300;}
    if(FindString(source, "#version 150")){return 150;}
    if(FindString(source, "#version 130")){return 130;}
    if(FindString(source, "#version 120")){return 120;}
    return 100;
}