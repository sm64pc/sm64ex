//
// RT64
//

#ifndef RT64_H_INCLUDED
#define RT64_H_INCLUDED

#include <Windows.h>
#include <stdio.h>

// Material constants.
#define RT64_MATERIAL_FILTER_POINT				0
#define RT64_MATERIAL_FILTER_LINEAR				1
#define RT64_MATERIAL_ADDR_WRAP					0
#define RT64_MATERIAL_ADDR_MIRROR				1
#define RT64_MATERIAL_ADDR_CLAMP				2
#define RT64_MATERIAL_CC_SHADER_0				0
#define RT64_MATERIAL_CC_SHADER_INPUT_1			1
#define RT64_MATERIAL_CC_SHADER_INPUT_2			2
#define RT64_MATERIAL_CC_SHADER_INPUT_3			3
#define RT64_MATERIAL_CC_SHADER_INPUT_4			4
#define RT64_MATERIAL_CC_SHADER_TEXEL0			5
#define RT64_MATERIAL_CC_SHADER_TEXEL0A			6
#define RT64_MATERIAL_CC_SHADER_TEXEL1			7

// Material attributes.
#define RT64_ATTRIBUTE_NONE						0x0
#define RT64_ATTRIBUTE_IGNORE_NORMAL_FACTOR		0x1
#define RT64_ATTRIBUTE_NORMAL_MAP_SCALE			0x2
#define RT64_ATTRIBUTE_REFLECTION_FACTOR		0x4
#define RT64_ATTRIBUTE_REFLECTION_SHINE_FACTOR	0x8
#define RT64_ATTRIBUTE_REFRACTION_FACTOR		0x10
#define RT64_ATTRIBUTE_SPECULAR_INTENSITY		0x20
#define RT64_ATTRIBUTE_SPECULAR_EXPONENT		0x40
#define RT64_ATTRIBUTE_SOLID_ALPHA_MULTIPLIER	0x80
#define RT64_ATTRIBUTE_SHADOW_ALPHA_MULTIPLIER	0x100
#define RT64_ATTRIBUTE_SELF_LIGHT				0x200
#define RT64_ATTRIBUTE_LIGHT_GROUP_MASK_BITS	0x400
#define RT64_ATTRIBUTE_DIFFUSE_COLOR_MIX		0x800

// Mesh flags.
#define RT64_MESH_RAYTRACE_ENABLED				0x1
#define RT64_MESH_RAYTRACE_UPDATABLE			0x2

// Instance flags.
#define RT64_INSTANCE_RASTER_BACKGROUND			0x1
#define RT64_INSTANCE_DISABLE_BACKFACE_CULLING	0x2

// Light flags.
#define RT64_LIGHT_GROUP_MASK_ALL				0xFFFFFFFF
#define RT64_LIGHT_GROUP_DEFAULT				0x1
#define RT64_LIGHT_MAX_SAMPLES					128

typedef struct {
	float x, y;
} RT64_VECTOR2;

typedef struct {
	float x, y, z;
} RT64_VECTOR3;

typedef struct {
	float x, y, z, w;
} RT64_VECTOR4;

typedef struct {
	float m[4][4];
} RT64_MATRIX4;

typedef struct {
	RT64_VECTOR3 position;
	RT64_VECTOR3 normal;
	RT64_VECTOR2 uv;
	RT64_VECTOR4 inputs[4];
} RT64_VERTEX;

typedef struct {
	int filterMode;
	int diffuseTexIndex;
	int normalTexIndex;
	int hAddressMode;
	int vAddressMode;
	float ignoreNormalFactor;
	float normalMapScale;
	float reflectionFactor;
	float reflectionShineFactor;
	float refractionFactor;
	float specularIntensity;
	float specularExponent;
	float solidAlphaMultiplier;
	float shadowAlphaMultiplier;
	RT64_VECTOR3 selfLight;
	unsigned int lightGroupMaskBits;
	RT64_VECTOR3 fogColor;
	RT64_VECTOR4 diffuseColorMix;
	float fogMul;
	float fogOffset;
	int _padA;

	// N64 Color combiner parameters.
	int c0[4];
	int c1[4];
	int do_single[2];
	int do_multiply[2];
	int do_mix[2];
	int color_alpha_same;
	int opt_alpha;
	int opt_fog;
	int opt_texture_edge;

	// Flag containing all attributes that are actually used by this material.
	int enabledAttributes;

	// Add padding to line up with the HLSL structure.
	int _padB;
} RT64_MATERIAL;

// Light
typedef struct {
	RT64_VECTOR3 position;
	RT64_VECTOR3 diffuseColor;
	float attenuationRadius;
	float pointRadius;
	float specularIntensity;
	float shadowOffset;
	float attenuationExponent;
	float flickerIntensity;
	unsigned int groupBits;
	unsigned int minSamples;
	unsigned int maxSamples;
} RT64_LIGHT;


// Forward declaration of types.
typedef struct RT64_DEVICE RT64_DEVICE;
typedef struct RT64_VIEW RT64_VIEW;
typedef struct RT64_SCENE RT64_SCENE;
typedef struct RT64_INSTANCE RT64_INSTANCE;
typedef struct RT64_MESH RT64_MESH;
typedef struct RT64_TEXTURE RT64_TEXTURE;
typedef struct RT64_INSPECTOR RT64_INSPECTOR;

inline void RT64_ApplyMaterialAttributes(RT64_MATERIAL *dst, RT64_MATERIAL *src) {
	if (src->enabledAttributes & RT64_ATTRIBUTE_IGNORE_NORMAL_FACTOR) {
		dst->ignoreNormalFactor = src->ignoreNormalFactor;
	}

	if (src->enabledAttributes & RT64_ATTRIBUTE_NORMAL_MAP_SCALE) {
		dst->normalMapScale = src->normalMapScale;
	}

	if (src->enabledAttributes & RT64_ATTRIBUTE_REFLECTION_FACTOR) {
		dst->reflectionFactor = src->reflectionFactor;
	}

	if (src->enabledAttributes & RT64_ATTRIBUTE_REFLECTION_SHINE_FACTOR) {
		dst->reflectionShineFactor = src->reflectionShineFactor;
	}

	if (src->enabledAttributes & RT64_ATTRIBUTE_REFRACTION_FACTOR) {
		dst->refractionFactor = src->refractionFactor;
	}

	if (src->enabledAttributes & RT64_ATTRIBUTE_SPECULAR_INTENSITY) {
		dst->specularIntensity = src->specularIntensity;
	}

	if (src->enabledAttributes & RT64_ATTRIBUTE_SPECULAR_EXPONENT) {
		dst->specularExponent = src->specularExponent;
	}

	if (src->enabledAttributes & RT64_ATTRIBUTE_SOLID_ALPHA_MULTIPLIER) {
		dst->solidAlphaMultiplier = src->solidAlphaMultiplier;
	}

	if (src->enabledAttributes & RT64_ATTRIBUTE_SHADOW_ALPHA_MULTIPLIER) {
		dst->shadowAlphaMultiplier = src->shadowAlphaMultiplier;
	}

	if (src->enabledAttributes & RT64_ATTRIBUTE_SELF_LIGHT) {
		dst->selfLight = src->selfLight;
	}

	if (src->enabledAttributes & RT64_ATTRIBUTE_LIGHT_GROUP_MASK_BITS) {
		dst->lightGroupMaskBits = src->lightGroupMaskBits;
	}

	if (src->enabledAttributes & RT64_ATTRIBUTE_DIFFUSE_COLOR_MIX) {
		dst->diffuseColorMix = src->diffuseColorMix;
	}
}

// Internal function pointer types.
typedef RT64_DEVICE* (*CreateDevicePtr)(void *hwnd);
typedef void(*DrawDevicePtr)(RT64_DEVICE* device, int vsyncInterval);
typedef void(*DestroyDevicePtr)(RT64_DEVICE* device);
typedef RT64_VIEW* (*CreateViewPtr)(RT64_SCENE* scenePtr);
typedef void(*SetViewPerspectivePtr)(RT64_VIEW* viewPtr, RT64_VECTOR3 eyePosition, RT64_VECTOR3 eyeFocus, RT64_VECTOR3 eyeUpDirection, float fovRadians, float nearDist, float farDist);
typedef void(*DestroyViewPtr)(RT64_VIEW* viewPtr);
typedef RT64_SCENE* (*CreateScenePtr)(RT64_DEVICE* devicePtr);
typedef void (*SetSceneLightsPtr)(RT64_SCENE* scenePtr, RT64_LIGHT* lightArray, int lightCount);
typedef void(*DestroyScenePtr)(RT64_SCENE* scenePtr);
typedef RT64_MESH* (*CreateMeshPtr)(RT64_DEVICE* devicePtr, int flags);
typedef void (*SetMeshPtr)(RT64_MESH* meshPtr, RT64_VERTEX* vertexArray, int vertexCount, unsigned int* indexArray, int indexCount);
typedef void (*DestroyMeshPtr)(RT64_MESH* meshPtr);
typedef RT64_INSTANCE* (*CreateInstancePtr)(RT64_SCENE* scenePtr);
typedef void (*SetInstancePtr)(RT64_INSTANCE* instancePtr, RT64_MESH* meshPtr, RT64_MATRIX4 transform, RT64_TEXTURE* diffuseTexture, RT64_TEXTURE* normalTexture, RT64_MATERIAL material, unsigned int flags);
typedef void (*DestroyInstancePtr)(RT64_INSTANCE* instancePtr);
typedef RT64_TEXTURE* (*CreateTextureFromRGBA8Ptr)(RT64_DEVICE* devicePtr, const void* bytes, int width, int height, int stride);
typedef void(*DestroyTexturePtr)(RT64_TEXTURE* texture);
typedef RT64_INSPECTOR* (*CreateInspectorPtr)(RT64_DEVICE* devicePtr);
typedef bool(*HandleMessageInspectorPtr)(RT64_INSPECTOR* inspectorPtr, UINT msg, WPARAM wParam, LPARAM lParam);
typedef void (*SetMaterialInspectorPtr)(RT64_INSPECTOR* inspectorPtr, RT64_MATERIAL* material);
typedef void(*SetLightsInspectorPtr)(RT64_INSPECTOR* inspectorPtr, RT64_LIGHT* lights, int *lightCount, int maxLightCount);
typedef void(*PrintToInspectorPtr)(RT64_INSPECTOR* inspectorPtr, const char* message);
typedef void(*DestroyInspectorPtr)(RT64_INSPECTOR* inspectorPtr);

// Stores all the function pointers used in the RT64 library.
typedef struct {
	HMODULE handle;
	CreateDevicePtr CreateDevice;
	DrawDevicePtr DrawDevice;
	DestroyDevicePtr DestroyDevice;
	CreateViewPtr CreateView;
	SetViewPerspectivePtr SetViewPerspective;
	DestroyViewPtr DestroyView;
	CreateScenePtr CreateScene;
	SetSceneLightsPtr SetSceneLights;
	DestroyScenePtr DestroyScene;
	CreateMeshPtr CreateMesh;
	SetMeshPtr SetMesh;
	DestroyMeshPtr DestroyMesh;
	CreateInstancePtr CreateInstance;
	SetInstancePtr SetInstance;
	DestroyInstancePtr DestroyInstance;
	CreateTextureFromRGBA8Ptr CreateTextureFromRGBA8;
	DestroyTexturePtr DestroyTexture;
	CreateInspectorPtr CreateInspector;
	HandleMessageInspectorPtr HandleMessageInspector;
	PrintToInspectorPtr PrintToInspector;
	SetMaterialInspectorPtr SetMaterialInspector;
	SetLightsInspectorPtr SetLightsInspector;
	DestroyInspectorPtr DestroyInspector;
} RT64_LIBRARY;


// Define RT64_DEBUG for loading the debug DLL.
inline RT64_LIBRARY RT64_LoadLibrary() {
	RT64_LIBRARY lib;
#ifdef RT64_DEBUG
	lib.handle = LoadLibrary(TEXT("rt64libd.dll"));
#else
	lib.handle = LoadLibrary(TEXT("rt64lib.dll"));
#endif
	if (lib.handle != 0) {
		lib.CreateDevice = (CreateDevicePtr)(GetProcAddress(lib.handle, "RT64_CreateDevice"));
		lib.DrawDevice = (DrawDevicePtr)(GetProcAddress(lib.handle, "RT64_DrawDevice"));
		lib.DestroyDevice = (DestroyDevicePtr)(GetProcAddress(lib.handle, "RT64_DestroyDevice"));
		lib.CreateView = (CreateViewPtr)(GetProcAddress(lib.handle, "RT64_CreateView"));
		lib.SetViewPerspective = (SetViewPerspectivePtr)(GetProcAddress(lib.handle, "RT64_SetViewPerspective"));
		lib.DestroyView = (DestroyViewPtr)(GetProcAddress(lib.handle, "RT64_DestroyView"));
		lib.CreateScene = (CreateScenePtr)(GetProcAddress(lib.handle, "RT64_CreateScene"));
		lib.SetSceneLights = (SetSceneLightsPtr)(GetProcAddress(lib.handle, "RT64_SetSceneLights"));
		lib.DestroyScene = (DestroyScenePtr)(GetProcAddress(lib.handle, "RT64_DestroyScene"));
		lib.CreateMesh = (CreateMeshPtr)(GetProcAddress(lib.handle, "RT64_CreateMesh"));
		lib.SetMesh = (SetMeshPtr)(GetProcAddress(lib.handle, "RT64_SetMesh"));
		lib.DestroyMesh = (DestroyMeshPtr)(GetProcAddress(lib.handle, "RT64_DestroyMesh"));
		lib.CreateInstance = (CreateInstancePtr)(GetProcAddress(lib.handle, "RT64_CreateInstance"));
		lib.SetInstance = (SetInstancePtr)(GetProcAddress(lib.handle, "RT64_SetInstance"));
		lib.DestroyInstance = (DestroyInstancePtr)(GetProcAddress(lib.handle, "RT64_DestroyInstance"));
		lib.CreateTextureFromRGBA8 = (CreateTextureFromRGBA8Ptr)(GetProcAddress(lib.handle, "RT64_CreateTextureFromRGBA8"));
		lib.DestroyTexture = (DestroyTexturePtr)(GetProcAddress(lib.handle, "RT64_DestroyTexture"));
		lib.CreateInspector = (CreateInspectorPtr)(GetProcAddress(lib.handle, "RT64_CreateInspector"));
		lib.HandleMessageInspector = (HandleMessageInspectorPtr)(GetProcAddress(lib.handle, "RT64_HandleMessageInspector"));
		lib.SetMaterialInspector = (SetMaterialInspectorPtr)(GetProcAddress(lib.handle, "RT64_SetMaterialInspector"));
		lib.SetLightsInspector = (SetLightsInspectorPtr)(GetProcAddress(lib.handle, "RT64_SetLightsInspector"));
		lib.PrintToInspector = (PrintToInspectorPtr)(GetProcAddress(lib.handle, "RT64_PrintToInspector"));
		lib.DestroyInspector = (DestroyInspectorPtr)(GetProcAddress(lib.handle, "RT64_DestroyInspector"));
	}

	return lib;
}

inline void RT64_UnloadLibrary(RT64_LIBRARY lib) {
	FreeLibrary(lib.handle);
}

#endif