import sys
import os

from wkf_utilities import *



def createSlurmScript(scriptFile, env_data, script):
    f = open(scriptFile, "w")
    f.write("#!/bin/bash\n")
    f.write("\n")
    f.write("#SBATCH --nodes " + str(env_data["num-ranks"]) + "\n" )
    f.write("#SBATCH --ntasks-per-node " + str(env_data["num-tasks-per-rank"]) + "\n" )
    f.write("#SBATCH --partition " + env_data["partition"] + "\n" )
    f.write("#SBATCH --mail-user=" + env_data["email"] + "\n")
    f.write("#SBATCH --mail-type=ALL" + "\n")
    f.write("\n")
    f.write(env_data["module"] + "\n")
    f.write(env_data["path"] + "\n")
    f.write("\n")
    f.write("srun python " + script)
    f.close()
    

def main(argv):
    json_data = readJsonFile(argv)


    script = json_data["extractHaloIDScript"]
    run_script_orig = script.replace("$particle-tag-file$", json_data["orig-files"]["particle-tag"])

    for halo in json_data["orig-files"]["halo-ids"]:
        _run_script_orig = run_script_orig.replace("$halo-id$", str(halo))
        _run_script_orig = _run_script_orig.replace("$halo-ids-name$", str(halo) + "_" + json_data["orig-files"]["prefix"] + ".csv")

        createSlurmScript("extractOrigHaloID_" + str(halo) + ".sh", json_data["env"], _run_script_orig)
        #print(_run_script_orig)


    # run on comparison files
    run_script_comp = script.replace("$particle-tag-file$", json_data["comparison-files"]["particle-tag"])

    for halo in json_data["comparison-files"]["halo-ids"]:
        _run_script_comp = run_script_comp.replace("$halo-id$", str(halo))
        _run_script_comp = _run_script_comp.replace("$halo-ids-name$", str(halo) + "_" + json_data["comparison-files"]["prefix"] + ".csv")

        createSlurmScript("extractCompHaloID_" + str(halo) + ".sh", json_data["env"], _run_script_comp)
        #print(_run_script_comp)




    # wkf_extractHaloParticles.py
    # run on original file
    script = json_data["extractParticles"]
    run_script_orig = script.replace("$particle-file$", json_data["orig-files"]["raw"])

    for halo in json_data["orig-files"]["halo-ids"]:
        _run_script_orig = run_script_orig.replace("$halo-id$", str(halo))
        _run_script_orig = _run_script_orig.replace("$particle-csv-list$", str(halo) + "_" + json_data["orig-files"]["prefix"])
        _run_script_orig = _run_script_orig.replace("$halo-ids-name$", str(halo) + "_" + json_data["orig-files"]["prefix"])
        _run_script_orig = _run_script_orig.replace("$output-name$", "halo_" + str(halo) + "_" + json_data["orig-files"]["prefix"] + ".csv")

        createSlurmScript("extractOrigHaloParticles_" + str(halo)+".sh", json_data["env"], _run_script_orig)

        #print(_run_script_orig)


    # run on comparison files
    run_script_comp = script.replace("$particle-file$", json_data["comparison-files"]["raw"])

    for halo in json_data["comparison-files"]["halo-ids"]:
        _run_script_comp = run_script_comp.replace("$halo-id$", str(halo))
        _run_script_orig = run_script_comp.replace("$particle-csv-list$", str(halo) + "_" + json_data["comparison-files"]["prefix"])
        _run_script_comp = _run_script_comp.replace("$halo-ids-name$", str(halo) + "_" + json_data["comparison-files"]["prefix"])
        _run_script_comp = _run_script_comp.replace("$output-name$", "halo_"+ str(halo) + "_" + json_data["comparison-files"]["prefix"] + ".csv")

        createSlurmScript("extractCompHaloParticles_" + str(halo) +".sh", json_data["env"], _run_script_comp)

        #print(_run_script_comp)




    #createSlurmScript(json_data["env"], script)



if __name__ == "__main__":
	main(sys.argv[1])


#python wkf_launchScripts.py inputs/halo_extract_input.json