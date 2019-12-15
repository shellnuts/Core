% Main Program for simulation
clear;
profile on
%% Setup simulation case
Setup; % Provide simulation inputs in this file;

%% read simulation inputs
dcname = SimCase.DCfilename;
time = SimCase.dc_sim(:,1);
load = SimCase.dc_sim(:,2);
Ncell_RC = SimCase.Ncell_RC;
Ncell_thermal = SimCase.Ncell_thermal;
Nother_thermal = SimCase.Nother_thermal;
DCtype = SimCase.DCtype;
Tground = SimCase.Tground;
Rground = SimCase.Rground;
SOC0 = SimCase.SOC0;
Tcell0 = SimCase.Tcell0;
Tother0 = SimCase.Tother0;
Qext_cell = SimCase.Qext_cell;
Qext_other = SimCase.Qext_other;
Rint = SimCase.Rint;
Vscale = SimCase.Vscale;
Iscale = SimCase.Iscale;
RCdata = CellModel.RCdata_SIM;
ocvdata = CellModel.ocvdata_SIM;
capacity = SimCase.capacity;
Rt = SimCase.Rt;
C = SimCase.C;
Rscale = SimCase.Rscale;
Capscale = SimCase.Capscale;


%% Allocate variables
dt_dc = diff(time);
Ntime = length(time);
Vcell = zeros(Ncell_RC,Ntime);
Icell = zeros(Ntime,1);
Vpack = zeros(Ntime,1);
Ipack = zeros(Ntime,1);
Ppack = zeros(Ntime,1);
Vocv_cell =  zeros(Ncell_RC,Ntime);
Vocv_pack = zeros(Ntime,1);
Vh = zeros(Ncell_RC,Ntime);
Vp1 = zeros(Ncell_RC,Ntime);
Vp2 = zeros(Ncell_RC,Ntime);
SOC = zeros(Ncell_RC,Ntime);
Tcell = zeros(Ncell_thermal,Ntime);
Tother = zeros(Nother_thermal,Ntime);
Qcell = zeros(Ncell_thermal,Ntime);
Qtot = zeros(Ncell_thermal+Nother_thermal,Ntime);


%% set intitial and boundary conditions
SOC(:,1) = SOC0;
Tcell(:,1) = Tcell0;
Tother(:,1) = Tother0;
Tall = [Tcell;Tother];
Chflag =1;
Vocv_cell(:,1) =ocvModel_SIM(SOC(:,1),Tcell(:,1),ocvdata);
Vocv_pack(1) = sum(Vocv_cell(:,1));
Vcell(:,1) = Vocv_cell(:,1);


%% Main time iteration

for itime = 2:Ntime
%%  load previous time step values
    SOC0 = SOC(:,itime-1);
    Tcell0 = Tcell(:,itime-1);
    Vcell0 = Vcell(:,itime-1);
    Vp10 = Vp1(:,itime-1);
    Vp20 = Vp2(:,itime-1);
    Vh0 = Vh(:,itime-1);
    I0 = Icell(itime-1);
    Chflag0 = Chflag;
    dt = dt_dc(itime-1);
    Qtot0 = Qtot(:,itime-1);
    Qcell0 = Qcell(:,itime-1);
    Tall0 = Tall(:,itime-1);
    load1 = load(itime);
%% Main calculation    
    switch DCtype         % 1-current; 2-power; 3-voltage; 4-resistance
        case 1
           I1 = load1;
        case 2
           I1 = Power2Current(I0,SOC0,Tcell0,Vcell0,load1,RCdata,20,Chflag,Rint,Vscale,Iscale);
        otherwise
            break;
    end
    
    [Vcell1,SOC1,Vp11,Vp21,Vh1,Vocv_cell1,dVdT1,Chflag]=TimeUpdateAll(dt,SOC0,Tcell0,Vp10,Vp20,Vh0,I1,I0,ocvdata,RCdata,capacity,Chflag0,Rscale);
    
    if(SimCase.ThermalModelOn)
        Qcell1 = calQcell(I1,Vcell1,Tcell0,Vocv_cell1,dVdT1);
        Qtot1  = [Qcell1+Qext_cell; Qext_other];
        Tall1  = ThermalModel_SIM(dt,Rt,C,Tall0,Tground,Rground,Qtot1,Qtot0);
    else
        Tall1 = Tall0; 
        Qcell1 = Qcell0;
        Qtot1 = Qtot0;
    end
%% Save data
    Vcell(:,itime) = Vcell1;
    Icell(itime) = I1;
    SOC(:,itime) = SOC1;
    Vp1(:,itime) = Vp11;
    Vp2(:,itime) = Vp21;
    Vh(:,itime) = Vh1;
    Vocv_cell(:,itime) = Vocv_cell1;
    Qcell(:,itime) =Qcell1;
    Qtot(:,itime) = Qtot1;
    Tall(:,itime) = Tall1;
    Tcell(:,itime) = Tall1(1:Ncell_thermal);
    Tother(:,itime) = Tall1(Ncell_thermal+1:end);
    Ipack(itime) = Icell(itime)*Iscale;
    Vpack(itime) = sum(Vcell(:,itime))*Vscale+Ipack(itime)*Rint;
    Ppack(itime) = Ipack(itime)*Vpack(itime);
    Vocv_pack(itime) = sum(Vocv_cell(:,itime))*Vscale;
    
end

%% post processing

Qrms = cumRMS(time,Qcell);
Irms = cumRMS(time,Icell');
Qavg = cumAvg(time,Qcell);
Rdc = calRdc(Vocv_cell,Vcell,Icell');
%% plot results
subplot(221)
plot(time,Vcell)
subplot(222)
plot(time,Ipack)
subplot(223)
plot(time,Tall)
subplot(224)
plot(time,Qcell)

%% save simulation results
Simdata.Vcell = Vcell;
Simdata.Icell = Icell;
Simdata.Vpack = Vpack;
Simdata.Ipack = Ipack;
Simdata.dc = SimCase.dc_sim;
Simdata.Tcell = Tcell;
Simdata.Tother = Tother;
Simdata.Vocv_cell = Vocv_cell;
Simdata.Vocv_pack = Vocv_pack;
Simdata.SOC = SOC;
Simdata.Qcell = Qcell;
Simdata.Qavg = Qavg;
Simdata.Qrms = Qrms;
Simdata.Irms = Irms;
Simdata.Rdc = Rdc;
Simdata.Ppack = Ppack;
Simdata.time = time;
save(['./Results/',dcname,'.mat'],'Simdata');
profile off;