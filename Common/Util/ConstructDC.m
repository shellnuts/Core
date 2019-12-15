function [dc_all1cycle] = ConstructDC(dcdata)

%% user input here
Ndc = 1;
Timestep_DC = [];
Timestep_Rest = 10;
Resttime_before = 0*60;
Resttime_after = 0*60;
Resttime_afterDC = 0*60;
%%

%dc time must start from 0
dc = dcdata;
if(dc(1,1)~=0)
    ttmp = [0;dc(:,1)];
    dctmp = [0;dc(:,2)];
    dc = [ttmp,dctmp];
end
%remove repeating values
dc_t_temp = dc(:,1);
dc_load_temp = dc(:,2);
[dc_t_temp,ia]= unique(dc_t_temp);
dc_load_temp=dc_load_temp(ia);

%reconsstruct dc using user-defined time step
if(~isempty(Timestep_DC))
    dc_t = (0:Timestep_DC:dc_t_temp(end))';
    dc_load = interp1(dc_t_temp,dc_load_temp,dc_t);
else
    dc_t = dc_t_temp;
    dc_load = dc_load_temp;
end

% add rest after DC
if(Resttime_afterDC~=0)
    ttemp = (0.1:Timestep_Rest:Resttime_afterDC)';
    dctemp = zeros(length(ttemp),1);
    dc_t = [dc_t;dc_t(end)+ttemp];
    dc_load = [dc_load;dctemp];
end

dc_total_t = [dc_t];
dc_total_load = [dc_load];
% construct total dc
for i = 1:Ndc-1
    dc_total_t = [dc_total_t;dc_total_t(end)+dc_t+0.1];
    dc_total_load = [dc_total_load;dc_load];
end

if(Resttime_before~=0)
    t_rest0 = (0:Timestep_Rest:Resttime_before)';
    load_rest0 = zeros(length(t_rest0),1);
    dc_total_t = [t_rest0;t_rest0(end)+dc_total_t+0.1];
    dc_total_load = [load_rest0;dc_total_load];
else
    t_rest0 =[];
end
if(Resttime_after~=0)
    t_rest2 = (0:Timestep_Rest:Resttime_after)';
    load_rest2 = zeros(length(t_rest2),1);
    dc_total_t = [dc_total_t;dc_total_t(end)+t_rest2+0.1];
    dc_total_load = [dc_total_load;load_rest2];
else
    t_rest2 =[];
end
dc_total = [dc_total_t,dc_total_load];
dc_all1cycle = dc_total;

% %scale drive cycle
% dc_type = DCtype;
% if(mod(SimCase.Ns,SimCase.Ncell_RC)==0)
%     Vscale = SimCase.Ns/SimCase.Ncell_RC;
%     Iscale = SimCase.Np;
% else
%     error('Number of total cells in series must be able to be divded completely by number of simulation cells'); 
% end
% if(strcmp(dc_type,'Current'))
%     dc_total(:,2) = dc_total(:,2)/Iscale;
% elseif(strcmp(dc_type,'Power'))
% %    dc_total(:,2) = dc_total(:,2)/(Iscale*Vscale);
% elseif(strcmp(dc_type,'Voltage'))
% %    dc_total(:,2) = dc_total(:,2)/Vscale;
% elseif(strcmp(dc_type,'Resistance'))
% %    dc_total(:,2) = dc_total(:,2)/Vscale*Iscale;
% else
%     error('Unknown Drive Cycle Type.');
% end
% cha_scale = SimCase.Chargescale;
% dis_scale = SimCase.Dischargescale;
% dc_total_tmp = dc_total(:,2);
% dc_total_tmp(dc_total_tmp>0)=dc_total_tmp(dc_total_tmp>0)*cha_scale;
% dc_total_tmp(dc_total_tmp<0)=dc_total_tmp(dc_total_tmp<0)*dis_scale;
% dc_total(:,2) = dc_total_tmp;
% SimCase.dc_sim = dc_total;
% SimCase.Iscale = Iscale;
% SimCase.Vscale = Vscale;
end