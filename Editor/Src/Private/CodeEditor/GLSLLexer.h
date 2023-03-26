#ifndef GLSLLEXER_H
#define GLSLLEXER_H

#include <Qsci/qscilexercpp.h>

class QscilexerGLSL : public QsciLexerCPP
{
    Q_OBJECT
public:
    QscilexerGLSL(){
        this->setDefaultColor(QColor(10,10,10));
        setColor(QColor(0,90,0),QsciLexerCPP::KeywordSet2);
        setColor(QColor(3,0,255),QsciLexerCPP::Keyword);
        setColor(QColor(50,50,50),QsciLexerCPP::Number);
        setColor(QColor(20,20,20),QsciLexerCPP::Operator);
        setColor(QColor(150,0,0),QsciLexerCPP::GlobalClass);
        setFont(this->font(0),QsciLexerCPP::Keyword);
        setFont(this->font(0),QsciLexerCPP::Operator);
        setColor(Qt::red,QsciLexerCPP::TaskMarker);
    }
    virtual const char *keywords(int set) const override{
        if(set == 1)        //数据类型
            return "bool break bvec2 bvec3 bvec4 centroid const continue discard "
                   "do else false float for if inout int invariant ivec2 ivec3 "
                   "ivec4 mat2 mat2x2 mat2x3 mat2x4 mat3 mat3x2 mat3x3 mat3x4 mat4 "
                   "mat4x2 mat4x3 mat4x4 return sampler1D sampler1DShadow sampler2D "
                   "sampler2DShadow sampler3D samplerCube struct true "
                   "vec2 vec3 vec4 void while precision mediump";
        if(set == 2)        //函数
            return "abs acos all any asin atan ceil clamp cos cross degrees dFdx "
                   "dFdy distance dot equal exp exp2 faceforward floor fract "
                   "fwidth greaterThan greaterThanEqual inversesqrt length lessThan "
                   "lessThanEqual log log2 main max min mix mod noise1noise2 noise3 "
                   "noise4 normalize not notEqual outProduct pow radians reflect "
                   "refract shadow1D shadow2D sign sin smoothstep sqrt step tan "
                   "texture1D texture2D texture3D textureCube transpose "
                   "EmitVertex() EndPrimitive()";

        if(set==4)
            return "gl_FragColor "
                   "gl_FragCoord "
                   "gl_FragDepth "
                   "gl_VertexID "
                   "uniform "
                   "in "
                   "out "
                   "varing ";

        return 0;
    }
    QStringList functions(){
        QStringList list;
        list<<"radians(genFType degrees)->genFType：将度转换为弧度，即 （π/ 180）·度。"
        <<"degrees(genFType radians)->genFType:将弧度转换为度数，即 （180 /π）·弧度。"
        <<"sin(genFType angle)->genFType:标准三角正弦函数。"
        <<"cos(genFType angle)->genFType:标准三角余弦函数。"
        <<"tan(genFType angle)->genFType:标准三角正切函数。"
        <<"asin(genFType x)->genFType：反正弦。返回正弦为x的角度。此函数返回的值范围为 [-π/ 2，π/ 2]。如果|x|，结果是不确定的 > 1。"
        <<"acos(genFType x)->genFType：反余弦。返回余弦为x的角度。此函数返回的值范围是[0，π]。如果|x|，结果是不确定的 > 1。"
        <<"atan(genFType y, genFType x)->genFType：圆弧切线。返回切线为y / x的角度。x和y的符号用于确定角度在哪个象限中。此函数返回的值范围为[-π，π]。如果x和y均为0，则结果不确定。"
        <<"atan(genFType y_over_x)->genFType：圆弧切线。返回切线为y_over_x的角度。此函数返回的值范围为 [-π/ 2，π/ 2]。"
        <<"sinh(genFType x)->genFType：返回双曲正弦函数（exp(x) - exp(-x)）/ 2。"
        <<"cosh(genFType x)->genFType：返回双曲余弦函数（exp(x) + exp(-x)）/ 2。"
        <<"tanh(genFType x)->genFType：返回双曲正切函数sinh（x）/ cosh（x）。"
        <<"asinh(genFType x)->genFType：弧双曲正弦；返回sinh的倒数。"
        <<"acosh(genFType x)->genFType：弧双曲余弦；返回cosh的非负逆。如果x <1，则结果不确定。"
        <<"atanh(genFType x)->genFType：弧双曲正切；返回tanh的倒数。，如果X ≥ 1，结果是不确定的。"
        <<"pow(genFType x, genFType y)->genFType：返回x的y次幂，即x y。如果x <0，则结果不确定。结果是不确定的，如果X = 0和ÿ≤0 。"
        <<"exp(genFType x)->genFType：返回x的自然指数。"
        <<"log(genFType x)->genFType：返回的自然对数X，即，返回值ÿ 满足式X = E ÿ。结果是不确定的，如果X≤0 。"
        <<"exp2(genFType x)->genFType：返回2的x次幂，即pow(2,x)。"
        <<"log2(genFType x)->genFType：返回的基体2的对数X"
        <<"sqrt(genFType x)->genFType：返回sqrt（x）。如果x <0，则结果不确定。"
        <<"inversesqrt(genFType x)->genFType：返回1 / sqrt（x）。结果是不确定的，如果X≤0 。"
        <<"abs(genFType x)->genFType：取绝对值；如果x≥0，则返回x；否则返回-x。"
        <<"sign(genFType x)->genFType：取符号；如果x>0，返回1.0，反之返回-1.0。"
        <<"floor(genFType x)->genFType：舍弃小数取整数"
        <<"trunc(genFType x)->genFType：返回一个值等于到最接近的整数X，其绝对值不大于X的绝对值。"
        <<"round(genFType x)->genFType：四舍五入。"
        <<"roundEven(genFType x)->genFType：返回等于x的最接近整数的值。0.5的小数部分将舍入到最接近的偶数整数。（x的3.5和4.5都将返回4.0。）"
        <<"ceil(genFType x)->genFType：返回等于或大于x的最接近整数的值"
        <<"fract(genFType x)->genFType：返回X-floor（X）。"
        <<"mod(genFType x, float y)->genFType：取模；返回x-y*floor（x / y）。"
        <<"modf(genFType x, out genFType i)->genFType：返回x的小数部分，并将i设置为整数部分（作为整数浮点值）。返回值和输出参数都将具有与x相同的符号。"
        <<"min(genFType x, genFType y)->genFType：取最小值。"
        <<"max(genFType x, genFType y)->genFType：取最大值。"
        <<"clamp(genFType X，genFType MINVAL，genFType MAXVAL)->genFType：返回min（max（x（minVal），maxVal）。如果minVal > maxVal，则结果不确定。"
        <<"mix(genFType x, genFType y, genFType a)->genFType：返回x和y的线性混合，即 x *（1-a）+ y*a。"
        <<"step(genFType edge, genFType x)->genFType：如果x < edge，则返回0.0 ; 否则返回1.0。"
        <<"smoothstep(genFType edge0, genFType edge1, genFType x)->genFType：返回0.0如果X≤EDGE0和1.0如果X≥EDGE1，并进行平滑0和1之间时Hermite插值EDGE0 <X <EDGE1。"
        <<"isnan(genFType x)->genFType：如果x保留NaN，则返回true。否则返回false。如果未实现NaN，则始终返回false。"
        <<"isinf(genFType x)->genFType：如果x保持正无穷大或负无穷大，则返回true。否则返回false。"
        <<"fma(genFType a, genFType b, genFType c)->genFType：计算并返回a * b + c。在返回值最终被声明为precision的变量使用的使用中"
        <<"length(genFType x)->float：返回向量x的长度，即 sqrt(x0*x0 + x1*x1 + …)"
        <<"distance(genFType p0, genFType p1)->float：返回p0和p1之间的距离，即 length(p0 - p1)"
        <<"dot(genFType x, genFType y)->float：返回x和y的点积，即 x0·y0 + x1·y1 +…"
        <<"cross(vec3 x, vec3 y)->vec3：返回x和y的叉积"
        <<"normalize(genFType x)->genFType：归一化；返回与x方向相同但长度为1的向量，即 x/length(x) 。"
        <<"faceforward(genFType N, genFType I, genFType Nref)->genFType：如果点（Nref，I）<0 ，则返回N，否则返回-N。"
        <<"reflect(genFType I, genFType N)->genFType：对于入射向量I和表面方向N，返回反射方向：I-2·点（N，I）·N。 N必须已经标准化才能获得所需的结果。"
        <<"refract(genFType I, genFType N, float eta)->genFType：对于入射向量I和表面法线N，以及折射率之比eta，返回折射向量。"
        <<"textureSize(gsampler1D sampler, int lod)->int"
        <<"textureSize(gsampler2D sampler, int lod)->ivec2"
        <<"textureSize(gsampler3D sampler, int lod)->ivec3"
        <<"texture(sampler1D sampler, float coord [, float bias] )->vec4"
        <<"texture(sampler2D sampler, vec2 coord [, float bias] )->vec4"
        <<"texture(sampler3D sampler, vec3 coord [, float bias] )->vec4";
        return list;
    }
};

#endif // GLSLLEXER_H
