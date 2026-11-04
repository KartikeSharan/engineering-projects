# project name
name pn_junction_lab
# execution graph
job 4   -post { extract_vars "$nodedir" n4_dvs.out 4 }  -o n4_dvs "sde -e -l n4_dvs.cmd"
job 8 -d "4"  -post { extract_vars "$nodedir" n8_des.out 8 }  -o n8_des "sdevice pp8_des.cmd"
job 9   -post { extract_vars "$nodedir" n9_dvs.out 9 }  -o n9_dvs "sde -e -l n9_dvs.cmd"
job 10 -d "9"  -post { extract_vars "$nodedir" n10_des.out 10 }  -o n10_des "sdevice pp10_des.cmd"
check sde_pn_dvs.cmd 1580734562
check sdevice_pn_des.cmd 1633028556
check sdevice.par 1580125545
check sde_pn1_dvs.cmd 1580734575
check sdevice_pn1_des.cmd 1584369081
check sdevice.par 1580125545
check global_tooldb 1651469274
check gtree.dat 1791795204
check gtooldb.tcl 1598270294
check ./Silicon.par 1557737032
# included files
file sdevice.par included ./Silicon.par
