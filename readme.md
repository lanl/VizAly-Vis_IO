# Aim
A bunch of utilities to compare halos before and after compression

## Python utils
- matchHalosPos.py: match halos based on position of the halo with an epsilon. Return a list showing the equivalent position in the original uncompressed data of from the compressed data e.g. pos 0 in compressed = pos 64 in uncompressed.
- findParticleIDs.py: find particle ids matching halo id from the particletag
- extractParticles.py: extract paticles matching a list of ids from the original file

- halo_utils.py: useful functions liked to genericio
- utils.py: useful functions

## Jupyter Notebook files:
- comp_pipeline_clean: compares two haloproperties file


# Workflow
1. matchHalosPos.py    : to find matching halos
2. comp_pipeline_clean : get halos filtered by largest difference
3. findParticleIDs     : find the particle id for a selected halo
4. extractParticles    : extract particles for that selected halo