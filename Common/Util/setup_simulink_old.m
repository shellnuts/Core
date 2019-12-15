Ts = 0.1;

Ncell_thermal = SimCase.Ncell_thermal;
Ncell_RC = SimCase.Ncell_RC;
Nother_thermal = SimCase.Nother_thermal;
Ntot = Ncell_thermal+Nother_thermal;
SOC0 = SimCase.SOC0;
T0 = SimCase.Tcell0;
Tcell0 = T0*ones(Ncell_thermal,1);
Vcell0 = zeros(Ncell_RC,1);
Vp10 = zeros(Ncell_RC,1);
Vp20 = zeros(Ncell_RC,1);
Vh0 = zeros(Ncell_RC,1);
Chflag0 = 1;
Qcell0 = zeros(Ncell_thermal,1);
Qtot0 = zeros(Ntot,1);
Qext_cell = SimCase.Qext_cell;
Qext_other = SimCase.Qext_other;
Tamb = SimCase.Tground;
if(~isempty(Nother_thermal) || Nother_thermal ~=0)
Tother0 = T0*ones(Nother_thermal,1);
end
C = SimCase.C;
Rt = SimCase.Rt;
Vscale = SimCase.Vscale;
Iscale = SimCase.Iscale;
ocvdata = CellModel.ocvdata;
RCdata = CellModel.RCdata;
Rint = SimCase.Rint;
Rground = SimCase.Rground;
DCtype = SimCase.DCtype;
ThermalModelOn=SimCase.ThermalModelOn;
Capscale = SimCase.Capscale;
Rscale = SimCase.Rscale;
NominalCapacity = CellModel.NominalCapacity;
Ntotal = Ncell_thermal + Nother_thermal;

save('setup_simulink.mat','Ts','SOC0','Tcell0','Tother0','C','Rt','Vscale','Iscale','ocvdata','RCdata',...
      'Rint','Rground','DCtype','ThermalModelOn','Capscale','Rscale','NominalCapacity','Vcell0','Vp10',...
      'Vp20','Vh0','Chflag0','Qcell0','Qtot0','Qext_cell','Qext_other','Tamb');