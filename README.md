----------------------------------------------------------------- Installation of IMS CWB -----------------------------------------------------------------
Welcome to InsightsNet_CWB installation. Right now we have the current latest version of IMS CWB 3.5.1.

In the "InsightsNet_cwb_ccc" directory you will see 3 directories,
    cwb ---> To install IMS CWB.
    cwb-perl ---> To index corpora via "InsightsNet_CWB" or via command line we need this package.
    cwb-ccc ---> Python package for CWB. To connect cwb with python.

You do not have to download any aditional dependecy files to install cwb, cwb-perl and cwb-ccc (python package for cwb).

**NOTE: Please make it sure that your system is updated before you install CWB and CWB-CCC**

We will suggest you please chack your system by these commands,
    `sudo apt -y update`
    `sudo apt -y upgrade`
    or
    `sudo apt update`
    `sudo apt upgrade`
    or
    `sudo apt-get update`
    `sudo apt-get upgrade`

# What is InsightsNet_CWB_CCC

The main objective of "InsightsNet_CWB_CCC" is to make it easy for users to install CWB and CWB-CCC.

InsightsNet_CWB_CCC is devided into two sections,

    + One section is to install CWB and CWB-perl. 
    + Other section is to install CWB-CCC.

Right now "InsightsNet_CWB_CCC" is combination of CWB(3.5.1) and CWB-CCC(0.11.8).

If you install CWB by using "InsightsNet_CWB_CCC", CWB-PERL will autometically install.

If you want to use CWB only, then you do not need CWB-CCC. But for CWB-CCC, you have to install CWB at first then you can install CWB-CCC.

# How To use InsightsNet_CWB_CCC

Using InsightsNet_CWB_CCC is mainly using "InsightNet_cwb_install.sh" and "InsightNet_cwb_ccc_install.sh".

## InsightNet_cwb_install.sh

The objective of "InsightNet_cwb_install.sh" is to install CWB along with CWB-PERL.

**It will be easy for you to run script files from this directory. And keep the files as it is.**


**.................................NOTE: DO NOT MOVE ANY FILES.................................**


You can run this script from your terminal,
 `./InsightNet_cwb_install.sh`

After running this shell file, you have to provide only one parameter and rest of the task will be done autometically.

After running the script it will ask you to provide " Enter the path of InsightsNet_CWB_CCC: " , here you have to give the path in where your "cwb" is located.

**NOTE: If this line is complicated for you right now, you can check the "img_cwb_ccc" for visual explanation.**

If everything works fine, your CWB will install sussessfully and you will you see an installation susscessful message.

To run CWB the command is `cqp -eC` and press enter.
To exit CWB the command is `exit` or `exit()`.
To check the version of CWB, the command is `cqp --v`.

## InsightNet_cwb_ccc_install.sh

The objective of "InsightNet_cwb_ccc_install.sh" is to install cwb-ccc, which is a python package for cwb.

**It will be easy for you to run script files from this directory. And keep the files as it is.**


**.................................NOTE: DO NOT MOVE ANY FILES.................................**

You can run this script from your terminal,
 `./InsightNet_cwb_ccc_install.sh`

After running this shell file, you have to provide only one parameter and rest of the task will be done autometically.

After running the script it will ask you to provide " Enter the path of InsightsNet_CWB_CCC: " , here you have to give the path in where your "cwb-ccc" is located.

**NOTE: If this line is complicated for you right now, you can check the "img_cwb_ccc" for visual explanation.**

If everything works fine, your cwb-ccc will install sussessfully and you will you see an installation susscessful message.

To check cwb-ccc installation you can do `pip list` and will show a list of python packages that you have installed, and there you will see `cwb-ccc 0.11.8` or you can check by `import ccc`

**NOTE: If you are facing any error regarding cwb-ccc installation via "InsightNet_cwb_ccc_install.sh", please check the InsightsNet_cwb_ccc_troubleshooting.txt file in the cwb-ccc.**


Now you can use IMS CWB and CWB-CCC from your system.


----------------------------------------------------------------- Thank You -----------------------------------------------------------------
