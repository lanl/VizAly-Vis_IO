# FR-IQA condisdering distortion sensitivity 

This is the test code for NTIRE Perceptual Image Quality Assessment (PIQA) Challenge

By running "test_FR.py", result of inference is saved on "output.txt".

Before running "test_FR.py" you have to consider two things. 

1. There are four parameters in "test_FR.py" (from line 132 to 146)
   1) GPU_NUM: name of GPU you want to use.
      - ex) GPU_NUM = "0" or GPU_NUM="2"
   2) dirname: forder directory where test image exist
   3) weights_file: file name of model weights
   4) result_score_txt: text file name for storing inference results
   
   if you setting 1), 2), 3) and running the "test_FR.py", inference result is saved in 4)

2. Test_Images folder
   You must save the reference and distoted images in "Test_Images" folder
   The example of saving file is listed as below

   1) put reference images in "Test_Images/Reference" folder
   2) put distorted images in "Test_Images folder"
   
   There are examplar images in "Test_Images" foler
