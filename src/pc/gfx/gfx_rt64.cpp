#ifdef RAPI_RT64

#if defined(_WIN32) || defined(_WIN64)

#if !defined(EXTERNAL_DATA) && !defined(RENDER_96_ALPHA)
#error "RT64 requires EXTERNAL_DATA to be enabled."
#endif

extern "C" {
#	include "../../game/area.h"
#	include "../../game/level_update.h"
#	include "../fs/fs.h"
#	include "../pc_main.h"
#	include "gfx_cc.h"
}

#include <cassert>
#include <fstream>
#include <iomanip>
#include <stdint.h>
#include <string>
#include <unordered_map>

#include "json/json.hpp"
#include "xxhash/xxhash64.h"

using json = nlohmann::json;

#include "gfx_rt64.h"
#include "gfx_rt64_geo_map.h"
#include "rt64/rt64.h"

#ifndef _LANGUAGE_C
# define _LANGUAGE_C
#endif
#include <PR/gbi.h>

#include <Windows.h>

#define MAX_GEO_LAYOUT_STACK_SIZE		32
#define CACHED_MESH_REQUIRED_FRAMES		3
#define CACHED_MESH_LIFETIME			900
#define CACHED_MESH_MAX_PER_FRAME		1
#define DYNAMIC_MESH_LIFETIME			30
#define MAX_INSTANCES					1024
#define MAX_LIGHTS						512
#define MAX_LEVELS						39
#define MAX_AREAS						3
#define MAX_LEVEL_LIGHTS				128
#define LEVEL_LIGHTS_FILENAME			FS_BASEDIR "/rt64/level_lights.json"
#define LIGHT_SAMPLE_PRESETS_FILENAME	FS_BASEDIR "/rt64/light_sample_presets.json"
#define GEO_LAYOUT_MODS_FILENAME		FS_BASEDIR "/rt64/geo_layout_mods.json"
#define TEXTURE_MODS_FILENAME			FS_BASEDIR "/rt64/texture_mods.json"
#define LIGHT_SAMPLE_PRESET_DEFAULT		"Simple"

struct ShaderProgram {
    uint32_t shader_id;
    uint8_t num_inputs;
    uint8_t num_floats;
    bool used_textures[2];
};

struct RecordedMesh {
    RT64_MESH *mesh;
    uint32_t vertexCount;
    uint32_t indexCount;
    int lifetime;
    bool inUse;
    bool raytraceable;
};

struct RecordedMeshKey {
	int counter;
    bool seen;
};

struct RecordedTexture {
	RT64_TEXTURE *texture;
	bool linearFilter;
	uint32_t cms;
	uint32_t cmt;
	uint64_t hash;
};

struct RecordedMod {
    RT64_MATERIAL *materialMod;
    RT64_LIGHT *lightMod;
	uint64_t normalMapHash;
};

//	Convention of bits for different lights.
//	The tiers allow more detailed control over the performance of areas of the game that are more demanding than others.
//		1 	- Directional Tier A
//		2 	- Directional Tier B
//		4 	- Stage Tier A 
//		8 	- Stage Tier B
//		16 	- Objects Tier A
//		32 	- Objects Tier B
//		64 	- Particles Tier A
//		128 - Particles Tier B

struct LightSampleSetting {
	unsigned int groupBits;
	unsigned int minSamples;
	unsigned int maxSamples;
};

struct LightSamplePreset {
	std::vector<LightSampleSetting> settings;
};

struct {
	HWND hwnd;
	
	// Library data.
	RT64_LIBRARY lib;
	RT64_DEVICE *device;
	RT64_INSPECTOR *inspector;
	RT64_SCENE *scene;
	RT64_VIEW *view;
	RT64_MATERIAL defaultMaterial;
	RT64_TEXTURE *blankTexture;
	RT64_INSTANCE *instances[MAX_INSTANCES];
	int instanceCount;
	int instanceAllocCount;
	std::unordered_map<uint32_t, uint64_t> textureHashIdMap;
	std::unordered_map<uint32_t, RecordedTexture> textures;
	std::unordered_map<uint64_t, RecordedMesh> staticMeshes;
	std::unordered_map<uint64_t, RecordedMesh> dynamicMeshes;
	std::unordered_map<uint64_t, RecordedMeshKey> dynamicMeshKeys;
	std::unordered_map<uint32_t, ShaderProgram *> shaderPrograms;
	LightSamplePreset activeLightSamplePreset;
	std::map<std::string, LightSamplePreset> lightSamplePresets;
	int cachedMeshesPerFrame;
	RT64_LIGHT lights[MAX_LIGHTS];
    unsigned int lightCount;
	RT64_LIGHT levelLights[MAX_LEVELS][MAX_AREAS][MAX_LEVEL_LIGHTS];
	int levelLightCounts[MAX_LEVELS][MAX_AREAS];

	// Ray picking data.
	bool pickTextureNextFrame;
	bool pickTextureHighlight;
	uint64_t pickedTextureHash;
	std::unordered_map<RT64_INSTANCE *, uint64_t> lastInstanceTextureHashes;

	// Geo layout mods.
	void *geoLayoutStack[MAX_GEO_LAYOUT_STACK_SIZE];
	int geoLayoutStackSize;
	std::unordered_map<void *, std::string> geoLayoutNameMap;
	std::map<std::string, void *> nameGeoLayoutMap;
	std::unordered_map<void *, RecordedMod *> geoLayoutMods;
	std::unordered_map<void *, RecordedMod *> graphNodeMods;
	
	// Texture mods.
	std::unordered_map<uint64_t, std::string> texNameMap;
	std::map<std::string, uint64_t> nameTexMap;
	std::unordered_map<uint64_t, RecordedMod *> texMods;

	// Camera.
	RT64_MATRIX4 viewMatrix;
    float fovRadians;
    float nearDist;
    float farDist;

	// Matrices.
	RT64_MATRIX4 identityTransform;

	// Rendering state.
	int currentTile;
    uint32_t currentTextureIds[2];
	ShaderProgram *shaderProgram;
	bool background;
	RT64_VECTOR3 fogColor;
	int16_t fogMul;
	int16_t fogOffset;
	RecordedMod *graphNodeMod;

	// Timing.
	LARGE_INTEGER StartingTime, EndingTime;
	LARGE_INTEGER Frequency;
	bool dropNextFrame;

	// Function pointers for game.
    void (*run_one_game_iter)(void);
    bool (*on_key_down)(int scancode);
    bool (*on_key_up)(int scancode);
    void (*on_all_keys_up)(void);
} RT64;

static inline size_t string_hash(const uint8_t *str) {
    size_t h = 0;
    for (const uint8_t *p = str; *p; p++)
        h = 31 * h + *p;
    return h;
}

uint64_t gfx_rt64_get_texture_name_hash(const std::string &name) {
	uint64_t hash = string_hash((const uint8_t *)(name.c_str()));
	RT64.texNameMap[hash] = name;
	RT64.nameTexMap[name] = hash;
	return hash;
}

void gfx_rt64_load_light(const json &jlight, RT64_LIGHT *light) {
	light->position.x = jlight["position"][0];
	light->position.y = jlight["position"][1];
	light->position.z = jlight["position"][2];
	light->attenuationRadius = jlight["attenuationRadius"];
	light->pointRadius = jlight["pointRadius"];
	light->diffuseColor.x = jlight["diffuseColor"][0];
	light->diffuseColor.y = jlight["diffuseColor"][1];
	light->diffuseColor.z = jlight["diffuseColor"][2];
	light->specularIntensity = jlight["specularIntensity"];
	light->shadowOffset = jlight["shadowOffset"];
	light->attenuationExponent = jlight["attenuationExponent"];
	light->flickerIntensity = jlight["flickerIntensity"];
	light->groupBits = jlight["groupBits"];
}

void gfx_rt64_set_light_samples(RT64_LIGHT *light) {
	unsigned int minSamples = 1;
	unsigned int maxSamples = 1;
	for (const auto &it : RT64.activeLightSamplePreset.settings) {
		if (it.groupBits & light->groupBits) {
			minSamples = std::max(minSamples, it.minSamples);
			maxSamples = std::max(maxSamples, it.maxSamples);
		}
	}

	light->minSamples = minSamples;
	light->maxSamples = std::max(minSamples, maxSamples);
}

LightSamplePreset gfx_rt64_load_light_sample_preset(const json &jpreset) {
	LightSamplePreset preset;
	for (const json &jsetting : jpreset["lightSampleSettings"]) {
		LightSampleSetting setting;
		setting.groupBits = jsetting["groupBits"];
		setting.minSamples =jsetting["minSamples"];
		setting.maxSamples =jsetting["maxSamples"];
		preset.settings.push_back(setting);
	}

	return preset;
}

void gfx_rt64_load_light_sample_presets() {
	std::ifstream i(LIGHT_SAMPLE_PRESETS_FILENAME);
	if (i.is_open()) {
		json j;
		i >> j;

		for (const json &jpreset : j["presets"]) {
			const std::string name = jpreset["name"];
			RT64.lightSamplePresets[name] = gfx_rt64_load_light_sample_preset(jpreset);
		}
	}
	else {
		fprintf(stderr, "Unable to load " LIGHT_SAMPLE_PRESETS_FILENAME ". Defaulting to hard shadows only.\n");
	}
}

uint64_t gfx_rt64_load_normal_map_mod(const json &jnormal) {
	return gfx_rt64_get_texture_name_hash(jnormal["name"]);
}

json gfx_rt64_save_normal_map_mod(const std::string &normalTexName) {
	json jnormal;
	jnormal["name"] = normalTexName;
	return jnormal;
}

RT64_VECTOR3 transform_position_affine(RT64_MATRIX4 m, RT64_VECTOR3 v) {
	RT64_VECTOR3 o;
	o.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
	o.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
	o.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];
	return o;
}

RT64_VECTOR3 transform_direction_affine(RT64_MATRIX4 m, RT64_VECTOR3 v) {
	RT64_VECTOR3 o;
	o.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0];
	o.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1];
	o.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2];
	return o;
}

float vector_length(RT64_VECTOR3 v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

json gfx_rt64_save_light(RT64_LIGHT *light) {
	json jlight;
	jlight["position"] = { light->position.x, light->position.y, light->position.z };
	jlight["attenuationRadius"] = light->attenuationRadius;
	jlight["pointRadius"] = light->pointRadius;
	jlight["diffuseColor"] = { light->diffuseColor.x, light->diffuseColor.y, light->diffuseColor.z };
	jlight["specularIntensity"] = light->specularIntensity;
	jlight["shadowOffset"] = light->shadowOffset;
	jlight["attenuationExponent"] = light->attenuationExponent;
	jlight["flickerIntensity"] = light->flickerIntensity;
	jlight["groupBits"] = light->groupBits;
	return jlight;
}

void gfx_rt64_load_level_lights() {
	std::ifstream i(LEVEL_LIGHTS_FILENAME);
	if (i.is_open()) {
		json j;
		i >> j;

		for (const json &jlevel : j["levels"]) {
			unsigned int l = jlevel["id"];
			assert(l < MAX_LEVELS);
			for (const json &jarea : jlevel["areas"]) {
				unsigned int a = jarea["id"];
				assert(a < MAX_AREAS);
				RT64.levelLightCounts[l][a] = 0;
				for (const json &jlight : jarea["lights"]) {
					assert(RT64.levelLightCounts[l][a] < MAX_LEVEL_LIGHTS);
					unsigned int i = RT64.levelLightCounts[l][a]++;
					RT64_LIGHT *light = &RT64.levelLights[l][a][i];
					gfx_rt64_load_light(jlight, light);
				}
			}
		}
	}
	else {
		fprintf(stderr, "Unable to load " LEVEL_LIGHTS_FILENAME ". Using default lighting.\n");
	}
}

void gfx_rt64_set_samples_level_lights() {
	for (int l = 0; l < MAX_LEVELS; l++) {
        for (int a = 0; a < MAX_AREAS; a++) {
			for (int i = 0; i < RT64.levelLightCounts[l][a]; i++) {
				gfx_rt64_set_light_samples(&RT64.levelLights[l][a][i]);
			}
		}
	}
}

void gfx_rt64_save_level_lights() {
	std::ofstream o(LEVEL_LIGHTS_FILENAME);
	if (o.is_open()) {
		json jroot;
		for (int l = 0; l < MAX_LEVELS; l++) {
			json jlevel;
			jlevel["id"] = l;

			for (int a = 0; a < MAX_AREAS; a++) {
				json jarea;
				jarea["id"] = a;
				for (int i = 0; i < RT64.levelLightCounts[l][a]; i++) {
					json jlight;
					RT64_LIGHT *light = &RT64.levelLights[l][a][i];
					jlight = gfx_rt64_save_light(light);
					jarea["lights"].push_back(jlight);
				}

				jlevel["areas"].push_back(jarea);
			}

			jroot["levels"].push_back(jlevel);
		}

		o << std::setw(4) << jroot << std::endl;

		if (o.bad()) {
			fprintf(stderr, "Error when saving " LEVEL_LIGHTS_FILENAME ".\n");
		}
		else {
			fprintf(stderr, "Saved " LEVEL_LIGHTS_FILENAME ".\n");
		}
	}
	else {
		fprintf(stderr, "Unable to save " LEVEL_LIGHTS_FILENAME ".\n");
	}
}

static void gfx_matrix_mul(float res[4][4], const float a[4][4], const float b[4][4]) {
    float tmp[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            tmp[i][j] = a[i][0] * b[0][j] +
                        a[i][1] * b[1][j] +
                        a[i][2] * b[2][j] +
                        a[i][3] * b[3][j];
        }
    }
    memcpy(res, tmp, sizeof(tmp));
}

void elapsed_time(const LARGE_INTEGER &start, const LARGE_INTEGER &end, const LARGE_INTEGER &frequency, LARGE_INTEGER &elapsed) {
	elapsed.QuadPart = end.QuadPart - start.QuadPart;
	elapsed.QuadPart *= 1000000;
	elapsed.QuadPart /= frequency.QuadPart;
}

void gfx_rt64_load_material_mod(const json &jmatmod, RT64_MATERIAL *materialMod) {
	if (jmatmod.find("ignoreNormalFactor") != jmatmod.end()) {
		materialMod->ignoreNormalFactor = jmatmod["ignoreNormalFactor"];
		materialMod->enabledAttributes |= RT64_ATTRIBUTE_IGNORE_NORMAL_FACTOR;
	}

	if (jmatmod.find("normalMapScale") != jmatmod.end()) {
		materialMod->normalMapScale = jmatmod["normalMapScale"];
		materialMod->enabledAttributes |= RT64_ATTRIBUTE_NORMAL_MAP_SCALE;
	}

	if (jmatmod.find("reflectionFactor") != jmatmod.end()) {
		materialMod->reflectionFactor = jmatmod["reflectionFactor"];
		materialMod->enabledAttributes |= RT64_ATTRIBUTE_REFLECTION_FACTOR;
	}

	if (jmatmod.find("reflectionFresnelFactor") != jmatmod.end()) {
		materialMod->reflectionFresnelFactor = jmatmod["reflectionFresnelFactor"];
		materialMod->enabledAttributes |= RT64_ATTRIBUTE_REFLECTION_FRESNEL_FACTOR;
	}

	if (jmatmod.find("reflectionShineFactor") != jmatmod.end()) {
		materialMod->reflectionShineFactor = jmatmod["reflectionShineFactor"];
		materialMod->enabledAttributes |= RT64_ATTRIBUTE_REFLECTION_SHINE_FACTOR;
	}

	if (jmatmod.find("refractionFactor") != jmatmod.end()) {
		materialMod->refractionFactor = jmatmod["refractionFactor"];
		materialMod->enabledAttributes |= RT64_ATTRIBUTE_REFRACTION_FACTOR;
	}

	if (jmatmod.find("specularIntensity") != jmatmod.end()) {
		materialMod->specularIntensity = jmatmod["specularIntensity"];
		materialMod->enabledAttributes |= RT64_ATTRIBUTE_SPECULAR_INTENSITY;
	}

	if (jmatmod.find("specularExponent") != jmatmod.end()) {
		materialMod->specularExponent = jmatmod["specularExponent"];
		materialMod->enabledAttributes |= RT64_ATTRIBUTE_SPECULAR_EXPONENT;
	}

	if (jmatmod.find("solidAlphaMultiplier") != jmatmod.end()) {
		materialMod->solidAlphaMultiplier = jmatmod["solidAlphaMultiplier"];
		materialMod->enabledAttributes |= RT64_ATTRIBUTE_SOLID_ALPHA_MULTIPLIER;
	}

	if (jmatmod.find("shadowAlphaMultiplier") != jmatmod.end()) {
		materialMod->shadowAlphaMultiplier = jmatmod["shadowAlphaMultiplier"];
		materialMod->enabledAttributes |= RT64_ATTRIBUTE_SHADOW_ALPHA_MULTIPLIER;
	}
	
	if (jmatmod.find("selfLight") != jmatmod.end()) {
		materialMod->selfLight.x = jmatmod["selfLight"][0];
		materialMod->selfLight.y = jmatmod["selfLight"][1];
		materialMod->selfLight.z = jmatmod["selfLight"][2];
		materialMod->enabledAttributes |= RT64_ATTRIBUTE_SELF_LIGHT;
	}

	if (jmatmod.find("lightGroupMaskBits") != jmatmod.end()) {
		materialMod->lightGroupMaskBits = jmatmod["lightGroupMaskBits"];
		materialMod->enabledAttributes |= RT64_ATTRIBUTE_LIGHT_GROUP_MASK_BITS;
	}
	
	if (jmatmod.find("diffuseColorMix") != jmatmod.end()) {
		materialMod->diffuseColorMix.x = jmatmod["diffuseColorMix"][0];
		materialMod->diffuseColorMix.y = jmatmod["diffuseColorMix"][1];
		materialMod->diffuseColorMix.z = jmatmod["diffuseColorMix"][2];
		materialMod->diffuseColorMix.w = jmatmod["diffuseColorMix"][3];
		materialMod->enabledAttributes |= RT64_ATTRIBUTE_DIFFUSE_COLOR_MIX;
	}
}

json gfx_rt64_save_material_mod(RT64_MATERIAL *materialMod) {
	json jmatmod;
	if (materialMod->enabledAttributes & RT64_ATTRIBUTE_IGNORE_NORMAL_FACTOR) {
		jmatmod["ignoreNormalFactor"] = materialMod->ignoreNormalFactor;
	}

	if (materialMod->enabledAttributes & RT64_ATTRIBUTE_NORMAL_MAP_SCALE) {
		jmatmod["normalMapScale"] = materialMod->normalMapScale;
	}

	if (materialMod->enabledAttributes & RT64_ATTRIBUTE_REFLECTION_FACTOR) {
		jmatmod["reflectionFactor"] = materialMod->reflectionFactor;
	}

	if (materialMod->enabledAttributes & RT64_ATTRIBUTE_REFLECTION_FRESNEL_FACTOR) {
		jmatmod["reflectionFresnelFactor"] = materialMod->reflectionFresnelFactor;
	}

	if (materialMod->enabledAttributes & RT64_ATTRIBUTE_REFLECTION_SHINE_FACTOR) {
		jmatmod["reflectionShineFactor"] = materialMod->reflectionShineFactor;
	}

	if (materialMod->enabledAttributes & RT64_ATTRIBUTE_REFRACTION_FACTOR) {
		jmatmod["refractionFactor"] = materialMod->refractionFactor;
	}

	if (materialMod->enabledAttributes & RT64_ATTRIBUTE_SPECULAR_INTENSITY) {
		jmatmod["specularIntensity"] = materialMod->specularIntensity;
	}

	if (materialMod->enabledAttributes & RT64_ATTRIBUTE_SPECULAR_EXPONENT) {
		jmatmod["specularExponent"] = materialMod->specularExponent;
	}

	if (materialMod->enabledAttributes & RT64_ATTRIBUTE_SOLID_ALPHA_MULTIPLIER) {
		jmatmod["solidAlphaMultiplier"] = materialMod->solidAlphaMultiplier;
	}

	if (materialMod->enabledAttributes & RT64_ATTRIBUTE_SHADOW_ALPHA_MULTIPLIER) {
		jmatmod["shadowAlphaMultiplier"] = materialMod->shadowAlphaMultiplier;
	}

	if (materialMod->enabledAttributes & RT64_ATTRIBUTE_SELF_LIGHT) {
		jmatmod["selfLight"] = { materialMod->selfLight.x, materialMod->selfLight.y, materialMod->selfLight.z };
	}

	if (materialMod->enabledAttributes & RT64_ATTRIBUTE_LIGHT_GROUP_MASK_BITS) {
		jmatmod["lightGroupMaskBits"] = materialMod->lightGroupMaskBits;
	}

	if (materialMod->enabledAttributes & RT64_ATTRIBUTE_DIFFUSE_COLOR_MIX) {
		jmatmod["diffuseColorMix"] = { materialMod->diffuseColorMix.x, materialMod->diffuseColorMix.y, materialMod->diffuseColorMix.z, materialMod->diffuseColorMix.w };
	}

	return jmatmod;
}

void gfx_rt64_load_geo_layout_mods() {
	RT64_MATERIAL *material;
	RT64_LIGHT *light;
	gfx_rt64_init_geo_layout_maps(RT64.geoLayoutNameMap, RT64.nameGeoLayoutMap);

	std::ifstream i(GEO_LAYOUT_MODS_FILENAME);
	if (i.is_open()) {
		json j;
		i >> j;

		for (const json &jgeo : j["geoLayouts"]) {
			std::string geoName = jgeo["name"];
			void *geoLayout = RT64.nameGeoLayoutMap[geoName];
			if (geoLayout != nullptr) {
				RecordedMod *recordedMod = new RecordedMod();
				if (jgeo.find("materialMod") != jgeo.end()) {
					material = new RT64_MATERIAL();
					material->enabledAttributes = RT64_ATTRIBUTE_NONE;
					gfx_rt64_load_material_mod(jgeo["materialMod"], material);
					recordedMod->materialMod = material;
				}
				else {
					recordedMod->materialMod = nullptr;
				}
				
				if (jgeo.find("lightMod") != jgeo.end()) {
					light = new RT64_LIGHT();
					gfx_rt64_load_light(jgeo["lightMod"], light);
					recordedMod->lightMod = light;
				}
				else {
					recordedMod->lightMod = nullptr;
				}

				// Parse normal map mod.
				if (jgeo.find("normalMapMod") != jgeo.end()) {
					recordedMod->normalMapHash = gfx_rt64_load_normal_map_mod(jgeo["normalMapMod"]);
				}
				else {
					recordedMod->normalMapHash = 0;
				}

				RT64.geoLayoutMods[geoLayout] = recordedMod;
			}
			else {
				fprintf(stderr, "Error when loading " GEO_LAYOUT_MODS_FILENAME ". Geo layout %s is not recognized.\n", geoName.c_str());
			}
		}
	}
	else {
		fprintf(stderr, "Unable to load " GEO_LAYOUT_MODS_FILENAME ".\n");
	}
}

void gfx_rt64_save_geo_layout_mods() {
	std::ofstream o(GEO_LAYOUT_MODS_FILENAME);
	if (o.is_open()) {
		json jroot;
		for (const auto &pair : RT64.nameGeoLayoutMap) {
			const std::string geoName = pair.first;
			void *geoLayout = pair.second;
			auto it = RT64.geoLayoutMods.find(geoLayout);
			if (it != RT64.geoLayoutMods.end()) {
				json jgeo;
				RecordedMod *geoMod = it->second;
				jgeo["name"] = geoName;

				RT64_MATERIAL *materialMod = geoMod->materialMod;
				if (materialMod != nullptr) {
					jgeo["materialMod"] = gfx_rt64_save_material_mod(materialMod);
				}

				RT64_LIGHT *lightMod = geoMod->lightMod;
				if (lightMod != nullptr) {
					jgeo["lightMod"] = gfx_rt64_save_light(lightMod);
				}

				const std::string normName = RT64.texNameMap[geoMod->normalMapHash];
				if (!normName.empty()) {
					jgeo["normalMapMod"] = gfx_rt64_save_normal_map_mod(normName);
				}
				
				jroot["geoLayouts"].push_back(jgeo);
			}
		}

		o << std::setw(4) << jroot << std::endl;
		
		if (o.bad()) {
			fprintf(stderr, "Error when saving " GEO_LAYOUT_MODS_FILENAME ".\n");
		}
		else {
			fprintf(stderr, "Saved " GEO_LAYOUT_MODS_FILENAME ".\n");
		}
	}
	else {
		fprintf(stderr, "Unable to save " GEO_LAYOUT_MODS_FILENAME ".\n");
	}
}

void gfx_rt64_load_texture_mods() {
	RT64_MATERIAL *material;
	RT64_LIGHT *light;
	std::ifstream i(TEXTURE_MODS_FILENAME);
	if (i.is_open()) {
		json j;
		i >> j;

		for (const json &jtex : j["textures"]) {
			uint64_t texHash = gfx_rt64_get_texture_name_hash(jtex["name"]);
			RT64.texMods[texHash] = new RecordedMod();

			// Parse material mod.
			if (jtex.find("materialMod") != jtex.end()) {
				material = new RT64_MATERIAL();
				material->enabledAttributes = RT64_ATTRIBUTE_NONE;
				gfx_rt64_load_material_mod(jtex["materialMod"], material);
				RT64.texMods[texHash]->materialMod = material;
			}
			else {
				RT64.texMods[texHash]->materialMod = nullptr;
			}
			
			// Parse light mod.
			if (jtex.find("lightMod") != jtex.end()) {
				light = new RT64_LIGHT();
				gfx_rt64_load_light(jtex["lightMod"], light);
				RT64.texMods[texHash]->lightMod = light;
			}
			else {
				RT64.texMods[texHash]->lightMod = nullptr;
			}

			// Parse normal map mod.
			if (jtex.find("normalMapMod") != jtex.end()) {
				RT64.texMods[texHash]->normalMapHash = gfx_rt64_load_normal_map_mod(jtex["normalMapMod"]);
			}
			else {
				RT64.texMods[texHash]->normalMapHash = 0;
			}
		}
	}
	else {
		fprintf(stderr, "Unable to load " TEXTURE_MODS_FILENAME ".\n");
	}
}

void gfx_rt64_save_texture_mods() {
	std::ofstream o(TEXTURE_MODS_FILENAME);
	if (o.is_open()) {
		json jroot;
		for (const auto &pair : RT64.nameTexMap) {
			const std::string texName = pair.first;
			uint64_t texHash = pair.second;
			auto it = RT64.texMods.find(texHash);
			if (it != RT64.texMods.end()) {
				json jtex;
				RecordedMod *texMod = it->second;
				jtex["name"] = texName;

				RT64_MATERIAL *materialMod = texMod->materialMod;
				if (materialMod != nullptr) {
					jtex["materialMod"] = gfx_rt64_save_material_mod(materialMod);
				}

				RT64_LIGHT *lightMod = texMod->lightMod;
				if (lightMod != nullptr) {
					jtex["lightMod"] = gfx_rt64_save_light(lightMod);
				}

				const std::string normName = RT64.texNameMap[texMod->normalMapHash];
				if (!normName.empty()) {
					jtex["normalMapMod"] = gfx_rt64_save_normal_map_mod(normName);
				}

				jroot["textures"].push_back(jtex);
			}
		}

		o << std::setw(4) << jroot << std::endl;
		
		if (o.bad()) {
			fprintf(stderr, "Error when saving " TEXTURE_MODS_FILENAME ".\n");
		}
		else {
			fprintf(stderr, "Saved " TEXTURE_MODS_FILENAME ".\n");
		}
	}
	else {
		fprintf(stderr, "Unable to save " TEXTURE_MODS_FILENAME ".\n");
	}
}

static void onkeydown(WPARAM w_param, LPARAM l_param) {
    int key = ((l_param >> 16) & 0x1ff);
    if (RT64.on_key_down != nullptr) {
        RT64.on_key_down(key);
    }
}

static void onkeyup(WPARAM w_param, LPARAM l_param) {
    int key = ((l_param >> 16) & 0x1ff);
    if (RT64.on_key_up != nullptr) {
        RT64.on_key_up(key);
    }
}

LRESULT CALLBACK gfx_rt64_wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if ((RT64.inspector != nullptr) && RT64.lib.HandleMessageInspector(RT64.inspector, message, wParam, lParam)) {
		return true;
	}
	
	switch (message) {
	case WM_CLOSE:
		PostQuitMessage(0);
		game_exit();
		break;
	case WM_ACTIVATEAPP:
        if (RT64.on_all_keys_up != nullptr) {
        	RT64.on_all_keys_up();
		}

        break;
	case WM_RBUTTONDOWN:
		RT64.pickedTextureHash = 0;
		RT64.pickTextureNextFrame = true;
		RT64.pickTextureHighlight = true;
		break;
	case WM_RBUTTONUP:
		RT64.pickTextureHighlight = false;
		break;
	case WM_KEYDOWN:
		if (wParam == VK_F5) {
			gfx_rt64_save_geo_layout_mods();
			gfx_rt64_save_texture_mods();
			gfx_rt64_save_level_lights();
		}

		onkeydown(wParam, lParam);
		break;
	case WM_KEYUP:
		onkeyup(wParam, lParam);
		break;
	case WM_PAINT: {
		LARGE_INTEGER ElapsedMicroseconds;
		
		// Run one game iteration.
		if (RT64.run_one_game_iter != nullptr) {
			LARGE_INTEGER StartTime, EndTime;
			QueryPerformanceCounter(&StartTime);
    		RT64.run_one_game_iter();
			QueryPerformanceCounter(&EndTime);
			elapsed_time(StartTime, EndTime, RT64.Frequency, ElapsedMicroseconds);
			char message[64];
			sprintf(message, "FRAMETIME: %.3f ms\n", ElapsedMicroseconds.QuadPart / 1000.0);
			RT64.lib.PrintToInspector(RT64.inspector, message);
		}

		// Try to maintain the fixed framerate.
		const int FixedFramerate = 30;
		const int FramerateMicroseconds = 1000000 / FixedFramerate;
		int cyclesWaited = 0;

		// Sleep if possible to avoid busy waiting too much.
		QueryPerformanceCounter(&RT64.EndingTime);
		elapsed_time(RT64.StartingTime, RT64.EndingTime, RT64.Frequency, ElapsedMicroseconds);
		int SleepMs = ((FramerateMicroseconds - ElapsedMicroseconds.QuadPart) - 500) / 1000;
		if (SleepMs > 0) {
			Sleep(SleepMs);
			cyclesWaited++;
		}

		// Busy wait to reach the desired framerate.
		do {
			QueryPerformanceCounter(&RT64.EndingTime);
			elapsed_time(RT64.StartingTime, RT64.EndingTime, RT64.Frequency, ElapsedMicroseconds);
			cyclesWaited++;
		} while (ElapsedMicroseconds.QuadPart < FramerateMicroseconds);

		RT64.StartingTime = RT64.EndingTime;

		// Drop the next frame if we didn't wait any cycles.
		RT64.dropNextFrame = (cyclesWaited == 1);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

static void gfx_rt64_fatal(const char *window_title, const char *error_message) {
	fprintf(stderr, "%s\n", error_message);
	MessageBox(NULL, error_message, window_title, MB_OK | MB_ICONEXCLAMATION);
	abort();
}

static void gfx_rt64_wapi_init(const char *window_title) {
	// Setup library.
	RT64.lib = RT64_LoadLibrary();
	if (RT64.lib.handle == 0) {
		gfx_rt64_fatal(window_title, "Failed to load library. Please make sure rt64.dll and dxil.dll are placed next to the game's executable and are up to date.");
	}

	// Register window class.
	WNDCLASS wc;
	memset(&wc, 0, sizeof(WNDCLASS));
	wc.lpfnWndProc = gfx_rt64_wnd_proc;
	wc.hInstance = GetModuleHandle(0);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = "RT64Sample";
	RegisterClass(&wc);

	// Create window.
	const int Width = 1280;
	const int Height = 720;
	RECT rect;
	UINT dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	rect.left = (GetSystemMetrics(SM_CXSCREEN) - Width) / 2;
	rect.top = (GetSystemMetrics(SM_CYSCREEN) - Height) / 2;
	rect.right = rect.left + Width;
	rect.bottom = rect.top + Height;
	AdjustWindowRectEx(&rect, dwStyle, 0, 0);
	RT64.hwnd = CreateWindow(wc.lpszClassName, window_title, dwStyle, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0, 0, wc.hInstance, NULL);

	// Setup device.
	RT64.device = RT64.lib.CreateDevice(RT64.hwnd);
	if (RT64.device == nullptr) {
		gfx_rt64_fatal(window_title, "Failed to create device. Please make sure you have D3D12 and DXR compatible hardware.");
	}

	// Setup inspector.
	RT64.inspector = RT64.lib.CreateInspector(RT64.device);

	// Setup scene and view.
	RT64.scene = RT64.lib.CreateScene(RT64.device);
	RT64.view = RT64.lib.CreateView(RT64.scene);

	// Start timers.
	QueryPerformanceFrequency(&RT64.Frequency);
	QueryPerformanceCounter(&RT64.StartingTime);
	RT64.dropNextFrame = false;

	// Initialize other attributes.
	RT64.instanceCount = 0;
	RT64.instanceAllocCount = 0;
	RT64.geoLayoutStackSize = 0;
	RT64.cachedMeshesPerFrame = 0;
	RT64.currentTile = 0;
	memset(RT64.currentTextureIds, 0, sizeof(RT64.currentTextureIds));
	RT64.shaderProgram = nullptr;
	RT64.fogColor.x = 0.0f;
	RT64.fogColor.y = 0.0f;
	RT64.fogColor.z = 0.0f;
	RT64.fogMul = RT64.fogOffset = 0;
	RT64.pickTextureNextFrame = false;
	RT64.pickTextureHighlight = false;
	RT64.pickedTextureHash = 0;

	// Preload a blank texture.
	int blankBytesCount = 256 * 256 * 4;
	unsigned char *blankBytes = (unsigned char *)(malloc(blankBytesCount));
	memset(blankBytes, 0xFF, blankBytesCount);
	RT64.blankTexture = RT64.lib.CreateTextureFromRGBA8(RT64.device, blankBytes, 256, 256, 4);
	free(blankBytes);

	// Build identity matrix.
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			RT64.identityTransform.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}

	// Build a default material.
	RT64.defaultMaterial.filterMode = 1;
	RT64.defaultMaterial.hAddressMode = 0;
	RT64.defaultMaterial.vAddressMode = 0;
	RT64.defaultMaterial.ignoreNormalFactor = 0.0f;
    RT64.defaultMaterial.normalMapScale = 1.0f;
	RT64.defaultMaterial.reflectionFactor = 0.0f;
	RT64.defaultMaterial.reflectionFresnelFactor = 1.0f;
    RT64.defaultMaterial.reflectionShineFactor = 0.0f;
	RT64.defaultMaterial.refractionFactor = 0.0f;
	RT64.defaultMaterial.specularIntensity = 1.0f;
	RT64.defaultMaterial.specularExponent = 5.0f;
	RT64.defaultMaterial.solidAlphaMultiplier = 1.0f;
	RT64.defaultMaterial.shadowAlphaMultiplier = 1.0f;
	RT64.defaultMaterial.diffuseColorMix.x = 0.0f;
    RT64.defaultMaterial.diffuseColorMix.y = 0.0f;
    RT64.defaultMaterial.diffuseColorMix.z = 0.0f;
    RT64.defaultMaterial.diffuseColorMix.w = 0.0f;
	RT64.defaultMaterial.selfLight.x = 0.0f;
    RT64.defaultMaterial.selfLight.y = 0.0f;
    RT64.defaultMaterial.selfLight.z = 0.0f;
	RT64.defaultMaterial.lightGroupMaskBits = RT64_LIGHT_GROUP_MASK_ALL;
	RT64.defaultMaterial.fogColor.x = 1.0f;
    RT64.defaultMaterial.fogColor.y = 1.0f;
    RT64.defaultMaterial.fogColor.z = 1.0f;
	RT64.defaultMaterial.fogMul = 0.0f;
	RT64.defaultMaterial.fogOffset = 0.0f;

    // Configure N64 Color combiner parameters.
	RT64.defaultMaterial.c0[0] = 0;
	RT64.defaultMaterial.c0[1] = 0;
	RT64.defaultMaterial.c0[2] = 0;
	RT64.defaultMaterial.c0[3] = 5;
	RT64.defaultMaterial.c1[0] = 0;
	RT64.defaultMaterial.c1[1] = 0;
	RT64.defaultMaterial.c1[2] = 0;
	RT64.defaultMaterial.c1[3] = 0;
	RT64.defaultMaterial.do_single[0] = 1;
	RT64.defaultMaterial.do_single[1] = 0;
	RT64.defaultMaterial.do_multiply[0] = 0;
	RT64.defaultMaterial.do_multiply[1] = 0;
	RT64.defaultMaterial.do_mix[0] = 0;
	RT64.defaultMaterial.do_mix[1] = 0;
	RT64.defaultMaterial.color_alpha_same = 0;
	RT64.defaultMaterial.opt_alpha = 0;
	RT64.defaultMaterial.opt_fog = 1;
	RT64.defaultMaterial.opt_texture_edge = 0;

	// Initialize the global lights to their default values.
	memset(RT64.levelLights, 0, sizeof(RT64.levelLights));
    memset(RT64.levelLightCounts, 0, sizeof(RT64.levelLightCounts));
    for (int l = 0; l < MAX_LEVELS; l++) {
        for (int a = 0; a < MAX_AREAS; a++) {
            RT64.levelLights[l][a][0].diffuseColor.x = 0.3f;
            RT64.levelLights[l][a][0].diffuseColor.y = 0.35f;
            RT64.levelLights[l][a][0].diffuseColor.z = 0.45f;

            RT64.levelLights[l][a][1].position.x = 100000.0f;
            RT64.levelLights[l][a][1].position.y = 200000.0f;
            RT64.levelLights[l][a][1].position.z = 100000.0f;
            RT64.levelLights[l][a][1].diffuseColor.x = 0.8f;
            RT64.levelLights[l][a][1].diffuseColor.y = 0.75f;
            RT64.levelLights[l][a][1].diffuseColor.z = 0.65f;
            RT64.levelLights[l][a][1].attenuationRadius = 1e11;
			RT64.levelLights[l][a][1].pointRadius = 5000.0f;
            RT64.levelLights[l][a][1].specularIntensity = 1.0f;
            RT64.levelLights[l][a][1].shadowOffset = 0.0f;
            RT64.levelLights[l][a][1].attenuationExponent = 0.0f;
			RT64.levelLights[l][a][1].groupBits = RT64_LIGHT_GROUP_DEFAULT;
            
            RT64.levelLightCounts[l][a] = 2;
        }
    }

	// Load the light sample presets and choose the default preset.
	gfx_rt64_load_light_sample_presets();
	RT64.activeLightSamplePreset = RT64.lightSamplePresets[LIGHT_SAMPLE_PRESET_DEFAULT];
	
	// Load the global lights from a file.
	gfx_rt64_load_level_lights();
	gfx_rt64_set_samples_level_lights();

	// Initialize camera.
	RT64.viewMatrix = RT64.identityTransform;
    RT64.nearDist = 1.0f;
    RT64.farDist = 1000.0f;
    RT64.fovRadians = 0.75f;

	// Load the geo layout mods from a file.
	gfx_rt64_load_geo_layout_mods();

	// Load the texture mods from a file.
	gfx_rt64_load_texture_mods();
}

static void gfx_rt64_wapi_shutdown(void) {
}

static void gfx_rt64_wapi_set_keyboard_callbacks(bool (*on_key_down)(int scancode), bool (*on_key_up)(int scancode), void (*on_all_keys_up)(void)) {
	RT64.on_key_down = on_key_down;
    RT64.on_key_up = on_key_up;
    RT64.on_all_keys_up = on_all_keys_up;
}

static void gfx_rt64_wapi_main_loop(void (*run_one_game_iter)(void)) {
	RT64.run_one_game_iter = run_one_game_iter;

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

static void gfx_rt64_wapi_get_dimensions(uint32_t *width, uint32_t *height) {
	RECT rect;
	GetWindowRect(RT64.hwnd, &rect);
	int rectWidth = rect.right - rect.left;
	int rectHeight = rect.bottom - rect.top;
	*width = rectWidth;
	*height = rectHeight;
}

static void gfx_rt64_wapi_handle_events(void) {
}

static bool gfx_rt64_wapi_start_frame(void) {
    if (RT64.dropNextFrame) {
		RT64.dropNextFrame = false;
		return false;
	}
	else {
		return true;
	}
}

static void gfx_rt64_wapi_swap_buffers_begin(void) {
}

static void gfx_rt64_wapi_swap_buffers_end(void) {
}

double gfx_rt64_wapi_get_time(void) {
    return 0.0;
}

static bool gfx_rt64_rapi_z_is_from_0_to_1(void) {
    return true;
}

static void gfx_rt64_rapi_unload_shader(struct ShaderProgram *old_prg) {
	
}

static void gfx_rt64_rapi_load_shader(struct ShaderProgram *new_prg) {
	RT64.shaderProgram = new_prg;
}

static struct ShaderProgram *gfx_rt64_rapi_create_and_load_new_shader(uint32_t shader_id) {
	ShaderProgram *shaderProgram = new ShaderProgram();
    int c[2][4];
    for (int i = 0; i < 4; i++) {
        c[0][i] = (shader_id >> (i * 3)) & 7;
        c[1][i] = (shader_id >> (12 + i * 3)) & 7;
    }

	shaderProgram->shader_id = shader_id;
    shaderProgram->used_textures[0] = false;
    shaderProgram->used_textures[1] = false;
    shaderProgram->num_inputs = 0;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            if (c[i][j] >= SHADER_INPUT_1 && c[i][j] <= SHADER_INPUT_4) {
                if (c[i][j] > shaderProgram->num_inputs) {
                    shaderProgram->num_inputs = c[i][j];
                }
            }
            if (c[i][j] == SHADER_TEXEL0 || c[i][j] == SHADER_TEXEL0A) {
                shaderProgram->used_textures[0] = true;
            }
            if (c[i][j] == SHADER_TEXEL1) {
                shaderProgram->used_textures[1] = true;
            }
        }
    }

	RT64.shaderPrograms[shader_id] = shaderProgram;

	gfx_rt64_rapi_load_shader(shaderProgram);

	return shaderProgram;
}

static struct ShaderProgram *gfx_rt64_rapi_lookup_shader(uint32_t shader_id) {
	auto it = RT64.shaderPrograms.find(shader_id);
    return (it != RT64.shaderPrograms.end()) ? it->second : nullptr;
}

static void gfx_rt64_rapi_shader_get_info(struct ShaderProgram *prg, uint8_t *num_inputs, bool used_textures[2]) {
    *num_inputs = prg->num_inputs;
    used_textures[0] = prg->used_textures[0];
    used_textures[1] = prg->used_textures[1];
}

static uint32_t gfx_rt64_rapi_new_texture(const char *name) {
	uint32_t textureKey = RT64.textures.size();
	auto &recordedTexture = RT64.textures[textureKey];
	recordedTexture.texture = nullptr;
	recordedTexture.linearFilter = 0;
	recordedTexture.cms = 0;
	recordedTexture.cmt = 0;
	recordedTexture.hash = gfx_rt64_get_texture_name_hash(name);
	RT64.textureHashIdMap[recordedTexture.hash] = textureKey;
    return textureKey;
}

static void gfx_rt64_rapi_select_texture(int tile, uint32_t texture_id) {
	assert(tile < 2);
	RT64.currentTile = tile;
    RT64.currentTextureIds[tile] = texture_id;
}

static void gfx_rt64_rapi_upload_texture(const uint8_t *rgba32_buf, int width, int height) {
	RT64_TEXTURE *texture = RT64.lib.CreateTextureFromRGBA8(RT64.device, rgba32_buf, width, height, 4);
	uint32_t textureKey = RT64.currentTextureIds[RT64.currentTile];
	RT64.textures[textureKey].texture = texture;
}

static void gfx_rt64_rapi_set_sampler_parameters(int tile, bool linear_filter, uint32_t cms, uint32_t cmt) {
	uint32_t textureKey = RT64.currentTextureIds[tile];
	auto &recordedTexture = RT64.textures[textureKey];
	recordedTexture.linearFilter = linear_filter;
	recordedTexture.cms = cms;
	recordedTexture.cmt = cmt;
}

static void gfx_rt64_rapi_set_depth_test(bool depth_test) {
}

static void gfx_rt64_rapi_set_depth_mask(bool depth_mask) {
}

static void gfx_rt64_rapi_set_zmode_decal(bool zmode_decal) {
}

static void gfx_rt64_rapi_set_viewport(int x, int y, int width, int height) {
}

static void gfx_rt64_rapi_set_scissor(int x, int y, int width, int height) {
}

static void gfx_rt64_rapi_set_use_alpha(bool use_alpha) {
}

static RT64_MESH *gfx_rt64_rapi_process_mesh(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris, bool raytraceable) {
	assert(RT64.shaderProgram != nullptr);

    const bool useTexture = RT64.shaderProgram->used_textures[0] || RT64.shaderProgram->used_textures[1];
	const int numInputs = RT64.shaderProgram->num_inputs;
	const bool useAlpha = RT64.shaderProgram->shader_id & SHADER_OPT_ALPHA;
	static const int MaxVertexCount = GFX_MAX_BUFFERED * 3;
	static const int MaxIndexCount = GFX_MAX_BUFFERED;
	static RT64_VERTEX vertices[MaxVertexCount];
	static unsigned int indices[MaxIndexCount];
	unsigned int f = 0, vertexCount = 0;
	assert((buf_vbo_num_tris * 3) <= MaxVertexCount);
	memset(vertices, 0, buf_vbo_num_tris * 3 * sizeof(RT64_VERTEX));
	while (f < buf_vbo_len) {
		auto &v = vertices[vertexCount];
		v.position.x = buf_vbo[f++];
		v.position.y = buf_vbo[f++];
		v.position.z = buf_vbo[f++];
		f++;

		v.normal.x = buf_vbo[f++];
		v.normal.y = buf_vbo[f++];
		v.normal.z = buf_vbo[f++];

		if (useTexture) {
			v.uv.x = buf_vbo[f++];
			v.uv.y = buf_vbo[f++];
		}

		for (int i = 0; i < numInputs; i++) {
			v.inputs[i].x = buf_vbo[f++];
			v.inputs[i].y = buf_vbo[f++];
			v.inputs[i].z = buf_vbo[f++];

			if (useAlpha) {
				v.inputs[i].w = buf_vbo[f++];
			}
			else {
				v.inputs[i].w = 1.0f;
			}
		}

		vertexCount++;
	}

	// Calculate hash and use it as key.
	// NOTE: We limit the max amount of elements that can be hashed to improve performance with model mods that add a lot of mesh data.
	const unsigned int HashMaxVertexCount = 32;
    XXHash64 hashStream(0);
    hashStream.add(vertices, sizeof(RT64_VERTEX) * std::min(vertexCount, HashMaxVertexCount));
    uint64_t key = hashStream.hash();

	// Check for static mesh first.
	auto staticMeshIt = RT64.staticMeshes.find(key);
	if (staticMeshIt != RT64.staticMeshes.end()) {
		staticMeshIt->second.lifetime = CACHED_MESH_LIFETIME;
		return staticMeshIt->second.mesh;
	}

	// Update the dynamic mesh key values.
	auto &dynamicMeshKey = RT64.dynamicMeshKeys[key];
	dynamicMeshKey.counter++;
	dynamicMeshKey.seen = true;

	// Build the index array.
	unsigned int indexCount = 0;
	for (int t = 0; t < buf_vbo_num_tris; t++) {
		indices[t * 3 + 0] = indexCount++;
		indices[t * 3 + 1] = indexCount++;
		indices[t * 3 + 2] = indexCount++;
	}

	// Store the mesh as static if requirements are met.
	if ((dynamicMeshKey.counter > CACHED_MESH_REQUIRED_FRAMES) && (RT64.cachedMeshesPerFrame < CACHED_MESH_MAX_PER_FRAME)) {
		auto &staticMesh = RT64.staticMeshes[key];
		staticMesh.mesh = RT64.lib.CreateMesh(RT64.device, raytraceable ? RT64_MESH_RAYTRACE_ENABLED : 0);
		staticMesh.vertexCount = vertexCount;
		staticMesh.indexCount = indexCount;
		staticMesh.raytraceable = raytraceable;
		RT64.lib.SetMesh(staticMesh.mesh, vertices, vertexCount, indices, indexCount);
		RT64.cachedMeshesPerFrame++;
		return staticMesh.mesh;
	}

	// Search for a dynamic mesh that has the same key.
	auto dynamicMeshIt = RT64.dynamicMeshes.find(key);
	if (dynamicMeshIt != RT64.dynamicMeshes.end()) {
		dynamicMeshIt->second.inUse = true;
		dynamicMeshIt->second.lifetime = DYNAMIC_MESH_LIFETIME;
		return dynamicMeshIt->second.mesh;
	}

	// Search linearly for a dynamic mesh that has the same amount of indices and vertices.
	uint64_t foundKey = 0;
	for (auto dynamicMeshIt : RT64.dynamicMeshes) {
		if (
			!dynamicMeshIt.second.inUse &&
			(dynamicMeshIt.second.vertexCount == vertexCount) && 
			(dynamicMeshIt.second.indexCount == indexCount) && 
			(dynamicMeshIt.second.raytraceable == raytraceable)
		) 
		{
			foundKey = dynamicMeshIt.first;
			break;
		}
	}

	// If we found a valid key, change the key where the mesh is stored.
	if (foundKey != 0) {
		RT64.dynamicMeshes[key] = RT64.dynamicMeshes[foundKey];
		RT64.dynamicMeshes.erase(foundKey);
	}

	auto &dynamicMesh = RT64.dynamicMeshes[key];

	// If no key was found before, we need to create the mesh.
	if (foundKey == 0) {
		dynamicMesh.mesh = RT64.lib.CreateMesh(RT64.device, raytraceable ? (RT64_MESH_RAYTRACE_ENABLED | RT64_MESH_RAYTRACE_UPDATABLE) : 0);
		dynamicMesh.vertexCount = vertexCount;
		dynamicMesh.indexCount = indexCount;
		dynamicMesh.raytraceable = raytraceable;
	}

	// Update the dynamic mesh.
	dynamicMesh.inUse = true;
	dynamicMesh.lifetime = DYNAMIC_MESH_LIFETIME;
	RT64.lib.SetMesh(dynamicMesh.mesh, vertices, vertexCount, indices, indexCount);
	return dynamicMesh.mesh;
}

RT64_INSTANCE *gfx_rt64_rapi_add_instance() {
	assert(RT64.instanceCount < MAX_INSTANCES);
	int instanceIndex = RT64.instanceCount++;
	if (instanceIndex >= RT64.instanceAllocCount) {
		RT64.instances[instanceIndex] = RT64.lib.CreateInstance(RT64.scene);
		RT64.instanceAllocCount++;
	}

	return RT64.instances[instanceIndex];
}

RT64_MATERIAL gfx_rt64_rapi_build_material(ShaderProgram *prg, bool linearFilter, uint32_t cms, uint32_t cmt) {
	RT64_MATERIAL mat = RT64.defaultMaterial;

	// Sampler.
	mat.filterMode = linearFilter;
	mat.hAddressMode = (cms & G_TX_CLAMP) ? 2 : (cms & G_TX_MIRROR) ? 1 : 0;
	mat.vAddressMode = (cmt & G_TX_CLAMP) ? 2 : (cmt & G_TX_MIRROR) ? 1 : 0;

	// Fog.
	mat.fogColor = RT64.fogColor;
	mat.fogMul = RT64.fogMul;
	mat.fogOffset = RT64.fogOffset;

	// N64 Color Combiner.
	uint32_t shader_id = prg->shader_id;
	mat.c0[0] = (shader_id >> (0 * 3)) & 7;
	mat.c0[1] = (shader_id >> (1 * 3)) & 7;
	mat.c0[2] = (shader_id >> (2 * 3)) & 7;
	mat.c0[3] = (shader_id >> (3 * 3)) & 7;
	mat.c1[0] = (shader_id >> (12 + 0 * 3)) & 7;
	mat.c1[1] = (shader_id >> (12 + 1 * 3)) & 7;
	mat.c1[2] = (shader_id >> (12 + 2 * 3)) & 7;
	mat.c1[3] = (shader_id >> (12 + 3 * 3)) & 7;
	mat.do_single[0] = mat.c0[2] == 0;
	mat.do_single[1] = mat.c1[2] == 0;
	mat.do_multiply[0] = mat.c0[1] == 0 && mat.c0[3] == 0;
	mat.do_multiply[1] = mat.c1[1] == 0 && mat.c1[3] == 0;
	mat.do_mix[0] = mat.c0[1] == mat.c0[3];
	mat.do_mix[1] = mat.c1[1] == mat.c1[3];
	mat.color_alpha_same = (shader_id & 0xfff) == ((shader_id >> 12) & 0xfff);
	mat.opt_alpha = (shader_id & SHADER_OPT_ALPHA) != 0;
	mat.opt_fog = (shader_id & SHADER_OPT_FOG) != 0;
	mat.opt_texture_edge = (shader_id & SHADER_OPT_TEXTURE_EDGE) != 0;
	//mat.opt_noise = (shader_id & SHADER_OPT_NOISE) != 0;

	return mat;
}

static void gfx_rt64_add_light(RT64_LIGHT *lightMod, RT64_MATRIX4 transform) {
    assert(RT64.lightCount < MAX_LIGHTS);
	auto &light = RT64.lights[RT64.lightCount++];
    light = *lightMod;

	gfx_rt64_set_light_samples(&light);

    light.position = transform_position_affine(transform, lightMod->position);

	// Use a vector that points in all three axes in case the node uses non-uniform scaling to get an estimate.
	RT64_VECTOR3 scaleVector = transform_direction_affine(transform, { 1.0f, 1.0f, 1.0f });
	float scale = vector_length(scaleVector) / sqrt(3);
	light.attenuationRadius *= scale;
	light.pointRadius *= scale;
	light.shadowOffset *= scale;
}

static void gfx_rt64_rapi_apply_mod(RT64_MATERIAL *material, RT64_TEXTURE **normal, RecordedMod *mod, RT64_MATRIX4 transform) {
	if (mod->materialMod != NULL) {
		RT64_ApplyMaterialAttributes(material, mod->materialMod);
	}

	if (mod->lightMod != NULL) {
		gfx_rt64_add_light(mod->lightMod, transform);
	}

	if (mod->normalMapHash != 0) {
		auto hashIt = RT64.textureHashIdMap.find(mod->normalMapHash);
		if (hashIt != RT64.textureHashIdMap.end()) {
			auto texIt = RT64.textures.find(hashIt->second);
			if (texIt != RT64.textures.end()) {
				*normal = texIt->second.texture;
			}
		}
	}
}

static void gfx_rt64_rapi_draw_triangles_common(RT64_MATRIX4 transform, float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris, bool double_sided, bool raytrace) {
	RT64_TEXTURE *diffuseMapTexture = RT64.blankTexture;
	RT64_TEXTURE *normalMapTexture = nullptr;
	RecordedMod *textureMod = nullptr;
	bool linearFilter = false;
	uint32_t cms = 0, cmt = 0;
	
	// Create the instance.
	RT64_INSTANCE *instance = gfx_rt64_rapi_add_instance();

	// Find all parameters associated to the texture if it's used.
	bool highlightMaterial = false;
	if (RT64.shaderProgram->used_textures[0]) {
		RecordedTexture &recordedTexture = RT64.textures[RT64.currentTextureIds[RT64.currentTile]];
		linearFilter = recordedTexture.linearFilter; 
		cms = recordedTexture.cms; 
		cmt = recordedTexture.cmt;

		if (recordedTexture.texture != nullptr) {
			diffuseMapTexture = recordedTexture.texture;
		}

		auto texModIt = RT64.texMods.find(recordedTexture.hash);
		if (texModIt != RT64.texMods.end()) {
			textureMod = texModIt->second;
		}
		
		// Update data for ray picking.
		if (RT64.pickTextureHighlight && (recordedTexture.hash == RT64.pickedTextureHash)) {
			highlightMaterial = true;
		}

		RT64.lastInstanceTextureHashes[instance] = recordedTexture.hash;
	}

	// Build material with applied mods.
	RT64_MATERIAL material = gfx_rt64_rapi_build_material(RT64.shaderProgram, linearFilter, cms, cmt);
	if (RT64.graphNodeMod != nullptr) {
		gfx_rt64_rapi_apply_mod(&material, &normalMapTexture, RT64.graphNodeMod, transform);
	}

	if (textureMod != nullptr) {
		gfx_rt64_rapi_apply_mod(&material, &normalMapTexture, textureMod, transform);
	}

	if (highlightMaterial) {
		material.diffuseColorMix = { 1.0f, 0.0f, 1.0f, 0.5f };
		material.selfLight = { 1.0f, 1.0f, 1.0f };
		material.lightGroupMaskBits = 0;
	}

	// Process the mesh that corresponds to the VBO.
	RT64_MESH *mesh = gfx_rt64_rapi_process_mesh(buf_vbo, buf_vbo_len, buf_vbo_num_tris, raytrace);

	// Mark the right instance flags.
	unsigned int instanceFlags = 0;
	if (RT64.background) {
		instanceFlags |= RT64_INSTANCE_RASTER_BACKGROUND;
	}

	if (double_sided) {
		instanceFlags |= RT64_INSTANCE_DISABLE_BACKFACE_CULLING;
	}

	// Update the instance.
	RT64.lib.SetInstance(instance, mesh, transform, diffuseMapTexture, normalMapTexture, material, instanceFlags);
}

void gfx_rt64_rapi_set_fog(uint8_t fog_r, uint8_t fog_g, uint8_t fog_b, int16_t fog_mul, int16_t fog_offset) {
	RT64.fogColor.x = fog_r / 255.0f;
	RT64.fogColor.y = fog_g / 255.0f;
	RT64.fogColor.z = fog_b / 255.0f;
	RT64.fogMul = fog_mul;
	RT64.fogOffset = fog_offset;
}

static void gfx_rt64_rapi_draw_triangles_ortho(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris, bool double_sided) {
	gfx_rt64_rapi_draw_triangles_common(RT64.identityTransform, buf_vbo, buf_vbo_len, buf_vbo_num_tris, double_sided, false);
}

static void gfx_rt64_rapi_draw_triangles_persp(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris, float transform_affine[4][4], bool double_sided) {
	// Stop considering the orthographic projection triangles as background as soon as perspective triangles are drawn.
	if (RT64.background) {
		RT64.background = false;
	}

	RT64_MATRIX4 transform;
	memcpy(transform.m, transform_affine, sizeof(float) * 16);
	gfx_rt64_rapi_draw_triangles_common(transform, buf_vbo, buf_vbo_len, buf_vbo_num_tris, double_sided, true);
}

static void gfx_rt64_rapi_init(void) {
}

static void gfx_rt64_rapi_on_resize(void) {

}

static void gfx_rt64_rapi_shutdown(void) {
}

static void gfx_rt64_rapi_start_frame(void) {
	RT64.background = true;
    RT64.instanceCount = 0;

	int levelIndex = gCurrLevelNum;
	int courseIndex = gCurrCourseNum;
	int areaIndex = gCurrAreaIndex;
	char marioMessage[256] = "";
    char levelMessage[256] = "";
    sprintf(marioMessage, "Mario pos: %.1f %.1f %.1f", gMarioState->pos[0], gMarioState->pos[1], gMarioState->pos[2]);
    sprintf(levelMessage, "Level #%d Course #%d Area #%d", levelIndex, courseIndex, areaIndex);
    RT64.lib.PrintToInspector(RT64.inspector, marioMessage);
    RT64.lib.PrintToInspector(RT64.inspector, levelMessage);
    RT64.lib.PrintToInspector(RT64.inspector, "F5: Save all configuration");

	// Level lights editor.
    RT64_LIGHT *lights = RT64.levelLights[levelIndex][areaIndex];
    int *lightCount = &RT64.levelLightCounts[levelIndex][areaIndex];
    RT64.lib.SetLightsInspector(RT64.inspector, lights, lightCount, MAX_LEVEL_LIGHTS);
	gfx_rt64_set_samples_level_lights();
    memcpy(RT64.lights, lights, sizeof(RT64_LIGHT) * (*lightCount));
    RT64.lightCount = *lightCount;

    RT64.graphNodeMod = nullptr;
}

static void gfx_rt64_rapi_end_frame(void) {
	// Check instances.
    while (RT64.instanceAllocCount > RT64.instanceCount) {
        int instanceIndex = RT64.instanceAllocCount - 1;
        RT64.lib.DestroyInstance(RT64.instances[instanceIndex]);
        RT64.instanceAllocCount--;
    }

	// Set the camera.
	RT64.lib.SetViewPerspective(RT64.view, RT64.viewMatrix, RT64.fovRadians, RT64.nearDist, RT64.farDist);

    // Set lights on the scene.
    RT64.lib.SetSceneLights(RT64.scene, RT64.lights, RT64.lightCount);

    char statsMessage[256] = "";
    sprintf(statsMessage, "Instances %d Lights %d", RT64.instanceCount, RT64.lightCount);
    RT64.lib.PrintToInspector(RT64.inspector, statsMessage);

	// Draw frame.
	LARGE_INTEGER StartTime, EndTime, ElapsedMicroseconds;
	QueryPerformanceCounter(&StartTime);
	RT64.lib.DrawDevice(RT64.device, 1);
	QueryPerformanceCounter(&EndTime);
	elapsed_time(StartTime, EndTime, RT64.Frequency, ElapsedMicroseconds);

	// Inspector.
	char message[64];
	sprintf(message, "RT64: %.3f ms\n", ElapsedMicroseconds.QuadPart / 1000.0);
	RT64.lib.PrintToInspector(RT64.inspector, message);

	// Left click allows to pick a texture for editing from the viewport.
	if (RT64.pickTextureNextFrame) {
		POINT cursorPos = {};
		GetCursorPos(&cursorPos);
		ScreenToClient(RT64.hwnd, &cursorPos);
		RT64_INSTANCE *instance = RT64.lib.GetViewRaytracedInstanceAt(RT64.view, cursorPos.x, cursorPos.y);
		if (instance != nullptr) {
			auto instIt = RT64.lastInstanceTextureHashes.find(instance);
			if (instIt != RT64.lastInstanceTextureHashes.end()) {
				RT64.pickedTextureHash = instIt->second;
			}
		}
		else {
			RT64.pickedTextureHash = 0;
		}

		RT64.pickTextureNextFrame = false;
	}

	RT64.lastInstanceTextureHashes.clear();

	// Edit last picked texture.
	if (RT64.pickedTextureHash != 0) {
		const std::string textureName = RT64.texNameMap[RT64.pickedTextureHash];
		RecordedMod *texMod = RT64.texMods[RT64.pickedTextureHash];
		if (texMod == nullptr) {
			texMod = new RecordedMod();
			texMod->materialMod = nullptr;
			texMod->lightMod = nullptr;
			texMod->normalMapHash = 0;
			RT64.texMods[RT64.pickedTextureHash] = texMod;
		}

		if (texMod->materialMod == nullptr) {
			texMod->materialMod = new RT64_MATERIAL();
			texMod->materialMod->enabledAttributes = RT64_ATTRIBUTE_NONE;
		}

		RT64.lib.SetMaterialInspector(RT64.inspector, texMod->materialMod, textureName.c_str());
	}

	// Mesh key cleanup.
	auto keyIt = RT64.dynamicMeshKeys.begin();
	while (keyIt != RT64.dynamicMeshKeys.end()) {
		if (keyIt->second.seen) {
			keyIt->second.seen = false;
		}
		else if (keyIt->second.counter > 0) {
			keyIt->second.counter--;
		}
		else {
			keyIt = RT64.dynamicMeshKeys.erase(keyIt);
			continue;
		}

		keyIt++;
	}

	// Mesh cleanup.
	auto staticMeshIt = RT64.staticMeshes.begin();
	while (staticMeshIt != RT64.staticMeshes.end()) {
		if (staticMeshIt->second.lifetime > 0) {
            staticMeshIt->second.lifetime--;
			staticMeshIt++;
        }
		else {
			RT64.lib.DestroyMesh(staticMeshIt->second.mesh);
			staticMeshIt = RT64.staticMeshes.erase(staticMeshIt);
		}
	}

	// Dynamic mesh cleanup.
	auto dynamicMeshIt = RT64.dynamicMeshes.begin();
	while (dynamicMeshIt != RT64.dynamicMeshes.end()) {
		if (dynamicMeshIt->second.lifetime > 0) {
			dynamicMeshIt->second.inUse = false;
            dynamicMeshIt->second.lifetime--;
			dynamicMeshIt++;
        }
		else {
			RT64.lib.DestroyMesh(dynamicMeshIt->second.mesh);
			dynamicMeshIt = RT64.dynamicMeshes.erase(dynamicMeshIt);
		}
	}

    RT64.cachedMeshesPerFrame = 0;
}

static void gfx_rt64_rapi_finish_render(void) {

}

static void gfx_rt64_rapi_set_camera_perspective(float fov_degrees, float near_dist, float far_dist) {
    RT64.fovRadians = (fov_degrees / 180.0f) * M_PI;
	RT64.nearDist = near_dist;
    RT64.farDist = far_dist;
}

static void gfx_rt64_rapi_set_camera_matrix(float matrix[4][4]) {
	memcpy(&RT64.viewMatrix.m, matrix, sizeof(float) * 16);
}

static void gfx_rt64_rapi_push_geo_layout(void *geoLayout) {
    assert(RT64.geoLayoutStackSize < MAX_GEO_LAYOUT_STACK_SIZE);
	RT64.geoLayoutStack[RT64.geoLayoutStackSize++] = geoLayout;
}

static void gfx_rt64_rapi_register_graph_node_layout(void *graphNode) {
    if (graphNode != nullptr) {
		for (int s = 0; s < RT64.geoLayoutStackSize; s++) {
			void *geo = RT64.geoLayoutStack[s];
			auto it = RT64.geoLayoutMods.find(geo);
			RecordedMod *geoMod = (it != RT64.geoLayoutMods.end()) ? it->second : nullptr;
			if (geoMod != nullptr) {
				RecordedMod *graphMod = RT64.graphNodeMods[graphNode];
				if (graphMod == nullptr) {
					graphMod = new RecordedMod();
					graphMod->materialMod = nullptr;
					graphMod->lightMod = nullptr;
					RT64.graphNodeMods[graphNode] = graphMod;
				}

				if (geoMod->materialMod != nullptr) {
					if (graphMod->materialMod == nullptr) {
						graphMod->materialMod = new RT64_MATERIAL();
						graphMod->materialMod->enabledAttributes = RT64_ATTRIBUTE_NONE;
					}

					RT64_ApplyMaterialAttributes(graphMod->materialMod, geoMod->materialMod);
					graphMod->materialMod->enabledAttributes |= geoMod->materialMod->enabledAttributes;
				}

				if (geoMod->lightMod != nullptr) {
					if (graphMod->lightMod == nullptr) {
						graphMod->lightMod = new RT64_LIGHT();
					}

					memcpy(graphMod->lightMod, geoMod->lightMod, sizeof(RT64_LIGHT));
				}
			}
		}
	}
}

static void gfx_rt64_rapi_pop_geo_layout() {
    assert(RT64.geoLayoutStackSize > 0);
	RT64.geoLayoutStackSize--;
}

static void *gfx_rt64_rapi_get_graph_node_mod(void *graphNode) {
	auto graphNodeIt = RT64.graphNodeMods.find(graphNode);
	return (graphNodeIt != RT64.graphNodeMods.end()) ? graphNodeIt->second : nullptr;
}

static void gfx_rt64_rapi_set_graph_node_mod(void *graph_node_mod) {
	RT64.graphNodeMod = (RecordedMod *)(graph_node_mod);
}

struct GfxWindowManagerAPI gfx_rt64_wapi = {
    gfx_rt64_wapi_init,
    gfx_rt64_wapi_set_keyboard_callbacks,
    gfx_rt64_wapi_main_loop,
    gfx_rt64_wapi_get_dimensions,
    gfx_rt64_wapi_handle_events,
    gfx_rt64_wapi_start_frame,
    gfx_rt64_wapi_swap_buffers_begin,
    gfx_rt64_wapi_swap_buffers_end,
    gfx_rt64_wapi_get_time,
    gfx_rt64_wapi_shutdown,
};

struct GfxRenderingAPI gfx_rt64_rapi = {
    gfx_rt64_rapi_z_is_from_0_to_1,
    gfx_rt64_rapi_unload_shader,
    gfx_rt64_rapi_load_shader,
    gfx_rt64_rapi_create_and_load_new_shader,
    gfx_rt64_rapi_lookup_shader,
    gfx_rt64_rapi_shader_get_info,
    gfx_rt64_rapi_new_texture,
    gfx_rt64_rapi_select_texture,
    gfx_rt64_rapi_upload_texture,
    gfx_rt64_rapi_set_sampler_parameters,
    gfx_rt64_rapi_set_depth_test,
    gfx_rt64_rapi_set_depth_mask,
    gfx_rt64_rapi_set_zmode_decal,
    gfx_rt64_rapi_set_viewport,
    gfx_rt64_rapi_set_scissor,
    gfx_rt64_rapi_set_use_alpha,
	gfx_rt64_rapi_set_fog,
	gfx_rt64_rapi_set_camera_perspective,
	gfx_rt64_rapi_set_camera_matrix,
	gfx_rt64_rapi_draw_triangles_ortho,
    gfx_rt64_rapi_draw_triangles_persp,
	gfx_rt64_rapi_push_geo_layout,
    gfx_rt64_rapi_register_graph_node_layout,
    gfx_rt64_rapi_pop_geo_layout,
    gfx_rt64_rapi_get_graph_node_mod,
	gfx_rt64_rapi_set_graph_node_mod,
    gfx_rt64_rapi_init,
	gfx_rt64_rapi_on_resize,
    gfx_rt64_rapi_start_frame,
	gfx_rt64_rapi_end_frame,
	gfx_rt64_rapi_finish_render,
    gfx_rt64_rapi_shutdown
};

#else

#error "RT64 is only supported on Windows"

#endif // _WIN32

#endif