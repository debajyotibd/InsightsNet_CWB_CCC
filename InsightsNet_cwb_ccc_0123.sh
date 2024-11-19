echo "******************* Welcome to InsghtsNet_CWB_CCC Installation Script *******************"
echo "*****************************************************************************"
echo "******************* Please Read the README *******************"


# Where your InsightsNet_CWB_CCC folder is located

read -p 'Enter the path of InsightsNet_CWB_CCC_0123: ' file_path

cd ${file_path}/cwb-ccc-0123

python3 -m pip install cwb-ccc==0.12.3

echo "-------------- Your Python CWB-CCC Installation is Susscessful. --------------"
echo "*****************************************************************************"
echo "-------------- Thank You For Using InsightsNet_CWB_CCC. --------------"