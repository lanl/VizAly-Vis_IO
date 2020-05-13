#pragma once

#include "../utils/json.hpp"

auto standard_output = R"(
{
	"description" : "used for standard output in Generic IO format",
	"octee" : "3",
	"variables" :
	[
		["x", "CoordX", "extra-space", "default-compressor:SZ~mode:abs 0.003",  "max-compression-level:Lossy"],
		["y", "CoordY", "extra-space", "default-compressor:SZ~mode:abs 0.003",  "max-compression-level:Lossy"],
		["z", "CoordZ", "extra-space", "default-compressor:SZ~mode:abs 0.003",  "max-compression-level:Lossy"],
		["vx",          "extra-space", "default-compressor:SZ~mode:pw_rel 0.1", "max-compression-level:Lossy"],
		["vy",          "extra-space", "default-compressor:SZ~mode:pw_rel 0.1", "max-compression-level:Lossy"],
		["vz",          "extra-space", "default-compressor:SZ~mode:pw_rel 0.1", "max-compression-level:Lossy"],
		["id",          "extra-space", "default-compression-level:BLOSC",       "max-compression-level:Lossless"],
		["phi",         "extra-space", "default-compressor:SZ~mode:abs 0.01",   "max-compression-level:Lossy"],
		["mask",        "extra-space", "default-compressor:BLOSC",              "max-compression-level:Lossless"]
	]
}
)";