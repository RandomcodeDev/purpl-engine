#include "purpl/win32/vulkan/validation.h"
using namespace purpl;

bool purpl::check_validation_layer_support(void)
{
	uint i, j;
	u32 layer_count;

	/* Enumerate the available validation layers */
	vkEnumerateInstanceLayerProperties(&layer_count, NULL);

	/* Allocate a buffer for the layer query */
	VkLayerProperties *layers = (VkLayerProperties *)calloc(
		layer_count, sizeof(VkLayerProperties));
	if (!layers)
		return false;

	vkEnumerateInstanceLayerProperties(&layer_count, layers);
	if (!layers)
		return false;


	/* Now we check if the layers we want are available */
	for (i = 0; i < P_ARRAYSIZE(enabled_layers); i++) {
		bool layer_found = false;

		for (j = 0; j < layer_count; i++) {
			if (strcmp(enabled_layers[i], layers[i].layerName) == 0) {
				layer_found = true; /* We found a match */
				break;
			}
		}

		return layer_found;
	}
}
