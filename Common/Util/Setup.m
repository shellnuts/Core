
%% Simulation configuration
SimCase.Ns = 14;
SimCase.Np = 1;
SimCase.Celltype = '8Ah_Bsample';
SimCase.Rint = 1.88e-3;      % inter-connect resistance
SimCase.Ncell_RC = 14;
SimCase.Ncell_thermal = 14;   % thermal model nodes for cells
SimCase.Nother_thermal = 1;   % thermal model nodes for other components such as enclosure
SimCase.ThermalModelOn = 1;
SimCase.DCfilename = 'dc_SOC_N20';
SimCase.DCtype = 1;           % 1-current; 2-power; 3-voltage; 4-resistance     
SimCase.Chargescale = 1;
SimCase.Dischargescale = 1;

%% Load dc
load(['./DC/',SimCase.DCfilename,'.mat']);
SimCase.dcdata = dc;
SimCase.dcdata(:,2) = -SimCase.dcdata(:,2);
% SimCase.Testdata = Data;
%% Load cell model
load(['./Cell Model/',SimCase.Celltype,'.mat']);

% initial and b.c conditions
%SimCase.V0 = mean(SimCase.Testdata(4:6,1))/1000;
%soc0 = interp1(CellModel.ocvdata.OCVd(:,2),CellModel.ocvdata.OCVd(:,1),SimCase.V0);
SimCase.SOC0 = 0.43*ones(SimCase.Ncell_RC,1);
SimCase.Capscale = 0.96*ones(SimCase.Ncell_RC,1);
SimCase.Rscale = 1.0*ones(SimCase.Ncell_RC,1);
SimCase.capacity = CellModel.NominalCapacity * ones(SimCase.Ncell_RC,1);
SimCase.capacity = (SimCase.capacity).* (SimCase.Capscale);
%SimCase.SOC0 = [0.8;0.8;0.8;0.8;0.8;0.8;0.8;0.8;0.8;0.8;0.8;0.8;0.8;0.8];
%SimCase.Capscale = [1.0;1.0;1.0;1.0;1.0;1.0;1.0;1.0;1.0;1.0;1.0;1.0;1.0;1.0];
%SimCase.Rscale = [1.0;1.0;1.0;1.0;1.0;1.0;1.0;1.0;1.0;1.0;1.0;1.0;1.0;1.0];

%% thermal property of other components
SimCase.Tamb = -19;
SimCase.Tcoolant = [];
SimCase.Tcell0 = -19;
SimCase.Tother0 = [-19];
SimCase.Qext_cell = 0*ones(SimCase.Ncell_RC,1);
SimCase.Qext_other = [0];
SimCase.Tground = [SimCase.Tamb;SimCase.Tcoolant];
SimCase.Rt_btw_cells = 0.25; % cell to cell thermal resistance
SimCase.Rtcell2other = [0.6;0.7;0.8;0.9;1.0;1.0;1.0;1.0;1.0;1.0;0.9;0.8;0.7;0.6]; % per cell
%SimCase.Rtcell2other = [1.0];
SimCase.Rtcell2ground = repmat([0],[SimCase.Ncell_RC,1]);
SimCase.Rtother2ground = [10/14]; %per pack
SimCase.Rground = [SimCase.Rtcell2ground;SimCase.Rtother2ground];
SimCase.mcp_other = 1.7*900;    %aluminum enclosure


%% Construct one full dc and scale dc
[SimCase.dc_all1cycle] = ConstructDC(SimCase.dcdata);
[SimCase.dc_sim,SimCase.Iscale,SimCase.Vscale] = scaleDC(SimCase.DCtype,SimCase.Ns,SimCase.Np,SimCase.Ncell_RC,SimCase.dc_all1cycle,SimCase.Chargescale,SimCase.Dischargescale);
%% Construct thermal paramters
[SimCase.C,SimCase.Qext_other,SimCase.Rt,SimCase.Rground] = ConstructThermal(SimCase.Ncell_thermal,SimCase.Nother_thermal,CellModel.Cellcp,CellModel.Cellmass,SimCase.mcp_other,SimCase.Vscale,SimCase.Iscale,SimCase.Qext_other,SimCase.Rt_btw_cells,SimCase.Rtcell2other,SimCase.Rground);
%% Generate lookup tables
[CellModel.RCdata_SIM,CellModel.ocvdata_SIM] = constructRCOCVlookup(CellModel.RCdata,CellModel.ocvdata);