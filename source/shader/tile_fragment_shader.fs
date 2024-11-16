#version 330

#define PI 3.14159265358979323
#define INV_PI 0.31830988618379067
#define TWO_PI 6.28318530717958648
#define INV_TWO_PI 0.15915494309189533
#define INV_4_PI 0.07957747154594766
#define EPS 0.0003
#define INF 1000000000.0

out vec4 FragColor;

in vec2 TexCoords;

// 摄像机
struct Camera
{
    vec3 up;           // 相机的“上”方向（垂直方向），用于确定视图矩阵
    vec3 right;        // 相机的“右”方向（水平方向），用于确定视图矩阵
    vec3 forward;      // 相机的前进方向，决定相机的朝向
    vec3 position;     // 相机的位置
    float fov;         // 相机的视场角（field of view）
    float focalDist;   // 相机的焦距（影响景深效果）
    float aperture;    // 相机的光圈大小（影响景深和模糊效果）
};

struct Ray
{
    vec3 origin;// 光线的起点
    vec3 direction;// 光线的方向
};

struct State
{
    int depth;           // 当前路径追踪的深度
    float eta;           // 折射率（用于折射光线的计算）
    float hitDist;       // 光线与物体的交点到光线起点的距离

    vec3 fhp;            // 交点的世界坐标（光线与物体交点的位置）
    vec3 normal;         // 物体表面的法线
    vec3 ffnormal;       // 光线入射时的法线（可能与normal不同，特别是透过介质的情况）
    vec3 tangent;        // 物体表面的切线（用于计算纹理坐标等）
    vec3 bitangent;      // 物体表面的副切线（用于计算纹理坐标等）

    bool isEmitter;      // 是否为发光物体（用于判断物体是否发光并直接贡献辐射）
    vec2 texCoord;
};

struct LightSampleRec
{
    vec3 normal;   // 光源的法线（用于计算光线的方向）
    vec3 emission; // 光源的发光强度
    vec3 direction; // 从物体交点到光源的方向
    float dist;     // 从交点到光源的距离
    float pdf;      // 该光源方向的概率密度函数（PDF）
};

float RectIntersect(in vec3 pos, in vec3 u, in vec3 v, in vec4 plane, in Ray r);
float SphereIntersect(float radius, vec3 center, in Ray r);
float AABBIntersect(vec3 minCorner, vec3 maxCorner, in Ray r);
bool ClosestHit(Ray r, inout State state, inout LightSampleRec lightSample);
vec4 PathTrace(Ray r);

uvec4 seed;
ivec2 pixel;
void InitRNG(vec2 p, int frame);
void pcg4d(inout uvec4 v);
float rand();

uniform int topBVHIndex;
uniform samplerBuffer BVH;
uniform samplerBuffer verticesTex;
uniform samplerBuffer normalsTex; 
uniform isamplerBuffer vertexIndicesTex;
uniform sampler2D transformsTex; 

uniform vec2 resolution;
Camera camera;

void main()
{
    // 从屏幕坐标生成光线
    vec2 d = TexCoords * 2.0 - 1.0; // 将坐标范围映射到 [-1, 1]
    float scale = tan(camera.fov * 0.5);
    d.y *= resolution.y / resolution.x * scale; // 调整纵横比
    d.x *= scale; // 按照视角缩放

    // 计算光线方向
    vec3 rayDir = normalize(d.x * camera.right + d.y * camera.up + camera.forward);

    // 创建光线
    Ray ray = Ray(camera.position, rayDir);

    vec4 pixelColor = PathTrace(ray);
    FragColor = pixelColor;
}

/********************************/
// 进行路径追踪计算
/********************************/
vec4 PathTrace(Ray r)
{
    vec3 radiance = vec3(0.0);   // 最终的辐射贡献 -- 颜色
    vec3 throughput = vec3(1.0); // 路径传播因子 -- 跟踪光线的强度
    State state;                 // 光线与物体交互的状态
    LightSampleRec lightSample;       // 光源采样记录
    // ScatterSampleRec scatterSample;   // 散射采样记录

    float alpha = 1.0;  // 透明度 -- 用于材质的遮挡和半透明效果

    bool inMedium = false;
    bool mediumSampled = false;
    bool surfaceScatter = false;

    int maxDepth = 10;
    int RRMaxDepth = 5;
    // 路径追踪循环
    for(state.depth = 0; state.depth < maxDepth; state.depth++) 
    {
        // 计算光线与场景的最近交点，如果没有击中任何物体，说明光线射到了背景或者环境光
        bool hit = ClosestHit(r, state, lightSample);

        if(!hit) 
        {
            // 没有命中物体，意味着光线穿过了场景并到达了背景
            // 不考虑环境贴图 直接返回一个设置一个背景颜色
            vec3 backgroundColor = vec3(0.5, 0.7, 1.0);
            radiance = backgroundColor;
            break;
        }

        // 获取交点物体的材质信息
        // GetMaterial(state, r);

        // 计算来自发光物体的辐射
        // radiance += state.mat.emission * throughput;
        radiance += vec3(1.0f, 0.5f, 0.5f) * throughput; // 先使用红色代替

        // 如果命中的物体时发光体/光源，则计算该发光体的辐射贡献并终止路径追踪
        if(state.isEmitter)
        {
            float misWeight = 1.0;

            radiance += misWeight * lightSample.emission * throughput;
            break;
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

    return vec4(radiance, alpha);
}
/********************************/
// 求取最近交点
/********************************/
bool ClosestHit(Ray r, inout State state, inout LightSampleRec lightSample)
{
    float tMin = INF; // 记录最近的击中时间
    float tTmp;       // 记录每次的击中时间

    // 检查是否击中光源
    // TODO

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
        } else if(leaf < 0) {
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
        } else {
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