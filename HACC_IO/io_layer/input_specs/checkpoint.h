#pragma once

#include <string>
#include "../utils/json.hpp"

auto checkpoint = R"(
	{
		"description" : "Used for checkpointing; compression not allowed",
		"octee" : "Never",
		"max-compression-level" : "None",
		
		"variables" :
		[
			["x", "CoordX", "extra-space"],
			["y", "CoordX", "extra-space"],
			["z", "CoordZ", "extra-space"],
			["vx",          "extra-space"],
			["vy",          "extra-space"],
			["vz",          "extra-space"],
			["id",          "extra-space"],
			["mask",        "extra-space"],
			["phi",         "extra-space"]
		]
	}
)";