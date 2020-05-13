#pragma once

#include "../utils/json.hpp"

#include "standard-output.h"
#include "checkpoint.h"

auto fileTypes = R"(
{
	"hacc-file-types" :
	[
		"checkpoint",
		"standard-output",
		"visualization",
		"prototype",
		"fof-properties",
		"sod-properties",
		"halo-particles"
	],

	"hacc-file-types-desc" :
	[
		"checkpoint: used for checkpointing ",
		"standard-output: used for regular standard output",
		"visualization: used for outputing selective ranks for visualization",
		"prototyping: used for test purposes",
		"fof-properties: halo file containing output of fof algorithm",
		"sod-properties: halo file containing output of sod agorithm",
		"halo-particles: halo file containfg the particles in a halo"
	]
}
)";