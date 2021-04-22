#include "src/game/envfx_snow.h"

const GeoLayout n64_geo[] = {
	GEO_NODE_START(),
	GEO_OPEN_NODE(),
		GEO_DISPLAY_LIST(LAYER_OPAQUE, n64_N_Logo_mesh),
		GEO_DISPLAY_LIST(LAYER_OPAQUE, n64_material_revert_render_settings),
	GEO_CLOSE_NODE(),
	GEO_END(),
};
