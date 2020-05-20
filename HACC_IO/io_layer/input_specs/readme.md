# File Specification

* file-level-specification : specifications that impact the whole file

  * description: describes what is the purpose of having this file

  * octee-level:
    * Never (octree should never be on)
    * Off (octree is off by default)
    * n (octree is on by default with n levels per rank)
    * Not specified = Off

  * max-compression-level:
    * None (no compression should be applied)
    * Lossless (No or lossless supporte)
    * Lossy (No, lossless or lossy)
    * Not specified = no file level restriction

* variable-description: lists the variables that this file will contain, along with their default option.

  * variable-name: The first parameter has to be the name of variable field; CANNOT be empty!!!

  * variable-coordinate:
    * CoordX
    * CoordY
    * CoordZ
    * not specified = non coordinate type

  * variable-extra-space
    * extra-space
    * not specified = no extra-space

  * variable-default-compressor:
    * default-compressor:compressor-name~param-name:param-value arg1 arg2~param-name:param-value arg1
    * Not specified = no compression unless specified by user

  * variable-max-compression:
    * max-compression:None (no compression)
    * max-compression:lossless
    * max-compression:lossy
    * Not specified = no restriction equivalent to lossy

## File example

```json
{
  "description" : "Just an example",
  "octee" : 4,
  "max-compression-level" : "Lossy",
  
  "variables" :
  [
    ["Variable-Name1", "CoordX", "extra-space", "default-compressor:SZ~mode:pw_rel 0.01"],
    ["Variable-Name2", "CoordY", "extra-space", "default-compressor:BLOSC~compressor:lz4~shuffle:on", "max-compression:Lossless"],
    ["Variable-Name3", "CoordZ", "extra-space", "max-compression:None"],

    ["x", "CoordX", "extra-space", "default-compressor:SZ~mode:abs 0.003", "max-compression:Lossy"],
    ["y", "CoordX", "extra-space", "default-compressor:BLOSC", "max-compression:Lossy"],
    ["z", "CoordZ", "extra-space", "default-compressor:BLOSC~compressor:SNAPPY~shuffle:BITSHUFFLE", "max-compression:Lossy"],
    ["vx", "extra-space", "default-compressor:SZ~mode:pw_rel 0.01"],
    ["vy", "extra-space", "default-compressor:None"],
    ["vz", "extra-space", "default-compressor:SZ~mode:pw_rel 0.01"],
    ["id", "extra-space", "default-compressor:BLOSC", "max-compression:Lossless"],
    ["mask", "extra-space"],
    ["phi", "extra-space", "max-compression:Lossly"]
  ]
}
```
