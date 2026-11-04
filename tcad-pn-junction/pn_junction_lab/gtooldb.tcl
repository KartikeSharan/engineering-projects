variable icon_dir Documents/TCADProjects/pn_junction_lab/icons

# tool: sde_pn
set WB_tool(sde_pn,category) process
set WB_tool(sde_pn,visual_category) process
set WB_tool(sde_pn,acronym) dvs
set WB_tool(sde_pn,after) [list START cshell sprocess]
set Icon(sde_pn) $icon_dir/test.gif
set WB_tool(sde_pn,exec_mode) batch ; # (interactive | batch)
set WB_tool(sde_pn,setup) { os_ln_rel @commands@ n@node@_dvs.cmd "@pwd@" }
set WB_tool(sde_pn,epilogue) { extract_vars "$wdir" @stdout@ @node@ }
set WB_binaries(tool,sde_pn) sde
set WB_tool(sde_pn,cmd_line) "n@node@_dvs.cmd"
set WB_tool(sde_pn,input) [list commands acis acis_journal scheme grid \
    boundary doping layout prosit_layout pref tdrboundary epicsv]
set WB_tool(sde_pn,input,commands,label)  "Commands..."
set WB_tool(sde_pn,input,commands,editor)  text
set WB_tool(sde_pn,input,commands,parametrized)  1
set WB_tool(sde_pn,input,boundary,user)  1
set WB_tool(sde_pn,input,boundary,label)  "Boundary File..."
set WB_tool(sde_pn,input,boundary,editor)  sde		
set WB_tool(sde_pn,input,boundary,parametrized)  1
set WB_tool(sde_pn,input,pref,label)  "Preferences..."
set WB_tool(sde_pn,input,pref,editor)  pref
set WB_tool(sde_pn,input,pref,file)  @tool_label@_dvs.prf
set WB_tool(sde_pn,input,epicsv,label)   "CSV Data File..."
set WB_tool(sde_pn,input,epicsv,parametrized) 1
set WB_tool(sde_pn,input,epicsv,editor)  text
set WB_tool(sde_pn,input,epicsv,file)    @tool_label@_epi.csv
set WB_tool(sde_pn,output) [list commands acis acis_journal scheme boundary grid \
    doping layout prosit_layout tdr tdrboundary episcm epitcl]
set WB_tool(sde_pn,output,acis,file)  n@node@.sat
set WB_tool(sde_pn,output,scheme,file)  n@node@.scm
set WB_tool(sde_pn,output,acis_journal,file)  n@node@.jrl
set WB_tool(sde_pn,output,boundary,file)  n@node@_msh.bnd
set WB_tool(sde_pn,output,commands,file)  n@node@_msh.cmd
set WB_tool(sde_pn,output,layout,file)  n@node@_dvs.lyt
set WB_tool(sde_pn,output,prosit_layout,file)  n@node@_dvs.lay
set WB_tool(sde_pn,output,grid,file)  n@node@_msh.grd
set WB_tool(sde_pn,output,doping,file)  n@node@_msh.dat
set WB_tool(sde_pn,output,tdr,file)  n@node@_msh.tdr
set WB_tool(sde_pn,output,tdrboundary,file) n@node@_bnd.tdr
set WB_tool(sde_pn,output,episcm,file)     n@node@_epi.scm
set WB_tool(sde_pn,output,epitcl,file)     n@node@_epi.tcl
set WB_tool(sde_pn,output,files) "n@node@_* pp@node@_* n@node@.*"
set WB_tool(sde_pn,interactive,option)  "-l"
set WB_tool(sde_pn,batch,option)  "-e -l"
set WB_tool(sde_pn,split,save_cmd)  "(sde_pn:save-model \"n@node@_dvs\")"
set WB_tool(sde_pn,split,load_cmd)  "(sde_pn:open-model \"n@previous@_dvs\")" 
lappend WB_tool(all) sde_pn

# tool: sdevice_pn
set WB_tool(sdevice_pn,category) device
set WB_tool(sdevice_pn,visual_category) device
set WB_tool(sdevice_pn,acronym) des
set WB_tool(sdevice_pn,after) [list START cshell]
set WB_tool(sdevice_pn,cmd_line) @commands@
set WB_binaries(tool,sdevice_pn) sdevice
set Icon(sdevice_pn) $icon_dir/sdevice_pn2.gif
set WB_tool(sdevice_pn,input) [list commands parameter grid doping edit tdr epitcl pref]
set WB_tool(sdevice_pn,input,commands,file)  @tool_label@_des.cmd
set WB_tool(sdevice_pn,input,commands,newfile)  @tool_label@_des.cmd
set WB_tool(sdevice_pn,input,commands,label)  Commands...
set WB_tool(sdevice_pn,input,commands,editor)  text
set WB_tool(sdevice_pn,input,parameter,files)  sdevice_pn_*.par
set WB_tool(sdevice_pn,input,parameter,file)  .par
set WB_tool(sdevice_pn,input,parameter,newfile)  sdevice_pn.par
set WB_tool(sdevice_pn,input,parameter,common) 1
set WB_tool(sdevice_pn,input,parameter,label)  "Parameter..."
set WB_tool(sdevice_pn,input,parameter,parametrized)  1
set WB_tool(sdevice_pn,input,parameter,editor)  text
set WB_tool(sdevice_pn,input,edit,label)  "Include Materials..."
set WB_tool(sdevice_pn,input,edit,cmd)  G_ToolInput::GetDatex
set WB_tool(sdevice_pn,input,pref,label)  "Preferences..."
set WB_tool(sdevice_pn,input,pref,editor)  pref
set WB_tool(sdevice_pn,input,pref,file)   @tool_label@_des.prf          
set WB_tool(sdevice_pn,output) [list log optlog plot acplot spectralplot optfarfield modegain dat optpattern save tdrdat]
set WB_tool(sdevice_pn,output,acplot,file)  n@node@_ac_des.plt
set WB_tool(sdevice_pn,output,plot,file)  n@node@_des.plt
set WB_tool(sdevice_pn,output,optfarfield,file)  n@node@_ff_des.plt
set WB_tool(sdevice_pn,output,modegain,file)  n@node@_gain_des.plt
set WB_tool(sdevice_pn,output,optpattern,file)  n@node@_op_des.dat
set WB_tool(sdevice_pn,output,optlog,file)  n@node@_optics_des.log
set WB_tool(sdevice_pn,output,spectralplot,file)  n@node@_spec_des.tdr
set WB_tool(sdevice_pn,output,files) "n@node@_* pp@node@_* *_n@node@_*"	
set WB_tool(sdevice_pn,epilogue) { extract_vars "$wdir" @stdout@ @node@ }
## parallelization settings
set WB_tool(sdevice_pn,parallel,activate)      0       ; # activate tool parallelization settings 
set WB_tool(sdevice_pn,parallel,mt)            1       ; # supports shared memory parallelization 
set WB_tool(sdevice_pn,parallel,mpi)           0       ; # supports MPI parallelization
set WB_tool(sdevice_pn,parallel,gpu)           0       ; # no support for GPU parallelization
set WB_tool(sdevice_pn,parallel,autodetect)    1       ; # supports auto-detection of threads 
set WB_tool(sdevice_pn,parallel,processes)     1       ; # default (MPI): number of processes to request 
set WB_tool(sdevice_pn,parallel,distribution)  lowest  ; # default (MPI): launch less processes on the same host  
                                          # highest ; # default (MPI): launch more processes on the same host  
set WB_tool(sdevice_pn,parallel,mpihosts)      ""      ; # default: user defined list of hosts for MPI processes
set WB_tool(sdevice_pn,parallel,threads)       1       ; # default: number of threads to request 
set WB_tool(sdevice_pn,parallel,mode)          user    ; # default: no auto-detection, takes (tool,parallel,threads)  
                                          # tool    ; # default: SWB detects number of threads to request 
set WB_tool(sdevice_pn,parallel,gpus)          0       ; # default (GPU): how many GPUs to request
set WB_tool(sdevice_pn,parallel,memory)        0       ; # default: allocates memory according to queue policy,
set WB_tool(sdevice_pn,LSF,resource)           ""      ; # default: additional LSF resources to request
set WB_tool(sdevice_pn,SGE,resource)           ""      ; # default: additional SGE resources to request
set WB_tool(sdevice_pn,TM,resource)            ""      ; # default: additional Torque resources to request
set WB_tool(sdevice_pn,RTDA,resource)          ""      ; # default: additional RTDA resources to request
##
lappend WB_tool(all) sdevice_pn

set WB_binaries(editor,sde_pn) 	    sde_
lappend WB_editor(all) sde_pn

set WB_viewer(sde_pn,files) 	"*n@node@_*.sat"
set WB_viewer(sde_pn,label) 	"S&DE (Select .sat File...)"
set WB_viewer(sde_pn,nbfiles) 	1
set WB_viewer(sde_pn,cmd_line) @files@
set WB_viewer(sde_pn,exec_dir) @pwd@
set WB_binaries(viewer,sde_pn) sde_pn
lappend WB_viewer(all) sde_pn

# takes effect only if the text editor is jedit 
set WB_viewer(sde_pn_editor,files) "*@node@_dvs.cmd"
set WB_viewer(sde_pn_editor,label) "SDE_pn and Edit&or"
set WB_viewer(sde_pn_editor,nbfiles) 1
set WB_viewer(sde_pn_editor,cmd_line) "-editor @files@"
set WB_viewer(sde_pn_editor,exec_dir) @pwd@
set WB_binaries(viewer,sde_pn_editor) sde
lappend WB_viewer(all) sde_pn_editor
