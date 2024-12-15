#version 330

#define PI 3.14159265358979323
#define INV_PI .31830988618379067
#define TWO_PI 6.28318530717958648
#define INV_TWO_PI .15915494309189533
#define INV_4_PI .07957747154594766
#define EPS .0003
#define INF 1000000000.

#define maxNumOfLights 10
#define RECT_LIGHT 0
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
// 光线
struct Ray
{
    vec3 origin;// 光线的起点
    vec3 direction;// 光线的方向
};
// 矩形光
struct RectLight
{
    vec3 position;
    vec3 emission;
    vec3 u;
    vec3 v;
    float area;
};
// 球形光
struct SphereLight
{
    vec3 position;
    vec3 emission;
    float radius;
    float area;
};
// 平行光
struct DistantLight
{
    vec3 position;
    vec3 emission;
};
// 材质
struct Material
{
    vec3 baseColor;     // 漫反射颜色
    vec3 specular;      // 镜面反射颜色
    vec3 emission;      // 自发光颜色
    vec3 normal;        // 表面法线贴图
    vec3 F0;            // 镜面反射的基础反射率
    
    float ior;// 折射率
    float alpha;// 透明度
    float roughness;// 粗糙度
    float matellic;// 金属度
    float scatter;// 散射
    float coat;// 涂层
    float coatRough;// 涂层粗糙度
    float ao;// 环境光遮蔽

    float absorption;// 吸收
    float density;// 密度
    float anisotropy;// 各向异性
    float isVolume;// 缩放
};
// 击中点记录
struct HitRec
{
    float eta;// 当前介质的折射率，用于计算折射行为
    // 记录光线交点信息
    bool isEmitter;// 记录击中的物体属性 -- 是否为光源
    vec3 emission;// 记录击中物体的自发光属性
    float HitDist;// 光线与物体的交点距离（光线参数 t 值）
    vec3 HitPoint;// 记录光线与物品的交点
    // 击中点三角形数据
    vec2 texCoord;// 纹理坐标 -- 通过重心坐标插值计算得到
    vec3 normal;// 表面法线 -- 通过重心坐标插值计算得到
    vec3 normal_tex;// 纹理法线
    vec3 ffnormal;// 面向外部的法线 -- 与光线进入的方向相反
    vec3 tangent;// 切线方向
    vec3 bitangent;// 副切线方向
    // 击中点材质信息
    int matID;// 材质 ID
    Material mat;// 物体的材质属性，包含颜色、粗糙度等细节
};
// 记录光源采样的结果
struct LightSampleRec
{
    vec3 normal;// 光源表面的法向量
    vec3 emission;// 光源的辐射亮度值
    vec3 direction;// 从交点指向光源的单位方向向量
    float dist;// 交点与光源之间的距离
    float pdf;// 采样到该光源的概率密度函数值
};

// 路径追踪函数
vec3 PathTrace(Ray r,int maxDepth,int RR_maxDepth);
vec3 calculateDirectLighting(Ray r, HitRec hit_record, LightSampleRec lightSample);
// 相交测试函数
bool ClosestHit(Ray r,inout HitRec hit_record,inout LightSampleRec lightSample);
bool AnyHit(Ray r,float maxDist);
float RectIntersect(in vec3 pos,in vec3 u,in vec3 v,in vec4 plane,in Ray r);
float SphereIntersect(float radius,vec3 center,in Ray r);
float AABBIntersect(vec3 minCorner,vec3 maxCorner,in Ray r);
// 获取相交点材质函数
void GetMaterial(inout HitRec hit_record,in Ray r);
// 光源采样函数
void sampleLight(vec3 hitPoint,inout LightSampleRec lightSample);
void SampleSphereLight(in SphereLight light,in vec3 hitPoint,inout LightSampleRec lightSample);
void SampleRectLight(in RectLight light,in vec3 hitPoint,inout LightSampleRec lightSample);
void SampleDistantLight(in DistantLight light,in vec3 hitPoint,inout LightSampleRec lightSample);
// 随机散射光线方向生成函数
vec3 SampleDirection(vec3 normal,bool useCosineWeighted);
vec3 UniformSampleHemisphere(float r1,float r2);
vec3 CosineSampleHemisphere(float r1,float r2);
// 随机光线方向生成辅助函数
void GenerateTBN(in vec3 N,inout vec3 T,inout vec3 B);
vec3 ToWorld(vec3 X,vec3 Y,vec3 Z,vec3 V);
vec3 ToLocal(vec3 X,vec3 Y,vec3 Z,vec3 V);
vec3 LocalToWorld(vec3 localDir,vec3 normal);
vec3 WorldToLocal(vec3 globalDir,vec3 normal);
// PBR光源模型函数
vec3 BRDF_PBR(vec3 N,vec3 V,vec3 L,vec3 radiance,vec3 albedo,float metallic,float roughness,vec3 F0);
float DistributionGGX(vec3 N,vec3 H,float roughness);
float GeometrySchlickGGX(float NdotV,float roughness);
float GeometrySmith(vec3 N,vec3 V,vec3 L,float roughness);
vec3 fresnelSchlick(float cosTheta,vec3 F0);
float fresnelSchlick(float cosTheta, float ior);
bool Refract(vec3 I, vec3 N, float ior, out vec3 refracted);
// 随机数函数
uvec4 seed;
ivec2 pixel;
void InitRNG(vec2 p,int frame);
void pcg4d(inout uvec4 v);
float rand();

uniform int topBVHIndex;
uniform sampler2D accumTexture;
uniform samplerBuffer BVHTex;
uniform samplerBuffer verticesTex;
uniform samplerBuffer normalsTex;
uniform isamplerBuffer vertexIndicesTex;
uniform samplerBuffer transformsTex;
uniform samplerBuffer materialsTex;
uniform samplerBuffer lightsTex;
uniform sampler2DArray textureMapsArrayTex;

// TODO:需要加入Uniform
int numOfRectLights;
int numOfSphereLights;
int numOfDistantLights;
RectLight rectLights[maxNumOfLights];
SphereLight sphereLights[maxNumOfLights];
DistantLight distantLights[maxNumOfLights];

uniform vec2 resolution;
uniform Camera camera;
uniform int SampleNum; // 采样数量 -- 

void main()
{
    // 屏幕坐标直接映射生成光线
    // TODO: 添加扰动
    InitRNG(gl_FragCoord.xy + vec2(rand(), rand()), SampleNum); // 更新随机数种子
    vec2 d=(TexCoords*2.-1.);// 将坐标范围映射到 [-1, 1]
    float scale=tan(camera.fov*.5);
    d.y*=resolution.y/resolution.x*scale;
    d.x*=scale;
    
    vec3 rayOrigin = camera.position;
    vec3 rayDirection = normalize(d.x * camera.right + d.y * camera.up + camera.forward);
    float radius = 1;
    sphereLights[0] = SphereLight(vec3(-34.53,27.02,8.634), vec3(100), radius, 4 * PI * radius * radius);
    sphereLights[1] = SphereLight(vec3(34.15,26.82,8.719), vec3(100), radius, 4 * PI * radius * radius);
    sphereLights[2] = SphereLight(vec3(-36.92,26.97,82.96), vec3(100), radius, 4 * PI * radius * radius);
    sphereLights[3] = SphereLight(vec3(33.55,26.88,83.29), vec3(100), radius, 4 * PI * radius * radius);
    sphereLights[4] = SphereLight(vec3(-34.31,26.83,158.16), vec3(100), radius, 4 * PI * radius * radius);
    sphereLights[5] = SphereLight(vec3(36.96,26.83,154.76), vec3(100), radius, 4 * PI * radius * radius);
    sphereLights[6] = SphereLight(vec3(-33.80,26.94,227.89), vec3(100), radius, 4 * PI * radius * radius);
    sphereLights[7] = SphereLight(vec3(34.24,26.84,228.11), vec3(100), radius, 4 * PI * radius * radius);
    sphereLights[8] = SphereLight(vec3(0.0,0.0,0.0), vec3(100), radius, 4 * PI * radius * radius);
    numOfSphereLights = 9;
    
    Ray ray=Ray(rayOrigin,rayDirection);// 生成光线
    
    // 后面可以更改为 uniform 使用imgui进行调节
    int maxDepth = 5;   // 光线弹射的最大深度
    int RR_maxDepth = 2;// 俄罗斯轮盘赌启动的最低深度
    
    vec4 accumColor=texture(accumTexture,TexCoords);
    vec4 color=vec4(PathTrace(ray,maxDepth,RR_maxDepth),1.0);
    
    vec3 finalColor=accumColor.xyz+color.xyz;
    
    FragColor=vec4(finalColor,1.0);
}

/********************************/
// 进行路径追踪计算
/********************************/
vec3 PathTrace(Ray r, int maxDepth, int RR_maxDepth)
{
    vec3 radiance = vec3(0.0);   // 最终的辐射贡献 -- 颜色
    vec3 throughput = vec3(1.0); // 路径的通量权重（每次交互更新光的贡献）
    
    LightSampleRec lightSample;      // 光源采样记录
    
    for(int depth = 0; depth < maxDepth; depth++)
    {
        HitRec hit_record;  // 记录当前的击中状态
        
        // 1. 判断光线是否与场景相交
        if(!ClosestHit(r, hit_record, lightSample))
        {
            // 如果没有交点，返回环境背景颜色
            radiance += throughput * vec3(0.8353, 0.4745, 0.8745); // 纯黑色
            break;
        }
        
        // 2. 从材质贴图中加载材质信息
        GetMaterial(hit_record, r); // 获取材质
        // return hit_record.mat.baseColor;
        
        // 3. 如果击中了发光体，添加其辐射贡献
        if (hit_record.isEmitter) {
            if(depth == 0){
                radiance += vec3(0.651, 0.8118, 0.1216);
            } else {
                radiance += throughput * calculateDirectLighting(r, hit_record, lightSample);
            }
            break;
        } else {
            sampleLight(hit_record.HitPoint, lightSample); // 进行光源采样
            vec3 L = lightSample.direction;
            Ray r2light = Ray(hit_record.HitPoint + 0.001 * L, L);   // 发射一条从散射点到光源的射线 -- 阴影射线
            bool isShadow = AnyHit(r2light, lightSample.dist - EPS); // 判断阴影
            if (!isShadow) {
                vec3 Lo = calculateDirectLighting(r, hit_record, lightSample);
                Lo = Lo / lightSample.pdf;
                // return Lo;
                radiance += throughput * Lo;
            }
        }
        //  // 4. 处理自发光 
        // if (hit_record.emission.r > 0.0001) {
        //     // return vec3(1.0, 0.0, 0.0);
        //     radiance += throughput * hit_record.emission * 0.001;
        //     // if (hit_record.mat.alpha < 1.0) {
        //         // continue;
        //     // }
        //     break;
        // }

        // // 5. 判断是否击中体积散射体
        // if (hit_record.mat.isVolume > 0.5) {
        //     r = Ray(hit_record.HitPoint + 0.001 * hit_record.ffnormal, r.direction);
        //     radiance += handleVolumeScattering(r, hit_record, throughput);
        //     continue;
        // }

        // 6. 间接光照计算（路径延续）
        // 7. 俄罗斯轮盘赌优化
        float prob = 0.7; // 概率阈值
        float m_prob = rand();
        if (depth > RR_maxDepth && m_prob > prob) {
            break; // 停止追踪路径
        }
        throughput *= m_prob; // 更新通量权重以考虑路径终止的概率
        
        // 4. 处理透明材质的反射和折射
        if(hit_record.mat.alpha < 1.0) 
        {
            vec3 refractedDirection;
            bool isTotalInternalReflection = !Refract(r.direction, hit_record.ffnormal, hit_record.mat.ior, refractedDirection);
            
            if (!isTotalInternalReflection) {
                // 计算菲涅耳反射系数
                float cosTheta = dot(-r.direction, hit_record.normal);
                float fresnel = fresnelSchlick(cosTheta, hit_record.mat.ior);

                // 反射路径
                vec3 reflectedDirection = reflect(r.direction, hit_record.normal);
                Ray reflectedRay = Ray(hit_record.HitPoint + 0.001 * reflectedDirection, reflectedDirection);

                // 折射路径
                Ray refractedRay = Ray(hit_record.HitPoint + 0.001 * refractedDirection, refractedDirection);

                // 通过菲涅耳系数决定光的分支
                if (rand() < fresnel) {
                    r = reflectedRay; // 反射路径
                } else {
                    r = refractedRay; // 折射路径
                }
            } 
            else {
                // 处理全内反射
                vec3 reflectedDirection = reflect(r.direction, hit_record.normal);
                r = Ray(hit_record.HitPoint + 0.001 * reflectedDirection, reflectedDirection);
            }

            throughput *= hit_record.mat.baseColor; // 透明材质的颜色
            continue;
        }
        // 8. 采样下一个方向 -- 漫反射
        bool useCosineWeighted = true;
        vec3 wi = SampleDirection(hit_record.normal, useCosineWeighted); // 在半球中随机采样
        float pdf = useCosineWeighted ?  (wi.z / PI) : (1.0 / (2.0 * PI));
        // throughput /= pdf;
        r = Ray(hit_record.HitPoint + 0.001 * hit_record.normal, wi);      // 更新光线
    }

    return radiance;
}
/********************************/
// PBR光照模型
// 计算单次光线与材质的交互结果
/********************************/
vec3 calculateDirectLighting(Ray r, HitRec hit_record, LightSampleRec lightSample)
{
    // 交点法线、视线方向和材质参数
    vec3 N = hit_record.normal;
    vec3 V = -r.direction;
    vec3 L = lightSample.direction;
    // return hit_record.mat.baseColor;
    vec3 albedo = pow(hit_record.mat.baseColor, vec3(2.2));
    // return albedo;
    float metallic = hit_record.mat.matellic;
    float roughness = hit_record.mat.roughness;
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    float invDistances2 = 1 / (lightSample.dist * lightSample.dist);
    vec3 Li = lightSample.emission;

    return BRDF_PBR(N, V, L, Li, albedo, metallic, roughness, F0);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
float fresnelSchlick(float cosTheta, float ior)
{
    float r0 = pow((1.0 - ior) / (1.0 + ior), 2.0);
    return r0 + (1.0 - r0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
bool Refract(vec3 I, vec3 N, float ior, out vec3 refracted)
{
    float cosThetaI = dot(-I, N);
    float eta = 1.0 / ior; // 介质比率（例如从空气 (1.0) 到玻璃 (1.5)）
    float sinThetaTSq = eta * eta * (1.0 - cosThetaI * cosThetaI);

    // 检查是否发生**全内反射**
    if (sinThetaTSq > 1.0) 
        return false;

    float cosThetaT = sqrt(1.0 - sinThetaTSq);
    refracted = eta * I + (eta * cosThetaI - cosThetaT) * N;
    return true;
}
// ----------------------------------------------------------------------------
vec3 BRDF_PBR(vec3 N, vec3 V, vec3 L, vec3 radiance, vec3 albedo, float metallic, float roughness, vec3 F0)
{
    vec3 H = normalize(V + L); // 半程向量
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);
    
    return (kD * albedo / PI + specular) * radiance * NdotL;
}
/********************************/
// 求取交点
/********************************/
bool ClosestHit(Ray r, inout HitRec hit_record, inout LightSampleRec lightSample)
{
    float tMin = INF; // 记录最近的击中时间
    float tTmp;       // 记录每次的击中时间
    
    // 检查是否击中光源
    // TODO
    for (int i = 0; i < numOfRectLights; i++)
    {
        
    }
    for (int i = 0; i < numOfSphereLights; i++)
    {
        vec3 position = sphereLights[i].position;
        vec3 emission = sphereLights[i].emission;
        float radius  = sphereLights[i].radius;
        float area    = sphereLights[i].area;
        tTmp = SphereIntersect(radius, position, r);
        if (tTmp < 0.)
        tTmp = INF;
        if (tTmp < tMin)
        {
            tMin = tTmp;
            vec3 hitPt = r.origin + tMin * r.direction;
            float cosTheta = dot(-r.direction, normalize(hitPt - position));
            lightSample.pdf = (tMin * tMin) / (area * cosTheta * 0.5);
            lightSample.emission = emission;
            hit_record.isEmitter = true;
        }
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
    
    int currMatID; // 记录当前Mesh的材质ID值
    
    // 使用栈的方式模仿递归
    while (index != -1)
    {
        ivec3 LRLeaf = ivec3(texelFetch(BVHTex, index * 3 + 2).xyz);
        
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
                    hit_record.matID = currMatID;
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
            // vec4 r1 = texelFetch(transformsTex, ivec2((-leaf - 1) * 4 + 0, 0), 0).xyzw;
            // vec4 r2 = texelFetch(transformsTex, ivec2((-leaf - 1) * 4 + 1, 0), 0).xyzw;
            // vec4 r3 = texelFetch(transformsTex, ivec2((-leaf - 1) * 4 + 2, 0), 0).xyzw;
            // vec4 r4 = texelFetch(transformsTex, ivec2((-leaf - 1) * 4 + 3, 0), 0).xyzw;
            vec4 r1 = texelFetch(transformsTex, (-leaf - 1) * 4 + 0);  // 获取变换矩阵的第一列
            vec4 r2 = texelFetch(transformsTex, (-leaf - 1) * 4 + 1);  // 获取变换矩阵的第二列
            vec4 r3 = texelFetch(transformsTex, (-leaf - 1) * 4 + 2);  // 获取变换矩阵的第三列
            vec4 r4 = texelFetch(transformsTex, (-leaf - 1) * 4 + 3);  // 获取变换矩阵的第四列
            
            // 构建变换矩阵 -- 将光线从世界坐标系转换到对象的局部坐标系
            transMat = mat4(r1, r2, r3, r4);
            
            rTrans.origin    = vec3(inverse(transMat) * vec4(r.origin, 1.0));
            rTrans.direction = vec3(inverse(transMat) * vec4(r.direction, 0.0));
            
            stack[ptr++] = -1; // 添加一个标记 -- 遍历完BLAS时返回到该位置
            index = leftIndex;
            BLAS = true;
            currMatID = rightIndex;
            continue;
        }
        else
        {
            // 非叶节点
            leftHit  = AABBIntersect(texelFetch(BVHTex, leftIndex  * 3 + 0).xyz, texelFetch(BVHTex, leftIndex  * 3 + 1).xyz, rTrans);
            rightHit = AABBIntersect(texelFetch(BVHTex, rightIndex * 3 + 0).xyz, texelFetch(BVHTex, rightIndex * 3 + 1).xyz, rTrans);
            
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
    
    hit_record.HitDist = tMin;                            // 记录光线击中物体的最近时间/距离
    hit_record.HitPoint = r.origin + r.direction * tMin;  // 记录光线与物体的交点
    
    // 光线击中的是一个三角形 -- 将交点的信息进行存储
    if (triID.x != -1)
    {
        hit_record.isEmitter = false;
        
        vec4 n0 = texelFetch(normalsTex, triID.x);
        vec4 n1 = texelFetch(normalsTex, triID.y);
        vec4 n2 = texelFetch(normalsTex, triID.z);
        // 从顶点坐标数据和法线数据的w分量中提取纹理坐标位置信息
        vec2 t0 = vec2(vert0.w, n0.w);
        vec2 t1 = vec2(vert1.w, n1.w);
        vec2 t2 = vec2(vert2.w, n2.w);
        // 使用重心坐标插值计算交点的纹理坐标和法线
        hit_record.texCoord = t0 * bary.x + t1 * bary.y + t2 * bary.z;                  // 纹理坐标
        vec3 normal = normalize(n0.xyz * bary.x + n1.xyz * bary.y + n2.xyz * bary.z);   // 法线向量
        hit_record.normal = normalize(transpose(inverse(mat3(transform))) * normal);
        hit_record.ffnormal = dot(hit_record.normal, r.direction) <= 0.0 ? hit_record.normal : -hit_record.normal;
        // 计算切线和副切线
        vec3 deltaPos1 = vert1.xyz - vert0.xyz;
        vec3 deltaPos2 = vert2.xyz - vert0.xyz;
        vec2 deltaUV1 = t1 - t0;
        vec2 deltaUV2 = t2 - t0;
        float invdet = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        hit_record.tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * invdet;
        hit_record.bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * invdet;
        // 将切线和副切线转换到世界坐标系并归一化
        hit_record.tangent = normalize(mat3(transform) * hit_record.tangent);
        hit_record.bitangent = normalize(mat3(transform) * hit_record.bitangent);
    }
    
    return true;
}

bool AnyHit(Ray r, float maxDist)
{
    // 检查是否击中光源SphereIntersect
    // TODO
    // for (int i = 0; i < numOfRectLights; i++)
    // {
        //     // 与矩形光源相交测试，假设我们已经有矩形光源的属性
        //     RectLight light = rectLights[i];
        //     if (IntersectRectLight(r, light, maxDist))
        //         return true;
    // }
    
    for (int i = 0; i < numOfSphereLights; i++)
    {
        vec3 position = sphereLights[i].position;
        vec3 emission = sphereLights[i].emission;
        float radius  = sphereLights[i].radius;
        float area    = sphereLights[i].area;
        
        float intersectionDistance = SphereIntersect(radius, position, r);
        if (intersectionDistance > 0.0 && intersectionDistance < maxDist)
        return true;
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
    
    float currMatID;
    
    while (index != -1)
    {
        ivec3 LRLeaf = ivec3(texelFetch(BVHTex, index * 3 + 2).xyz);
        
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
            vec4 r1 = texelFetch(transformsTex, (-leaf - 1) * 4 + 0);
            vec4 r2 = texelFetch(transformsTex, (-leaf - 1) * 4 + 1);
            vec4 r3 = texelFetch(transformsTex, (-leaf - 1) * 4 + 2);
            vec4 r4 = texelFetch(transformsTex, (-leaf - 1) * 4 + 3);
            // 构建变换矩阵 -- 将光线从世界坐标系转换到对象的局部坐标系
            mat4 transform = mat4(r1, r2, r3, r4);
            
            rTrans.origin    = vec3(inverse(transform) * vec4(r.origin, 1.0));
            rTrans.direction = vec3(inverse(transform) * vec4(r.direction, 0.0));
            
            stack[ptr++] = -1; // 添加一个标记 -- 遍历完BLAS时返回到该位置
            index = leftIndex;
            BLAS = true;
            currMatID = rightIndex;
            continue;
        }
        else
        {
            leftHit =  AABBIntersect(texelFetch(BVHTex, leftIndex  * 3 + 0).xyz, texelFetch(BVHTex, leftIndex  * 3 + 1).xyz, rTrans);
            rightHit = AABBIntersect(texelFetch(BVHTex, rightIndex * 3 + 0).xyz, texelFetch(BVHTex, rightIndex * 3 + 1).xyz, rTrans);
            
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
void GetMaterial(inout HitRec hit_record, in Ray r)
{
    // 获取材质信息
    // 并通过光线计算光照颜色
    // TODO: 这里需要根据实际的材质类包含的内容进行调整
    int index = hit_record.matID * 8; // Material类有7块
    Material mat;
    // 材质默认值
    // 灰色（默认非金属材质）
    vec3 defaultAlbedo = vec3(0.1804, 0.6745, 0.7294);   // 默认反射率：灰色
    float defaultMetallic = 0.0;                // 默认金属度：非金属
    float defaultRoughness = 0.5;               // 默认粗糙度：中等
    float defaultAO = 1.0;                      // 默认 AO 值：完全无遮蔽
    // 水体材质的参数设置
    vec3 defaultAlbedoWater = vec3(0.1, 0.3, 0.5);  // 水的反射率：轻微蓝色
    float defaultMetallicWater = 0.0;               // 水体是非金属
    float defaultRoughnessWater = 0.1;              // 水体表面非常平滑
    float defaultAOWater = 1.0;                     // 水体的环境遮蔽值，假设水面平坦无遮挡
    // 铝材质（Metal）
    vec3 defaultAlbedoMetal = vec3(0.8, 0.85, 0.9);  // 默认铝的反射率（银白色）
    float defaultMetallicMetal = 1.0;                // 完全金属
    float defaultRoughnessMetal = 0.1;               // 低粗糙度
    float defaultAOMetal = 1.0;                      // 完全无遮蔽
    // 木材材质
    vec3 defaultAlbedoWood = vec3(0.5, 0.25, 0.1);   // 默认木材反射率（棕色）
    float defaultMetallicWood = 0.0;                 // 非金属
    float defaultRoughnessWood = 0.7;                // 高粗糙度
    float defaultAOWood = 1.0;                       // 完全无遮蔽
    // 云材料
    vec3 defaultAlbedoCloud = vec3(0.9, 0.9, 0.9);   // 默认云的反射率（白色）
    float defaultMetallicCloud = 0.0;                // 非金属
    float defaultRoughnessCloud = 0.8;               // 高粗糙度
    float defaultAOCloud = 1.0;                      // 完全无遮蔽
    vec3 defaultemissiveColor = vec3(0.5, 0.5, 0.5); // 默认发光颜色
    float defaultior = 1.0;                          // 默认折射率
    float defaultalpha = 0.5;                        // 默认透明度

    // 默认设置
    mat.normal = hit_record.normal;
    mat.F0 = vec3(0.04);
    // 获取材质的信息
    vec4 param1 = texelFetch(materialsTex, index + 0);  // 获取第一个材质参数
    vec4 param2 = texelFetch(materialsTex, index + 1);  // 获取第二个材质参数
    vec4 param3 = texelFetch(materialsTex, index + 2);  // 获取第三个材质参数
    vec4 param4 = texelFetch(materialsTex, index + 3);  // 获取第四个材质参数
    vec4 param5 = texelFetch(materialsTex, index + 4);  // 获取第五个材质参数
    vec4 param6 = texelFetch(materialsTex, index + 5);  // 获取第六个材质参数
    vec4 param7 = texelFetch(materialsTex, index + 6);  // 获取第七个材质参数
    vec4 param8 = texelFetch(materialsTex, index + 7);  // 获取第八个材质参数

    // 获取材质的基本颜色
    mat.baseColor = param1.rgb;
    mat.specular = param2.rgb;
    mat.emission = param3.rgb;
    hit_record.emission = mat.emission;

    // 获取材质的参数
    vec4 para_1  = param4;
    vec4 para_2  = param5;
    mat.ior = para_1.x; // 折射率
    mat.alpha = para_1.y;// 透明度
    mat.roughness = para_1.w;// 粗糙度
    mat.matellic = para_2.x;// 金属度
    mat.scatter = para_2.y;// 散射
    mat.coat = para_2.z;// 涂层
    mat.coatRough = para_2.w;// 涂层粗糙度
    mat.absorption = param8.r;// 吸收率
    mat.density = param8.g;// 透射率
    mat.anisotropy = param8.b;// 各向异性
    mat.isVolume = param8.a;// 各向异性旋转
    
    // 获取纹理相应的索引
    ivec4 texIDs_1  = ivec4(param6);
    ivec4 texIDs_2  = ivec4(param7);
    // 根据纹理图片修正材质的数据
    if(texIDs_1.x >= 0) // diffuseTexId
    {
        vec4 col = texture(textureMapsArrayTex, vec3(hit_record.texCoord, texIDs_1.x));
        mat.baseColor.rgb = col.rgb;
    }
    if(texIDs_1.y >= 0) // specularTexId
    {
        vec4 col = texture(textureMapsArrayTex, vec3(hit_record.texCoord, texIDs_1.y));
        mat.specular.rgb = col.rgb;
    }
    if(texIDs_1.z >= 0) // normalTexId
    {
        vec3 tangentNormal  = texture(textureMapsArrayTex, vec3(hit_record.texCoord, texIDs_1.z)).xyz * 2.0 - 1.0;
        mat.normal = LocalToWorld(tangentNormal, hit_record.normal);
    }
    if(texIDs_1.w >= 0) // heightTexId
    {
    }
    if(texIDs_2.x >= 0) // metalnessTexId
    {
        vec4 metallic = texture(textureMapsArrayTex, vec3(hit_record.texCoord, texIDs_2.x));
        mat.matellic = metallic.r;
    }
    if(texIDs_2.y >= 0) // diffuse_roughnessTexId
    {
        vec4 roughness = texture(textureMapsArrayTex, vec3(hit_record.texCoord, texIDs_2.y));
        mat.roughness = roughness.r;
    }
    if(texIDs_2.z >= 0) // ambient_occlusionTexId
    {
        vec4 ao = texture(textureMapsArrayTex, vec3(hit_record.texCoord, texIDs_2.z));
        mat.ao = ao.r;
    }
    hit_record.mat = mat;
}
/********************************/
// 局部坐标系与世界坐标系变换
// 基于表面法线位置
/********************************/
void GenerateTBN(in vec3 N, inout vec3 T, inout vec3 B)
{
    // 计算给定法线向量 N 的正交基底
    vec3 up = abs(N.z) < 0.9999999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
    T = normalize(cross(up, N));  // 第一个正交向量
    B = cross(N, T);              // 第二个正交向量
}
vec3 ToWorld(vec3 X, vec3 Y, vec3 Z, vec3 V)
{
    return V.x * X + V.y * Y + V.z * Z;
}
vec3 ToLocal(vec3 X, vec3 Y, vec3 Z, vec3 V)
{
    return vec3(dot(V, X), dot(V, Y), dot(V, Z));
}
vec3 LocalToWorld(vec3 localDir, vec3 normal)
{
    vec3 T, B;
    GenerateTBN(normal, T, B); // 基于法线生成切线 T 和副切线 B
    return ToWorld(T, B, normal, localDir); // 转换到全局坐标系
}
vec3 WorldToLocal(vec3 globalDir, vec3 normal)
{
    vec3 T, B;
    GenerateTBN(normal, T, B); // 基于法线生成切线 T 和副切线 B
    return ToLocal(T, B, normal, globalDir); // 转换到局部坐标系
}
/********************************/
// 采样函数 -- 散射方向
/********************************/
vec3 SampleDirection(vec3 normal, bool useCosineWeighted)
{
    vec3 localDir;
    
    float r1 = rand();
    float r2 = rand();
    
    // 基于是否加权采样选择采样方式
    if (useCosineWeighted) {
        // 使用余弦加权半球采样
        localDir = CosineSampleHemisphere(r1, r2);
    } else {
        // 使用均匀半球采样
        localDir = UniformSampleHemisphere(r1, r2);
    }
    
    return LocalToWorld(localDir, normal);
}
vec3 UniformSampleHemisphere(float r1, float r2)
{
    // --------------------- //
    // 函数名称：Uniform_Sample_Hemisphere 均匀半球采样
    // 函数功能:
    // 根据给定的随机数生成一个单位半球上的随机方向
    // 采样的方向在半球上分布是均匀的，即所有方向具有相同的概率密度
    // 随机数 r1 控制 z
    // 随机数 r2 控制 x y
    // PDF = 1 / 2 * PI
    // --------------------- //
    float r = sqrt(max(0.0, 1.0 - r1 * r1));
    float phi = TWO_PI * r2;
    
    return vec3(r * cos(phi), r * sin(phi), r1);
}
vec3 CosineSampleHemisphere(float r1, float r2)
{
    // --------------------- //
    // 函数名称：Cosine_Sample_Hemisphere 余弦加权半球采样
    // 函数功能:
    // 根据给定的随机数生成一个单位半球上的随机方向
    // 采样方向更倾向于表面法线附近的方向，符合漫反射的余弦分布特性
    // 随机数 r1 控制 θ 角度
    // 随机数 r2 控制 ϕ 角度
    // PDF = cos(θ) / PI
    // --------------------- //
    
    float r = sqrt(r1);
    float phi = TWO_PI * r2;
    
    float x = r * cos(phi);                         // x 分量
    float y = r * sin(phi);                         // y 分量
    float z = sqrt(max(0.0, 1.0 - x * x - y * y));  // z 分量
    
    return vec3(x, y, z);
}
/********************************/
// 采样函数 -- 光源方向
/********************************/
void sampleLight(vec3 hitPoint,inout LightSampleRec lightSample)
{
    int lightIndex=int(rand()*float(numOfSphereLights));
    SampleSphereLight(sphereLights[lightIndex],hitPoint,lightSample);
}
void SampleRectLight(in RectLight light,in vec3 hitPoint,inout LightSampleRec lightSample)
{
    // 矩形光源
    float r1=rand();
    float r2=rand();
    
    vec3 lightSurfacePos=light.position+light.u*r1+light.v*r2;// 在矩形上随机采样一个点
    lightSample.direction=lightSurfacePos-hitPoint;
    lightSample.dist=length(lightSample.direction);
    float distSq=lightSample.dist*lightSample.dist;
    lightSample.direction/=lightSample.dist;
    lightSample.normal=normalize(cross(light.u,light.v));
    lightSample.emission=light.emission*float(numOfRectLights);
    lightSample.pdf=distSq/(light.area*abs(dot(lightSample.normal,lightSample.direction)));
}
void SampleSphereLight(in SphereLight light,in vec3 hitPoint,inout LightSampleRec lightSample)
{
    // 球形光源
    float r1=rand();
    float r2=rand();
    
    vec3 sphereCentertoSurface=hitPoint-light.position;
    float distToSphereCenter=length(sphereCentertoSurface);
    vec3 sampledDir;
    
    // TODO: Fix this. Currently assumes the light will be hit only from the outside
    sphereCentertoSurface/=distToSphereCenter;
    // 在均匀球上随机采样一个点
    sampledDir=UniformSampleHemisphere(r1,r2);
    // 通过 Onb 将半球采样结果从局部坐标系转换到球光源的方向
    vec3 T,B;
    GenerateTBN(sphereCentertoSurface,T,B);
    sampledDir=T*sampledDir.x+B*sampledDir.y+sphereCentertoSurface*sampledDir.z;
    
    vec3 lightSurfacePos=light.position+sampledDir*light.radius;// 计算采样点的位置
    
    lightSample.direction=lightSurfacePos-hitPoint;
    lightSample.dist=length(lightSample.direction);
    // float distSq=lightSample.dist*lightSample.dist;
    float distSq = lightSample.dist;
    
    lightSample.direction/=lightSample.dist;
    lightSample.normal=normalize(lightSurfacePos-light.position);
    lightSample.emission=light.emission*float(numOfSphereLights);
    lightSample.pdf=distSq/(light.area*.5*abs(dot(lightSample.normal,lightSample.direction)));
}
void SampleDistantLight(in DistantLight light,in vec3 hitPoint,inout LightSampleRec lightSample)
{
    // 平行光源
    lightSample.direction=normalize(light.position-vec3(0.));
    lightSample.normal=normalize(hitPoint-light.position);
    lightSample.emission=light.emission*float(numOfDistantLights);
    lightSample.dist=INF;
    lightSample.pdf=1.;
}
