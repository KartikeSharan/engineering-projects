*** PN JUNCTION - sdevice_des.cmd **

File
{
 *** INPUT FILES
 * geometry, contacts, doping and mesh
 Grid ="n4_msh.tdr"
 * physical parameters
 Parameter = "pp8_des.par"
 *** OUTPUT FILES
 * to visualize distributed variables
 Plot = "n8_des.tdr"
 * to visualize electrical characteristics at the electrodes
 Current= "n8_des.plt"
}

Electrode
{
 * defines which contacts have to be treated as electrodes
 * & initial boundary conditions
 * obviously, electrode names must match the contact names of the
 * sde_dvs.cmd file
 { name="p_contact" voltage=0.0 }
 { name="n_contact" voltage=0.0 }
}


Physics
{
   Mobility (
            DopingDependence
             )
   Fermi
   EffectiveIntrinsicDensity( oldSlotboom   )
   Noise ( DiffusionNoise (eTemperature) )
   Noise ( DiffusionNoise (hTemperature) )
   AreaFactor=2
 
}

Plot
{
* On-mesh-defined variables to be saved in the .tdr output file
*- Doping Profiles
     Doping DonorConcentration AcceptorConcentration
*- Charge, field, potential and potential energy
     SpaceCharge
     ElectricField/Vector Potential
     BandGap EffectiveBandGap BandGapNarrowing ElectronAffinity
     ConductionBandEnergy ValenceBandEnergy
*- Carrier Densities:
     EffectiveIntrinsicDensity IntrinsicDensity
     eDensity hDensity
     eQuasiFermiEnergy hQuasiFermiEnergy
*- Currents and current components: 
     eGradQuasiFermi/Vector hGradQuasiFermi/Vector
     eMobility hMobility eVelocity hVelocity
     Current/Vector eCurrent/Vector hCurrent/Vector
     eDriftVelocity/Vector hDriftVelocity/Vector
     eCurrentDensity hCurrentDensity
*- Band2Band Tunneling & II
    eBand2BandGeneration hBand2BandGeneration Band2BandGeneration
    eAvalanche hAvalanche Avalanche
}

Math 
{
  NumberOfThreads = maximum
  * use previous two solutions (if any) to extrapolate next
  Extrapolate
  * use full derivatives in Newton method
  Derivatives
  * control on relative and absolute errors
  -RelErrControl
  * relative error= 10^(-Digits)
  Digits=7
  * absolute error
  Error(electron)=1e8
  Error(hole)=1e8
  * numerical parameter for space-charge regions
  eDrForceRefDens=1e10
  hDrForceRefDens=1e10
  * maximum number of iteration at each step
  Iterations=10
  * choosing the solver of the linear system
  Method=ParDiSo
  * display simulation time in 'human' units
  Wallclock
  * display max.error information
  CNormPrint
  * to avoid convergence problem when simulating defect-assisted tunneling
  NoSRHperPotential
}


Solve
{
 coupled {poisson}
 coupled {poisson electron hole}
 ** TURN-ON
 * decreasing p_contact to goal
 quasistationary (InitialStep = 1e-2 MaxStep = 1e-2 MinStep=1e-6  Goal {name= "p_contact" voltage = -1}  plot { range=(0, 1) intervals=1 }  )
 {coupled {poisson electron hole} }
 quasistationary (InitialStep = 1e-2 MaxStep = 1e-2 MinStep=1e-6  Goal {name= "p_contact" voltage = 0.6}   plot { range=(0, 1) intervals=2 } )
 {coupled {poisson electron hole} }
}

