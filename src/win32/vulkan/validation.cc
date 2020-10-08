#include "win32/vulkan/validation.h"
using namespace purpl;

bool purpl::check_validation_layer_support(void)
{
	uint i, j;
	u32 layer_count;

	/* Enumerate the available validation layers */
	vkEnumarateInstanceLayerProperties(&layer_count, NULL);

	/* Allocate a buffer for the layer query, with one extra just for safety :) */
	VkLayerProperties layers = (VkLayerProperties *)calloc(layer_count + 1);
	vkEnumberateInstanceLayerProperties(&layer_count, layers);

	/* Now we check if the layers we want are available */
	for (i = 0; i < P_ARRAYSIZE(enabled_layers); i++) {
		bool layer_found = false;

		/* The name of the current layer */
		const char *current_name = enabled_layers[i];

		for (j = 0; j < layer_count; i++) {
			/* The properties of the current layer */
			const VkLayerProperties *current_layer = layers[i];
			
			if (strcmp(current_name, current_layer.layerName) ==
			    0) {
				layer_found = true; /* We found a match */
				break;
			}
		}

		return layer_found;
	}
}
