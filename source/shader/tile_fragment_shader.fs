#version 330

#define PI 3.14159265358979323
#define INV_PI 0.31830988618379067
#define TWO_PI 6.28318530717958648
#define INV_TWO_PI 0.15915494309189533
#define INV_4_PI 0.07957747154594766
#define EPS 0.0003
#define INF 1000000000.0

#define QUAD_LIGHT 0
#define SPHERE_LIGHT 1
#define DISTANT_LIGHT 2

#define MEDIUM_NONE 0
#define MEDIUM_ABSORB 1
#define MEDIUM_SCATTER 2
#define MEDIUM_EMISSIVE 3

out vec4 FragColor;

in vec2 TexCoords;

// 摄像机
struct Camera
{
    vec3 up;// 相机的“上”方向（垂直方向），用于确定视图矩阵
    vec3 right;// 相机的“右”方向（水平方向），用于确定视图矩阵
    vec3 forward;// 相机的前进方向，决定相机的朝向
    vec3 position;// 相机的位置
    float fov;// 相机的视场角（field of view）
    float focalDist;// 相机的焦距（影响景深效果）
    float aperture;// 相机的光圈大小（影响景深和模糊效果）
};

struct Ray
{
    vec3 origin;// 光线的起点
    vec3 direction;// 光线的方向
};

struct Light
{
    vec3 position;
    vec3 emission;
    vec3 u;
    vec3 v;
    float radius;
    float area;
    float type;
};

struct Medium
{
    int type;
    float density;
    vec3 color;
    float anisotropy;
};

struct Material
{
    vec3 baseColor;          // 基础颜色，用于定义材质的主要颜色
    float opacity;           // 不透明度，0 表示完全透明，1 表示完全不透明
    int alphaMode;           // 透明度模式，通常表示混合模式（如 OPAQUE、MASK、BLEND 等）
    float alphaCutoff;       // 透明度裁剪阈值，主要用于 MASK 模式（低于该值的像素会被裁剪）
    vec3 emission;           // 自发光颜色，用于定义材质的自发光效果
    float anisotropic;       // 各向异性程度，控制材质的各向异性反射
    float metallic;          // 金属度，0 表示完全非金属，1 表示完全金属
    float roughness;         // 粗糙度，控制表面的光滑程度
    float subsurface;        // 次表面散射强度，用于模拟半透明材质的光透过效果
    float specularTint;      // 镜面光的颜色调节强度
    float sheen;             // 光泽效果强度，用于模拟织物等材质表面的微光效果
    float sheenTint;         // 光泽颜色调节，用于改变光泽的颜色
    float clearcoat;         // 清漆层强度，用于模拟双层材质的效果
    float clearcoatRoughness;// 清漆层的粗糙度，控制清漆的光滑程度
    float specTrans;         // 光学透明度，控制材质的光学透过性
    float ior;               // 折射率（Index of Refraction），决定光线穿过材质的折射行为
    float ax;                // 各向异性参数（X 方向），控制表面粗糙度的方向性
    float ay;                // 各向异性参数（Y 方向），控制表面粗糙度的方向性
    Medium medium;           // 材质内部的介质属性，用于控制光线在材质内部的传播行为
};

struct State
{
    int depth;               // 当前光线递归深度，用于跟踪反射或折射的次数
    float eta;               // 当前介质的折射率，用于计算折射行为
    float hitDist;           // 光线与物体的交点距离（光线参数 t 值）

    vec3 fhp;                // 首次相交点（First Hit Point），即光线与物体的交点
    vec3 normal;             // 表面法线（可能是未修正的法线，方向未必一致）
    vec3 ffnormal;           // 面向外部的法线（正面法线，确保方向总是朝向光线进入的外部）
    vec3 tangent;            // 切线方向，用于构建局部坐标系
    vec3 bitangent;          // 副切线方向，与法线和切线一起构建局部坐标系

    bool isEmitter;          // 是否为发光体，`true` 表示该物体会发光

    vec2 texCoord;           // 纹理坐标，用于采样纹理的 UV 坐标
    int matID;               // 材质 ID，用于标识物体使用的材质
    Material mat;            // 物体的材质属性，包含颜色、粗糙度等细节
    Medium medium;           // 介质属性，描述光线在当前环境中的传播行为
};

// 记录散射采样的结果
struct ScatterSampleRec
{
    vec3 L;         // 出射方向的单位向量
    vec3 f;         // BRDF（双向反射分布函数）值或相关散射函数的乘积
    float pdf;      // 该散射方向的概率密度函数值
};

// 记录光源采样的结果
struct LightSampleRec
{
    vec3 normal;    // 光源表面的法向量
    vec3 emission;  // 光源的辐射亮度值
    vec3 direction; // 从交点指向光源的单位方向向量
    float dist;     // 交点与光源之间的距离
    float pdf;      // 采样到该光源的概率密度函数值
};

float RectIntersect(in vec3 pos,in vec3 u,in vec3 v,in vec4 plane,in Ray r);
float SphereIntersect(float radius,vec3 center,in Ray r);
float AABBIntersect(vec3 minCorner,vec3 maxCorner,in Ray r);
bool AnyHit(Ray r, float maxDist);
bool ClosestHit(Ray r,inout State state,inout LightSampleRec lightSample);
void GetMaterial(inout State state, in Ray r);
vec4 PathTrace(Ray r);

float PowerHeuristic(float a, float b); // 幂次启发式
vec3 DirectLight(in Ray r, in State state, bool isSurface);

void SampleOneLight(in Light light, in vec3 scatterPos, inout LightSampleRec lightSample);
void SampleSphereLight(in Light light, in vec3 scatterPos, inout LightSampleRec lightSample);
void SampleRectLight(in Light light, in vec3 scatterPos, inout LightSampleRec lightSample);
void SampleDistantLight(in Light light, in vec3 scatterPos, inout LightSampleRec lightSample);
vec3 UniformSampleHemisphere(float r1, float r2);
vec3 SampleHG(vec3 V, float g, float r1, float r2);
float PhaseHG(float cosTheta, float g);
vec3 CosineSampleHemisphere(float r1, float r2);
vec3 SampleGGXVNDF(vec3 V, float ax, float ay, float r1, float r2);
vec3 SampleGTR1(float rgh, float r1, float r2);

vec3 DisneySample(State state, vec3 V, vec3 N, out vec3 L, out float pdf);
vec3 DisneyEval(State state, vec3 V, vec3 N, vec3 L, out float pdf);
vec3 EvalDisneyDiffuse(Material mat, vec3 Csheen, vec3 V, vec3 L, vec3 H, out float pdf);
vec3 EvalMicrofacetReflection(Material mat, vec3 V, vec3 L, vec3 H, vec3 F, out float pdf);
vec3 EvalMicrofacetRefraction(Material mat, float eta, vec3 V, vec3 L, vec3 H, vec3 F, out float pdf);

uvec4 seed;
ivec2 pixel;
void InitRNG(vec2 p,int frame);
void pcg4d(inout uvec4 v);
float rand();

uniform int topBVHIndex;
uniform samplerBuffer BVH;
uniform samplerBuffer verticesTex;
uniform samplerBuffer normalsTex;
uniform isamplerBuffer vertexIndicesTex;
uniform sampler2D transformsTex;
uniform sampler2D materialsTex;
uniform sampler2D lightsTex;

uniform int numOfLights;

uniform vec2 resolution;
uniform Camera camera;

void main()
{
    // 屏幕坐标直接映射生成光线
    vec2 d = (TexCoords * 2.0 - 1.0);// 将坐标范围映射到 [-1, 1]
    float scale = tan(camera.fov * 0.5);
    d.y *= resolution.y / resolution.x*scale;
    d.x *= scale;
    vec3 rayDir = normalize(d.x * camera.right + d.y * camera.up + camera.forward);
    Ray ray = Ray(camera.position, rayDir);
    
    vec4 pixelColor = PathTrace(ray);
    FragColor = vec4(pixelColor);
}

/********************************/
// 进行路径追踪计算
/********************************/
vec4 PathTrace(Ray r)
{
    vec3 radiance = vec3(0.0);   // 最终的辐射贡献 -- 颜色
    vec3 throughput = vec3(1.0); // 路径的通量权重（每次交互更新光的贡献）
    State state;                   // 保存当前光线状态（碰撞信息、深度等）
    LightSampleRec lightSample;          // 光源采样记录
    ScatterSampleRec scatterSample;      // 散射采样记录
    
    float alpha = 1.0;  // 透明度 -- 用于材质的遮挡和半透明效果
    
    bool inMedium = false;       // 是否在介质中（如雾或水中）
    bool mediumSampled = false;  // 是否在介质中采样到了散射点
    bool surfaceScatter = false; // 是否发生了表面散射
    
    int maxDepth = 10;
    int RRMaxDepth = 5;
    
    // 路径追踪循环
    for(state.depth = 0; state.depth < maxDepth; state.depth++)
    {
        // 计算光线与场景的最近交点，如果没有击中任何物体，说明光线射到了背景或者环境光
        bool hit = ClosestHit(r, state, lightSample);
        
        if(hit)
        {
            // 获取交点物体的材质信息
            GetMaterial(state, r);
            
            // 计算来自发光物体的辐射
            radiance += state.mat.emission * throughput;
            
            // 如果命中的物体时发光体/光源，则计算该发光体的辐射贡献并终止路径追踪
            if(state.isEmitter)
            {
                float misWeight = (state.depth > 0) ? PowerHeuristic(scatterSample.pdf, lightSample.pdf) : 1.0;
                
                radiance += misWeight * lightSample.emission * throughput;

                break;
            }

            if(inMedium)
            {
                // 处理介质吸收：如果介质类型是吸收（MEDIUM_ABSORB）
                if(state.medium.type == MEDIUM_ABSORB)
                {
                    // 计算透射率，并更新光强度（通过指数衰减模型）
                    throughput *= exp(-(1.0 - state.medium.color) * state.hitDist * state.medium.density);
                }
                // 处理介质发射：如果介质类型是发光（MEDIUM_EMISSIVE）
                else if(state.medium.type == MEDIUM_EMISSIVE)
                {
                    // 如果是发光介质，增加辐射量（根据发射强度、距离和介质密度计算）
                    radiance += state.medium.color * state.hitDist * state.medium.density * throughput;
                }
                else
                {
                    // 如果是散射介质，进行散射过程的采样
                    float scatterDist = min(-log(rand()) / state.medium.density, state.hitDist);
                    mediumSampled = scatterDist < state.hitDist; // 判断是否发生了散射
                    
                    if (mediumSampled)
                    {
                        // 如果发生散射，更新光线强度并调整光线的传播位置
                        throughput *= state.medium.color;
                        r.origin += r.direction * scatterDist;
                        state.fhp = r.origin; // 设置光线的当前位置（散射点）

                        // 计算从散射点开始的直接光照贡献
                        radiance += DirectLight(r, state, false) * throughput;

                        // 基于相函数采样新的散射方向
                        vec3 scatterDir = SampleHG(-r.direction, state.medium.anisotropy, rand(), rand());
                        scatterSample.pdf = PhaseHG(dot(-r.direction, scatterDir), state.medium.anisotropy);
                        r.direction = scatterDir; // 更新光线的方向
                    }
                }
            }

            if(!mediumSampled)
            {
                // 表示发生了表面散射
                surfaceScatter = true;

                radiance += DirectLight(r, state, true) * throughput;
                // radiance += vec3(0.0) * throughput;
                // radiance = vec3(0.0706, 0.8588, 0.3725);
                scatterSample.f = DisneySample(state, -r.direction, state.ffnormal, scatterSample.L, scatterSample.pdf); // 散射方向采样
                if (scatterSample.pdf > 0.0)
                    throughput *= scatterSample.f / scatterSample.pdf;
                else
                    break;

                // 更新光线的原点和方向，准备进行下一次反射
                r.direction = scatterSample.L;
                r.origin = state.fhp + r.direction * EPS;
            }
            
            // 俄罗斯轮盘赌(Russian Roulette)优化
            if (state.depth >= RRMaxDepth)
            {
                // 计算当前传播因子的最大分量，用于决定是否继续追踪
                float q = min(max(throughput.x, max(throughput.y, throughput.z)) + 0.001, 0.95);
                if (rand() > q)
                break;
                throughput /= q;  // 根据概率衰减传播因子
            }
        }
        else
        {
            // TODO:环境贴图 -- 暂时使用纯色背景代替


            vec3 backgroundColor = vec3(0.0941, 0.8078, 0.0667);
            radiance += backgroundColor;
            break;
        }
    }
    
    return vec4(radiance, alpha);
}
/********************************/
// 求取交点
/********************************/
bool ClosestHit(Ray r, inout State state, inout LightSampleRec lightSample)
{
    float tMin = INF; // 记录最近的击中时间
    float tTmp;       // 记录每次的击中时间
    
    // 检查是否击中光源
    // TODO
    for (int i = 0; i < numOfLights; i++)
    {
        
    }
    
    // 检查是否击中模型的BVH节点
    int stack[64];  // 用栈模拟递归
    int ptr = 0;
    stack[ptr++] = -1;
    
    int index = topBVHIndex; // BVH节点索引
    float leftHit = 0.0;     // 记录光线与BVH左孩子节点的击中时间
    float rightHit = 0.0;    // 记录光线与BVH右孩子节点的击中时间
    bool BLAS = false;
    
    mat4 transMat;  // 世界坐标转换到局部坐标的转换矩阵
    mat4 transform; // 记录最近相交三角形所在模型的转换矩阵
    ivec3 triID = ivec3(-1);   // 记录相交三角形三个顶点的索引值
    vec3 bary;                 // 记录三角形重心坐标
    vec4 vert0, vert1, vert2;  // 记录相交三角形的三个顶点
    
    // 光线
    Ray rTrans;
    rTrans.origin = r.origin;
    rTrans.direction = r.direction;
    // 使用栈的方式模仿递归
    while (index != -1)
    {
        ivec3 LRLeaf = ivec3(texelFetch(BVH, index * 3 + 2).xyz);
        
        int leftIndex  = int(LRLeaf.x);
        int rightIndex = int(LRLeaf.y);
        // 标识着BVH节点的类型
        // =0:内部节点
        // >0:底层加速结构节点
        // <0:顶层加速结构节点
        int leaf = int(LRLeaf.z);
        
        if(leaf > 0) {
            // 底层加速数据结构的叶节点 BLAS
            for (int i = 0; i < rightIndex; i++)
            {
                // 获取三个顶点
                ivec3 vertIndices = ivec3(texelFetch(vertexIndicesTex, leftIndex + i).xyz);
                vec4 v0 = texelFetch(verticesTex, vertIndices.x);
                vec4 v1 = texelFetch(verticesTex, vertIndices.y);
                vec4 v2 = texelFetch(verticesTex, vertIndices.z);
                // MT算法 -- 光线与三角形的相交计算
                vec3 e0 = v1.xyz - v0.xyz;
                vec3 e1 = v2.xyz - v0.xyz;
                vec3 pv = cross(rTrans.direction, e1);
                float det = dot(e0, pv);
                
                vec3 tv = rTrans.origin - v0.xyz;
                vec3 qv = cross(tv, e0);
                
                vec4 uvt;
                uvt.x = dot(tv, pv);                  // b1
                uvt.y = dot(rTrans.direction, qv);    // b2
                uvt.z = dot(e1, qv);                  // t
                uvt.xyz = uvt.xyz / det;
                uvt.w = 1.0 - uvt.x - uvt.y;          // 1-b1-b2
                // 相交判断
                if (all(greaterThanEqual(uvt, vec4(0.0))) && uvt.z < tMin)
                {
                    tMin = uvt.z;
                    triID = vertIndices;
                    // state.matID = currMatID;
                    bary = uvt.wxy;
                    vert0 = v0;
                    vert1 = v1;
                    vert2 = v2;
                    transform = transMat;
                }
            }
        } 
        else if(leaf < 0) 
        {
            // 顶层加速数据结构的叶节点 TLAS
            // 提取变换矩阵的行数据
            vec4 r1 = texelFetch(transformsTex, ivec2((-leaf - 1) * 4 + 0, 0), 0).xyzw;
            vec4 r2 = texelFetch(transformsTex, ivec2((-leaf - 1) * 4 + 1, 0), 0).xyzw;
            vec4 r3 = texelFetch(transformsTex, ivec2((-leaf - 1) * 4 + 2, 0), 0).xyzw;
            vec4 r4 = texelFetch(transformsTex, ivec2((-leaf - 1) * 4 + 3, 0), 0).xyzw;
            // 构建变换矩阵 -- 将光线从世界坐标系转换到对象的局部坐标系
            transMat = mat4(r1, r2, r3, r4);
            
            rTrans.origin    = vec3(inverse(transMat) * vec4(r.origin, 1.0));
            rTrans.direction = vec3(inverse(transMat) * vec4(r.direction, 0.0));
            
            stack[ptr++] = -1; // 添加一个标记 -- 遍历完BLAS时返回到该位置
            index = leftIndex;
            BLAS = true;
            // currMatID = rightIndex;
            continue;
        } 
        else 
        {
            // 非叶节点
            leftHit  = AABBIntersect(texelFetch(BVH, leftIndex  * 3 + 0).xyz, texelFetch(BVH, leftIndex  * 3 + 1).xyz, rTrans);
            rightHit = AABBIntersect(texelFetch(BVH, rightIndex * 3 + 0).xyz, texelFetch(BVH, rightIndex * 3 + 1).xyz, rTrans);
            
            if (leftHit > 0.0 && rightHit > 0.0)
            {
                index = (leftHit > rightHit) ? rightIndex : leftIndex;
                stack[ptr++] = (leftHit > rightHit) ? leftIndex : rightIndex; // 将较远的包围盒压入栈中
                continue;
            }
            else if (leftHit > 0.)
            {
                index = leftIndex;
                continue;
            }
            else if (rightHit > 0.)
            {
                index = rightIndex;
                continue;
            }
        }
        
        index = stack[--ptr];
        // 当遍历完底层的加速数据结构之后返回到顶层的加速数据结构
        if(BLAS && index == -1)
        {
            BLAS = false;
            index = stack[--ptr];
            rTrans.origin = r.origin;
            rTrans.direction = r.direction;
        }
    }
    
    // 没有相交点
    if (tMin == INF)
    return false;
    
    state.hitDist = tMin;                       // 记录最近的击中时间/距离
    state.fhp = r.origin + r.direction * tMin;  // 记录交点的世界坐标
    
    // 光线击中的是一个三角形而不是光源 -- 光源被遮挡住
    if (triID.x != -1)
    {
        state.isEmitter = false;
        
        vec4 n0 = texelFetch(normalsTex, triID.x);
        vec4 n1 = texelFetch(normalsTex, triID.y);
        vec4 n2 = texelFetch(normalsTex, triID.z);
        // 从顶点坐标数据和法线数据的w分量中提取纹理坐标位置信息
        vec2 t0 = vec2(vert0.w, n0.w);
        vec2 t1 = vec2(vert1.w, n1.w);
        vec2 t2 = vec2(vert2.w, n2.w);
        // 使用重心坐标插值计算交点的纹理坐标和法线
        state.texCoord = t0 * bary.x + t1 * bary.y + t2 * bary.z;
        vec3 normal = normalize(n0.xyz * bary.x + n1.xyz * bary.y + n2.xyz * bary.z);
        // 计算法线在世界坐标系中的方向
        state.normal = normalize(transpose(inverse(mat3(transform))) * normal);
        state.ffnormal = dot(state.normal, r.direction) <= 0.0 ? state.normal : -state.normal;
        // 计算切线和副切线
        vec3 deltaPos1 = vert1.xyz - vert0.xyz;
        vec3 deltaPos2 = vert2.xyz - vert0.xyz;
        vec2 deltaUV1 = t1 - t0;
        vec2 deltaUV2 = t2 - t0;
        float invdet = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        state.tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * invdet;
        state.bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * invdet;
        // 将切线和副切线转换到世界坐标系并归一化
        state.tangent = normalize(mat3(transform) * state.tangent);
        state.bitangent = normalize(mat3(transform) * state.bitangent);
    }
    
    return true;
}
bool AnyHit(Ray r, float maxDist)
{
    // 检查是否击中光源
    // TODO
    for (int i = 0; i < numOfLights; i++)
    {

    }

    int stack[64];  // 用栈模拟递归
    int ptr = 0;
    stack[ptr++] = -1;
    
    int index = topBVHIndex; // BVH节点索引
    float leftHit = 0.0;     // 记录光线与BVH左孩子节点的击中时间
    float rightHit = 0.0;    // 记录光线与BVH右孩子节点的击中时间
    bool BLAS = false;

    Ray rTrans;
    rTrans.origin = r.origin;
    rTrans.direction = r.direction;

    while (index != -1)
    {
        ivec3 LRLeaf = ivec3(texelFetch(BVH, index * 3 + 2).xyz);

        int leftIndex  = int(LRLeaf.x); // 左子节点索引
        int rightIndex = int(LRLeaf.y); // 右子节点索引
        int leaf       = int(LRLeaf.z); // 叶节点标志位

        if (leaf > 0) // Leaf node of BLAS
        {
            // 底层加速数据结构的叶节点 BLAS
            for (int i = 0; i < rightIndex; i++)
            {
                // 获取三个顶点
                ivec3 vertIndices = ivec3(texelFetch(vertexIndicesTex, leftIndex + i).xyz);
                vec4 v0 = texelFetch(verticesTex, vertIndices.x);
                vec4 v1 = texelFetch(verticesTex, vertIndices.y);
                vec4 v2 = texelFetch(verticesTex, vertIndices.z);
                // MT算法 -- 光线与三角形的相交计算
                vec3 e0 = v1.xyz - v0.xyz;
                vec3 e1 = v2.xyz - v0.xyz;
                vec3 pv = cross(rTrans.direction, e1);
                float det = dot(e0, pv);
                
                vec3 tv = rTrans.origin - v0.xyz;
                vec3 qv = cross(tv, e0);
                
                vec4 uvt;
                uvt.x = dot(tv, pv);                  // b1
                uvt.y = dot(rTrans.direction, qv);    // b2
                uvt.z = dot(e1, qv);                  // t
                uvt.xyz = uvt.xyz / det;
                uvt.w = 1.0 - uvt.x - uvt.y;          // 1-b1-b2
                // 相交判断
                if (all(greaterThanEqual(uvt, vec4(0.0))) && uvt.z < maxDist)
                {
                    // TODO 没有考虑透明介质
                    return true;
                }
            }
        }
        else if (leaf < 0) // Leaf node of TLAS
        {
            // 顶层加速数据结构的叶节点 TLAS
            // 提取变换矩阵的行数据
            vec4 r1 = texelFetch(transformsTex, ivec2((-leaf - 1) * 4 + 0, 0), 0).xyzw;
            vec4 r2 = texelFetch(transformsTex, ivec2((-leaf - 1) * 4 + 1, 0), 0).xyzw;
            vec4 r3 = texelFetch(transformsTex, ivec2((-leaf - 1) * 4 + 2, 0), 0).xyzw;
            vec4 r4 = texelFetch(transformsTex, ivec2((-leaf - 1) * 4 + 3, 0), 0).xyzw;
            // 构建变换矩阵 -- 将光线从世界坐标系转换到对象的局部坐标系
            mat4 transform = mat4(r1, r2, r3, r4);
            
            rTrans.origin    = vec3(inverse(transform) * vec4(r.origin, 1.0));
            rTrans.direction = vec3(inverse(transform) * vec4(r.direction, 0.0));
            
            stack[ptr++] = -1; // 添加一个标记 -- 遍历完BLAS时返回到该位置
            index = leftIndex;
            BLAS = true;
            // currMatID = rightIndex;
            continue;
        }
        else
        {
            leftHit =  AABBIntersect(texelFetch(BVH, leftIndex  * 3 + 0).xyz, texelFetch(BVH, leftIndex  * 3 + 1).xyz, rTrans);
            rightHit = AABBIntersect(texelFetch(BVH, rightIndex * 3 + 0).xyz, texelFetch(BVH, rightIndex * 3 + 1).xyz, rTrans);

            if (leftHit > 0.0 && rightHit > 0.0)
            {
                index = (leftHit > rightHit) ? rightIndex : leftIndex;
                stack[ptr++] = (leftHit > rightHit) ? leftIndex : rightIndex; // 将较远的包围盒压入栈中
                continue;
            }
            else if (leftHit > 0.)
            {
                index = leftIndex;
                continue;
            }
            else if (rightHit > 0.)
            {
                index = rightIndex;
                continue;
            }
        }

        index = stack[--ptr];

        if(BLAS && index == -1)
        {
            BLAS = false;
            index = stack[--ptr];
            rTrans.origin = r.origin;
            rTrans.direction = r.direction;
        }
    }

    return false;
}
/********************************/
// 相交函数
/********************************/
float RectIntersect(in vec3 pos, in vec3 u, in vec3 v, in vec4 plane, in Ray r)
{
    // 计算光线与矩形平面相交的时间
    vec3 n = vec3(plane);
    float dt = dot(r.direction, n);
    float t = (plane.w - dot(n, r.origin)) / dt;
    
    if(t > EPS)
    {
        // 判断光线相交点是否在矩形内部
        vec3 p = r.origin + r.direction * t;
        vec3 vi = p - pos;
        float a1 = dot(u, vi);
        if(a1 >= 0.0f && a1 <= 1.0f)
        {
            float a2 = dot(v, vi);
            if(a2 >= 0.0f && a2 <= 1.0f)
            return t;
        }
    }
    
    return INF;
}
float SphereIntersect(float radius, vec3 center, Ray r)
{
    vec3 op = center - r.origin;
    float eps = 0.001;
    float b = dot(op, r.direction);
    float det = b * b - dot(op, op) + radius * radius;
    if (det < 0.0)
    return INF;
    
    det = sqrt(det);
    float t1 = b - det;
    if (t1 > eps)
    return t1;
    
    float t2 = b + det;
    if (t2 > eps)
    return t2;
    
    return INF;
}
float AABBIntersect(vec3 minCorner, vec3 maxCorner, Ray r)
{
    vec3 invDir = 1.0 / r.direction;
    
    vec3 f = (maxCorner - r.origin) * invDir;
    vec3 n = (minCorner - r.origin) * invDir;
    
    vec3 tmax = max(f, n);
    vec3 tmin = min(f, n);
    
    float t1 = min(tmax.x, min(tmax.y, tmax.z)); // 出射时间
    float t0 = max(tmin.x, max(tmin.y, tmin.z)); // 入射时间
    
    return (t1 >= t0) ? (t0 > 0.f ? t0 : t1) : -1.0;
}
/********************************/
// 随机数相关函数
/********************************/
void InitRNG(vec2 p, int frame)
{
    pixel = ivec2(p);
    seed = uvec4(p, uint(frame), uint(p.x) + uint(p.y));
}
void pcg4d(inout uvec4 v)
{
    v = v * 1664525u + 1013904223u;
    v.x += v.y * v.w; v.y += v.z * v.x; v.z += v.x * v.y; v.w += v.y * v.z;
    v = v ^ (v >> 16u);
    v.x += v.y * v.w; v.y += v.z * v.x; v.z += v.x * v.y; v.w += v.y * v.z;
}
float rand()
{
    pcg4d(seed);
    return float(seed.x) / float(0xffffffffu);
}
/********************************/
// 获取材质信息
/********************************/
void GetMaterial(inout State state, in Ray r)
{
    Material mat;

    mat.baseColor = vec3(0.7098, 0.0392, 0.5647);
    mat.emission = vec3(0.0, 0.0, 0.0);

    state.mat = mat;
}
float PowerHeuristic(float a, float b)
{
    float t = a * a;             // 将采样策略 a 的概率密度平方
    return t / (b * b + t);      // 计算 a 的权重
}
vec3 DirectLight(in Ray r, in State state, bool isSurface)
{
    vec3 Ld = vec3(0.0);  // 用于存储直接光照的最终结果
    vec3 Li = vec3(0.0);  // 用于存储来自光源的入射光强度
    vec3 scatterPos = state.fhp + state.normal * EPS;  // 散射位置（略微偏移表面以避免光照计算时的自交）

    ScatterSampleRec scatterSample;  // 存储散射样本数据

    // 解析光源（如点光源、方向光源、矩形光源等）
    if(numOfLights > 0)
    {
        LightSampleRec lightSample;
        Light light;

        // 随机选择一个光源进行采样
        int index = int(rand() * float(numOfLights)) * 5;

        // 获取光源数据
        vec3 position = texelFetch(lightsTex, ivec2(index + 0, 0), 0).xyz;
        vec3 emission = texelFetch(lightsTex, ivec2(index + 1, 0), 0).xyz;
        vec3 u        = texelFetch(lightsTex, ivec2(index + 2, 0), 0).xyz;  // 矩形光源的u向量
        vec3 v        = texelFetch(lightsTex, ivec2(index + 3, 0), 0).xyz;  // 矩形光源的v向量
        vec3 params   = texelFetch(lightsTex, ivec2(index + 4, 0), 0).xyz;
        float radius  = params.x;
        float area    = params.y;
        float type    = params.z;  // 0->矩形光源，1->球形光源，2->远距离光源

        light = Light(position, emission, u, v, radius, area, type);  // 创建光源对象
        SampleOneLight(light, scatterPos, lightSample);  // 对光源进行采样
        Li = lightSample.emission;  // 获取光源的辐射强度

        // 检查光源方向是否朝向表面（必要时处理双面发射的矩形光源）
        if (dot(lightSample.direction, lightSample.normal) < 0.0)  // 如果光源发射方向与表面法线方向相反
        {
            Ray shadowRay = Ray(scatterPos, lightSample.direction);  // 从散射点发射阴影射线

            // 如果场景中没有介质，则使用简单的相交测试
            bool inShadow = AnyHit(shadowRay, lightSample.dist - EPS);  // 检查光源是否被遮挡

            if (!inShadow)
            {
                scatterSample.f = DisneyEval(state, -r.direction, state.ffnormal, lightSample.direction, scatterSample.pdf);  // 表面上的散射

                // 计算MIS权重
                float misWeight = 1.0;
                if(light.area > 0.0)  // 对于远距离光源，没有使用MIS
                    misWeight = PowerHeuristic(lightSample.pdf, scatterSample.pdf);

                if (scatterSample.pdf > 0.0)
                    Ld += misWeight * Li * scatterSample.f / lightSample.pdf;  // 累加光照贡献
            }
        }
    }
    
    return Ld;
}
/********************************/
// 采样函数
/********************************/
void Onb(in vec3 N, inout vec3 T, inout vec3 B)
{
    // 计算给定法线向量 N 的正交基底
    vec3 up = abs(N.z) < 0.9999999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
    T = normalize(cross(up, N));  // 第一个正交向量
    B = cross(N, T);              // 第二个正交向量
}
vec3 UniformSampleHemisphere(float r1, float r2)
{
    // 均匀半球采样
    float r = sqrt(max(0.0, 1.0 - r1 * r1));
    float phi = TWO_PI * r2;
    return vec3(r * cos(phi), r * sin(phi), r1);
}
void SampleOneLight(in Light light, in vec3 scatterPos, inout LightSampleRec lightSample)
{
    int type = int(light.type);

    if (type == QUAD_LIGHT)
        SampleRectLight(light, scatterPos, lightSample);
    else if (type == SPHERE_LIGHT)
        SampleSphereLight(light, scatterPos, lightSample);
    else
        SampleDistantLight(light, scatterPos, lightSample);
}
void SampleRectLight(in Light light, in vec3 scatterPos, inout LightSampleRec lightSample)
{
    // 矩形光源
    float r1 = rand();
    float r2 = rand();

    vec3 lightSurfacePos = light.position + light.u * r1 + light.v * r2; // 在矩形上随机采样一个点
    lightSample.direction = lightSurfacePos - scatterPos;
    lightSample.dist = length(lightSample.direction);
    float distSq = lightSample.dist * lightSample.dist;
    lightSample.direction /= lightSample.dist;
    lightSample.normal = normalize(cross(light.u, light.v));
    lightSample.emission = light.emission * float(numOfLights);
    lightSample.pdf = distSq / (light.area * abs(dot(lightSample.normal, lightSample.direction)));
}
void SampleSphereLight(in Light light, in vec3 scatterPos, inout LightSampleRec lightSample)
{
    // 球形光源
    float r1 = rand();
    float r2 = rand();

    vec3 sphereCentertoSurface = scatterPos - light.position;
    float distToSphereCenter = length(sphereCentertoSurface);
    vec3 sampledDir;

    // TODO: Fix this. Currently assumes the light will be hit only from the outside
    sphereCentertoSurface /= distToSphereCenter;
    // 在均匀球上随机采样一个点
    sampledDir = UniformSampleHemisphere(r1, r2); 
    // 通过 Onb 将半球采样结果从局部坐标系转换到球光源的方向
    vec3 T, B;
    Onb(sphereCentertoSurface, T, B);
    sampledDir = T * sampledDir.x + B * sampledDir.y + sphereCentertoSurface * sampledDir.z;

    vec3 lightSurfacePos = light.position + sampledDir * light.radius; // 计算采样点的位置

    lightSample.direction = lightSurfacePos - scatterPos;
    lightSample.dist = length(lightSample.direction);
    float distSq = lightSample.dist * lightSample.dist;

    lightSample.direction /= lightSample.dist;
    lightSample.normal = normalize(lightSurfacePos - light.position);
    lightSample.emission = light.emission * float(numOfLights);
    lightSample.pdf = distSq / (light.area * 0.5 * abs(dot(lightSample.normal, lightSample.direction)));
}
void SampleDistantLight(in Light light, in vec3 scatterPos, inout LightSampleRec lightSample)
{
    // 平行光源
    lightSample.direction = normalize(light.position - vec3(0.0));
    lightSample.normal = normalize(scatterPos - light.position);
    lightSample.emission = light.emission * float(numOfLights);
    lightSample.dist = INF;
    lightSample.pdf = 1.0;
}
vec3 SampleHG(vec3 V, float g, float r1, float r2)
{
    // 从哈农-格里斯基分布（Henyey-Greenstein distribution）中采样半球上的一个方向
    // 计算cosTheta，表示散射角度的余弦值
    float cosTheta;

    if (abs(g) < 0.001)
        cosTheta = 1 - 2 * r2;
    else 
    {
        // 计算哈农-格里斯基分布的cosTheta值
        float sqrTerm = (1 - g * g) / (1 + g - 2 * g * r2);
        cosTheta = -(1 + g * g - sqrTerm * sqrTerm) / (2 * g);
    }

    // 计算散射角度的方位角（phi）
    float phi = r1 * TWO_PI;  // phi是在0到2π之间的均匀分布角度
    float sinTheta = clamp(sqrt(1.0 - (cosTheta * cosTheta)), 0.0, 1.0);  // 计算sinTheta，保证结果在合理范围内
    float sinPhi = sin(phi);  // 计算方位角的正弦值
    float cosPhi = cos(phi);  // 计算方位角的余弦值

    // 计算正交基（ONB）用于旋转和变换
    vec3 v1, v2;
    Onb(V, v1, v2);  // 使用V作为基准方向，计算两个正交基v1和v2

    // 返回最终采样的方向向量
    return sinTheta * cosPhi * v1 + sinTheta * sinPhi * v2 + cosTheta * V;
}
float PhaseHG(float cosTheta, float g)
{
    float denom = 1 + g * g + 2 * g * cosTheta;
    return INV_4_PI * (1 - g * g) / (denom * sqrt(denom));
}
vec3 CosineSampleHemisphere(float r1, float r2)
{
    vec3 dir;  // 用于存储返回的散射方向

    // 计算在单位圆内的半径
    float r = sqrt(r1);  // r1 是随机数，通过 sqrt(r1) 得到单位圆的半径值

    // 计算散射方向的角度，r2 控制角度的分布
    float phi = TWO_PI * r2;  // phi 是圆周角度，r2 通过乘以 2π 来控制角度

    // 使用极坐标系转换为笛卡尔坐标
    dir.x = r * cos(phi);  // x 分量：单位圆半径与角度的余弦
    dir.y = r * sin(phi);  // y 分量：单位圆半径与角度的正弦

    // 计算 z 分量，确保结果在半球上
    dir.z = sqrt(max(0.0, 1.0 - dir.x * dir.x - dir.y * dir.y));  // 确保 z >= 0，表示半球上的点

    return dir;  // 返回计算出的散射方向
}
vec3 SampleGGXVNDF(vec3 V, float ax, float ay, float r1, float r2)
{
    // 归一化视角向量 V 的斜率成分
    vec3 Vh = normalize(vec3(ax * V.x, ay * V.y, V.z));

    // 计算视向量在切线平面上的分量
    float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
    vec3 T1 = lensq > 0 ? vec3(-Vh.y, Vh.x, 0) * inversesqrt(lensq) : vec3(1, 0, 0);
    vec3 T2 = cross(Vh, T1);  // 法向量与 T1 的叉积，获得第二个正交基向量 T2

    // 使用 r1 和 r2 来生成随机角度
    float r = sqrt(r1);  // r1 用于控制采样半径
    float phi = 2.0 * PI * r2;  // phi 用于控制角度

    // 基于极坐标计算 t1 和 t2
    float t1 = r * cos(phi);
    float t2 = r * sin(phi);

    // s 是与 Vh.z 相关的加权系数
    float s = 0.5 * (1.0 + Vh.z);
    
    // t2 被调整以确保在法线方向上的平滑过渡
    t2 = (1.0 - s) * sqrt(1.0 - t1 * t1) + s * t2;

    // 计算法向量 Nh（在半球上）
    vec3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2)) * Vh;

    // 返回单位化后的法向量，重新转换回原始的空间
    return normalize(vec3(ax * Nh.x, ay * Nh.y, max(0.0, Nh.z)));
}
vec3 SampleGTR1(float rgh, float r1, float r2)
{
    // 确保粗糙度不会过小
    float a = max(0.001, rgh);
    float a2 = a * a; // 计算粗糙度的平方

    // 根据随机数 r1 生成角度 phi
    float phi = r1 * TWO_PI;

    // 根据 r2 和粗糙度计算 cosTheta，控制法线分布的锐度
    float cosTheta = sqrt((1.0 - pow(a2, 1.0 - r2)) / (1.0 - a2));
    
    // 计算 sinTheta，保证三角函数一致性
    float sinTheta = clamp(sqrt(1.0 - (cosTheta * cosTheta)), 0.0, 1.0);
    
    // 根据 phi 计算 sinPhi 和 cosPhi，用于方向计算
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);

    // 返回法向量（微面元法线方向），以世界空间为单位
    return vec3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
}
/********************************/
// BRDF 双向反射分布函数
/********************************/
vec3 ToWorld(vec3 X, vec3 Y, vec3 Z, vec3 V)
{
    return V.x * X + V.y * Y + V.z * Z;
}
vec3 ToLocal(vec3 X, vec3 Y, vec3 Z, vec3 V)
{
    return vec3(dot(V, X), dot(V, Y), dot(V, Z));
}
float Luminance(vec3 c)
{
    // 计算输入颜色 `c` 的亮度值
    return 0.212671 * c.x + 0.715160 * c.y + 0.072169 * c.z;
}
void TintColors(Material mat, float eta, out float F0, out vec3 Csheen, out vec3 Cspec0)
{
    // 计算材质的基础颜色的亮度
    float lum = Luminance(mat.baseColor);
    // 根据亮度调整基础颜色，如果亮度大于0，则对颜色进行归一化，否则保持为白色
    vec3 ctint = lum > 0.0 ? mat.baseColor / lum : vec3(1.0);

    // 使用折射率计算初始反射率 F0，并将其平方以符合物理模型
    F0 = (1.0 - eta) / (1.0 + eta);
    F0 *= F0;
    
    // 计算镜面反射颜色 Cspec0，结合基础颜色和镜面反射色调
    Cspec0 = F0 * mix(vec3(1.0), ctint, mat.specularTint);

    // 计算光泽颜色 Csheen，结合基础颜色和光泽色调
    Csheen = mix(vec3(1.0), ctint, mat.sheenTint);
}
float SchlickWeight(float u)
{
    // 计算基于 Schlick 近似的反射权重值
    float m = clamp(1.0 - u, 0.0, 1.0);
    float m2 = m * m;
    return m2 * m2 * m;
}
float DielectricFresnel(float cosThetaI, float eta)
{
    // 计算介质折射率下的菲涅尔反射值，基于入射角与折射率（η）计算光的反射程度
    // 计算折射角的正弦值平方（用于检测全内反射情况）
    float sinThetaTSq = eta * eta * (1.0f - cosThetaI * cosThetaI);

    // 判断是否发生全内反射：如果折射角的正弦值大于1，则发生全内反射
    if (sinThetaTSq > 1.0)
        return 1.0;  // 完全反射

    // 如果未发生全内反射，计算折射角的余弦值
    float cosThetaT = sqrt(max(1.0 - sinThetaTSq, 0.0));

    // 计算平行和垂直分量的反射系数
    float rs = (eta * cosThetaT - cosThetaI) / (eta * cosThetaT + cosThetaI);  // 平行分量的反射系数
    float rp = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);  // 垂直分量的反射系数

    // 返回平行与垂直分量反射系数的平均值作为菲涅尔反射值
    return 0.5f * (rs * rs + rp * rp);
}
float GTR2Aniso(float NDotH, float HDotX, float HDotY, float ax, float ay)
{
    // 计算与粗糙度方向的标准化点积值
    float a = HDotX / ax;  // 在X方向的粗糙度标准化点积
    float b = HDotY / ay;  // 在Y方向的粗糙度标准化点积

    // 计算该点的总分布强度
    float c = a * a + b * b + NDotH * NDotH;  // 结合法线和切向量的粗糙度

    // 计算GTR2分布函数的值
    return 1.0 / (PI * ax * ay * c * c);  // 返回微面分布的强度
}
float SmithGAniso(float NDotV, float VDotX, float VDotY, float ax, float ay)
{
    float a = VDotX * ax;
    float b = VDotY * ay;
    float c = NDotV;
    return (2.0 * NDotV) / (NDotV + sqrt(a * a + b * b + c * c));
}
float GTR1(float NDotH, float a)
{
    if (a >= 1.0)
        return INV_PI;
    float a2 = a * a;
    float t = 1.0 + (a2 - 1.0) * NDotH * NDotH;
    return (a2 - 1.0) / (PI * log(a2) * t);
}
float SmithG(float NDotV, float alphaG)
{
    float a = alphaG * alphaG;
    float b = NDotV * NDotV;
    return (2.0 * NDotV) / (NDotV + sqrt(a + b - a * b));
}
vec3 EvalDisneyDiffuse(Material mat, vec3 Csheen, vec3 V, vec3 L, vec3 H, out float pdf)
{
    pdf = 0.0;

    // 如果光源方向 L 的 z 分量小于等于0，说明是折射不是漫反射
    if (L.z <= 0.0)
        return vec3(0.0);

    // 计算光源方向 L 与半程向量 H 的点积
    float LDotH = dot(L, H);

    // 计算粗糙度相关项 Rr，影响反射
    float Rr = 2.0 * mat.roughness * LDotH * LDotH;

    // 使用 Schlick 近似计算反射因子
    // SchlickWeight 函数计算光源方向和视角方向的反射权重
    float FL = SchlickWeight(L.z);  // 光源的反射权重
    float FV = SchlickWeight(V.z);  // 观察者的反射权重

    // 计算反射退化因子，基于粗糙度和光源/观察反射
    float Fretro = Rr * (FL + FV + FL * FV * (Rr - 1.0));

    // 计算漫反射反射率，SchlickWeight 调整影响
    float Fd = (1.0 - 0.5 * FL) * (1.0 - 0.5 * FV);

    // 模拟亚表面散射效果（Fake Subsurface Scattering）
    // Fss90 为在 90 度角的散射系数，Fss 结合了反射和散射的效果
    float Fss90 = 0.5 * Rr;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1.0 / (L.z + V.z) - 0.5) + 0.5);

    // 计算光泽（Sheen）效果
    // SchlickWeight 计算光泽影响因子
    float FH = SchlickWeight(LDotH);
    vec3 Fsheen = FH * mat.sheen * Csheen;

    // PDF 用于重要性采样，计算为 L.z / PI
    pdf = L.z * INV_PI;

    // 返回最终的漫反射反射值，混合漫反射、亚表面散射和光泽效果
    // 使用基色和材质的亚表面散射属性进行加权合成
    return INV_PI * mat.baseColor * mix(Fd + Fretro, ss, mat.subsurface) + Fsheen;
}
vec3 EvalMicrofacetReflection(Material mat, vec3 V, vec3 L, vec3 H, vec3 F, out float pdf)
{
    // 计算微面反射模型下的反射部分，用于处理光滑或粗糙材质的反射行为。
    pdf = 0.0;

    // 如果入射光的z分量小于等于0，则返回零（没有有效反射）
    if (L.z <= 0.0)
        return vec3(0.0);

    // 计算微面分布函数（GTR2），用于处理表面粗糙度
    float D = GTR2Aniso(H.z, H.x, H.y, mat.ax, mat.ay);

    // 计算几何遮蔽函数（Smith G），考虑视线和入射光的遮蔽
    float G1 = SmithGAniso(abs(V.z), V.x, V.y, mat.ax, mat.ay);
    float G2 = G1 * SmithGAniso(abs(L.z), L.x, L.y, mat.ax, mat.ay);

    // 计算概率密度函数（pdf）
    pdf = G1 * D / (4.0 * V.z);

    // 返回微面反射的颜色贡献，结合分布函数、几何遮蔽项和菲涅尔反射项
    return F * D * G2 * abs(dot(V, H)) / (4.0 * V.z);
}
vec3 EvalMicrofacetRefraction(Material mat, float eta, vec3 V, vec3 L, vec3 H, vec3 F, out float pdf)
{
    // 计算微面反射模型下的折射部分，用于处理透明材质（如玻璃、液体等）的折射行为。
    pdf = 0.0;

    // 检查光线是否为反射方向，若是则返回零
    if (L.z >= 0.0)
        return vec3(0.0);

    // 计算相关的内积
    float LDotH = dot(L, H);  // 入射光与半程向量的内积
    float VDotH = dot(V, H);  // 视线与半程向量的内积

    // 微面分布函数（GTR2）和几何遮蔽函数（Smith G）
    float D = GTR2Aniso(H.z, H.x, H.y, mat.ax, mat.ay);  // 微面分布项
    float G1 = SmithGAniso(abs(V.z), V.x, V.y, mat.ax, mat.ay);  // 视线的几何遮蔽项
    float G2 = G1 * SmithGAniso(abs(L.z), L.x, L.y, mat.ax, mat.ay);  // 入射光的几何遮蔽项

    // 折射部分的分母项
    float denom = LDotH + VDotH * eta;
    denom *= denom;  // 分母的平方

    // 折射率的平方
    float eta2 = eta * eta;

    // 雅可比矩阵的分量
    float jacobian = abs(LDotH) / denom;

    // 计算概率密度函数
    pdf = G1 * max(0.0, VDotH) * D * jacobian / V.z;

    // 计算折射部分的颜色贡献，包含菲涅尔反射项
    return pow(mat.baseColor, vec3(0.5)) * (1.0 - F) * D * G2 * abs(VDotH) * jacobian * eta2 / abs(L.z * V.z);
}
vec3 EvalClearcoat(Material mat, vec3 V, vec3 L, vec3 H, out float pdf)
{
    pdf = 0.0;

    // 如果入射光的z分量小于等于0，则返回零
    if (L.z <= 0.0)
        return vec3(0.0);

    // 计算视线与半程向量的内积
    float VDotH = dot(V, H);

    // 使用Schlick近似计算菲涅尔反射值
    float F = mix(0.04, 1.0, SchlickWeight(VDotH));

    // 微面分布函数，采用GTR1模型
    float D = GTR1(H.z, mat.clearcoatRoughness);

    // 计算几何遮蔽项
    float G = SmithG(L.z, 0.25) * SmithG(V.z, 0.25);

    // 计算雅可比矩阵的分量
    float jacobian = 1.0 / (4.0 * VDotH);

    // 计算概率密度函数
    pdf = D * H.z * jacobian;

    // 计算清漆层的反射贡献
    return vec3(F) * D * G;
}
vec3 DisneySample(State state, vec3 V, vec3 N, out vec3 L, out float pdf)
{
    // Disney BSDF采样函数，根据给定的状态（state），入射方向（V）和法线（N），
    // 采样出散射方向L，并计算采样的概率密度函数pdf。

    // 初始化pdf为0
    pdf = 0.0;

    // 随机数生成，用于后续的采样过程
    float r1 = rand();
    float r2 = rand();

    vec3 T, B;
    Onb(N, T, B);

    // 将入射方向V转换到局部坐标系，简化计算过程（N·L = L.z; N·V = V.z; N·H = H.z）
    V = ToLocal(T, B, N, V);

    // 计算颜色调节相关的参数（光泽度和反射率等）
    vec3 Csheen, Cspec0;
    float F0;
    TintColors(state.mat, state.eta, F0, Csheen, Cspec0); // 获取材质颜色参数

    // 材质的权重计算
    float dielectricWt = (1.0 - state.mat.metallic) * (1.0 - state.mat.specTrans); // 介质权重
    float metalWt = state.mat.metallic; // 金属权重
    float glassWt = (1.0 - state.mat.metallic) * state.mat.specTrans; // 玻璃权重

    // 使用Schlick近似计算视角相关的反射权重
    float schlickWt = SchlickWeight(V.z);

    // 计算不同材质的光照贡献概率
    float diffPr = dielectricWt * Luminance(state.mat.baseColor);  // 漫反射贡献
    float dielectricPr = dielectricWt * Luminance(mix(Cspec0, vec3(1.0), schlickWt));  // 介质反射贡献
    float metalPr = metalWt * Luminance(mix(state.mat.baseColor, vec3(1.0), schlickWt));  // 金属反射贡献
    float glassPr = glassWt;  // 玻璃贡献
    float clearCtPr = 0.25 * state.mat.clearcoat;  // 清漆贡献

    // 归一化所有权重，使得它们的总和为1
    float invTotalWt = 1.0 / (diffPr + dielectricPr + metalPr + glassPr + clearCtPr);
    diffPr *= invTotalWt;
    dielectricPr *= invTotalWt;
    metalPr *= invTotalWt;
    glassPr *= invTotalWt;
    clearCtPr *= invTotalWt;

    // 计算每种材质的累计概率分布（CDF）
    float cdf[5];
    cdf[0] = diffPr; 
    cdf[1] = cdf[0] + dielectricPr;
    cdf[2] = cdf[1] + metalPr;
    cdf[3] = cdf[2] + glassPr;
    cdf[4] = cdf[3] + clearCtPr;

    // 生成一个随机数，选择一个材质进行采样
    float r3 = rand();

    // 根据CDF选择合适的材质类型并采样对应的散射方向
    if (r3 < cdf[0]) // 漫反射（Diffuse）
    {
        L = CosineSampleHemisphere(r1, r2); // 使用余弦加权半球采样
    }
    else if (r3 < cdf[2]) // 介质反射 + 金属反射（Dielectric + Metallic reflection）
    {
        vec3 H = SampleGGXVNDF(V, state.mat.ax, state.mat.ay, r1, r2); // 使用GGX分布采样半角H

        // 如果H的z分量为负，则反转半角H
        if (H.z < 0.0)
            H = -H;

        // 计算反射方向L
        L = normalize(reflect(-V, H)); // 根据半角H和入射方向V计算反射方向
    }
    else if (r3 < cdf[3]) // 玻璃（Glass）
    {
        vec3 H = SampleGGXVNDF(V, state.mat.ax, state.mat.ay, r1, r2); // 使用GGX分布采样半角H
        float F = DielectricFresnel(abs(dot(V, H)), state.eta); // 计算Fresnel反射系数

        // 如果H的z分量为负，则反转半角H
        if (H.z < 0.0)
            H = -H;

        // 重用随机数r3，并对其进行归一化，用于后续的折射与反射判断
        r3 = (r3 - cdf[2]) / (cdf[3] - cdf[2]);

        // 根据Fresnel反射系数选择反射或折射
        if (r3 < F) // 反射
        {
            L = normalize(reflect(-V, H));
        }
        else // 折射
        {
            L = normalize(refract(-V, H, state.eta)); // 使用折射公式计算折射方向
        }
    }
    else // 清漆（Clearcoat）
    {
        vec3 H = SampleGTR1(state.mat.clearcoatRoughness, r1, r2); // 使用GTR1分布采样清漆表面的半角H

        // 如果H的z分量为负，则反转半角H
        if (H.z < 0.0)
            H = -H;

        // 计算反射方向L
        L = normalize(reflect(-V, H)); // 根据半角H和入射方向V计算反射方向
    }

    // 将散射方向L和入射方向V从局部坐标系转换回世界坐标系
    L = ToWorld(T, B, N, L);
    V = ToWorld(T, B, N, V);

    // 返回Disney评估函数的结果，并计算采样的pdf
    return DisneyEval(state, V, N, L, pdf);
}
vec3 DisneyEval(State state, vec3 V, vec3 N, vec3 L, out float pdf)
{
    // 初始化返回值
    pdf = 0.0;
    vec3 f = vec3(0.0); // 最终的反射颜色（BRDF值）

    vec3 T, B;
    Onb(N, T, B);

    V = ToLocal(T, B, N, V);
    L = ToLocal(T, B, N, L);

    // 计算半程向量H（视角V和光源L的单位和）
    vec3 H;
    if (L.z > 0.0)
        H = normalize(L + V);  // 反射方向
    else
        H = normalize(L + V * state.eta);  // 折射方向

    // 保证H.z为正，以确保计算正确
    if (H.z < 0.0)
        H = -H;

    // 获取材质的反射颜色（清漆和金属反射）
    vec3 Csheen, Cspec0;
    float F0;
    TintColors(state.mat, state.eta, F0, Csheen, Cspec0);

    // 计算不同反射模式的权重
    float dielectricWt = (1.0 - state.mat.metallic) * (1.0 - state.mat.specTrans); // 非金属和非透明权重
    float metalWt = state.mat.metallic;  // 金属权重
    float glassWt = (1.0 - state.mat.metallic) * state.mat.specTrans; // 玻璃权重

    // 计算每种反射模式的亮度权重
    float diffPr = dielectricWt * Luminance(state.mat.baseColor);  // 漫反射的亮度
    float dielectricPr = dielectricWt * Luminance(mix(Cspec0, vec3(1.0), SchlickWeight(V.z)));  // 非金属反射
    float metalPr = metalWt * Luminance(mix(state.mat.baseColor, vec3(1.0), SchlickWeight(V.z)));  // 金属反射
    float glassPr = glassWt;  // 玻璃反射
    float clearCtPr = 0.25 * state.mat.clearcoat;  // 清漆反射的权重

    // 归一化权重，以确保所有概率的总和为1
    float invTotalWt = 1.0 / (diffPr + dielectricPr + metalPr + glassPr + clearCtPr);
    diffPr *= invTotalWt;
    dielectricPr *= invTotalWt;
    metalPr *= invTotalWt;
    glassPr *= invTotalWt;
    clearCtPr *= invTotalWt;

    // 判断反射方向
    bool reflect = L.z * V.z > 0; // 如果视角和光源的z分量同符号则是反射

    // 临时PDF值，用于存储各反射模式的PDF
    float tmpPdf = 0.0;
    float VDotH = abs(dot(V, H));  // 视角与半程向量的点积

    // 计算漫反射部分
    if (diffPr > 0.0 && reflect)
    {
        // 计算漫反射BRDF值，并更新PDF
        f += EvalDisneyDiffuse(state.mat, Csheen, V, L, H, tmpPdf) * dielectricWt;
        pdf += tmpPdf * diffPr; // 更新PDF
    }

    // 计算非金属反射部分（Dielectric Reflection）
    if (dielectricPr > 0.0 && reflect)
    {
        // 计算折射率F（Schlick近似），根据VDotH和折射率eta计算
        float F = (DielectricFresnel(VDotH, 1.0 / state.mat.ior) - F0) / (1.0 - F0);

        // 计算微面元反射BRDF，并更新PDF
        f += EvalMicrofacetReflection(state.mat, V, L, H, mix(Cspec0, vec3(1.0), F), tmpPdf) * dielectricWt;
        pdf += tmpPdf * dielectricPr;  // 更新PDF
    }

    // 计算金属反射部分
    if (metalPr > 0.0 && reflect)
    {
        // 计算基于视角和半程向量的Schlick反射（与材质的基本颜色混合）
        vec3 F = mix(state.mat.baseColor, vec3(1.0), SchlickWeight(VDotH));

        // 计算微面元反射BRDF，并更新PDF
        f += EvalMicrofacetReflection(state.mat, V, L, H, F, tmpPdf) * metalWt;
        pdf += tmpPdf * metalPr; // 更新PDF
    }

    // 玻璃/折射反射部分
    if (glassPr > 0.0)
    {
        // 使用Dielectric Fresnel公式计算折射率F
        float F = DielectricFresnel(VDotH, state.eta);

        if (reflect)
        {
            // 计算微面元反射BRDF，并更新PDF
            f += EvalMicrofacetReflection(state.mat, V, L, H, vec3(F), tmpPdf) * glassWt;
            pdf += tmpPdf * glassPr * F;  // 更新PDF
        }
        else
        {
            // 计算折射BRDF，并更新PDF
            f += EvalMicrofacetRefraction(state.mat, state.eta, V, L, H, vec3(F), tmpPdf) * glassWt;
            pdf += tmpPdf * glassPr * (1.0 - F);  // 更新PDF
        }
    }

    // 清漆层反射部分
    if (clearCtPr > 0.0 && reflect)
    {
        // 计算清漆反射BRDF，并更新PDF
        f += EvalClearcoat(state.mat, V, L, H, tmpPdf) * 0.25 * state.mat.clearcoat;
        pdf += tmpPdf * clearCtPr; // 更新PDF
    }

    // 最终返回反射颜色，乘以光源与表面法线的夹角cos(θ)
    return f * abs(L.z);
}