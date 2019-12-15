function [dc,Iscale,Vscale] = scaleDC(DCtype,Ns_real,Np_real,Ns_RC,dc,ChScale,DisScale)
%scale drive cycle

if(mod(Ns_real,Ns_RC)==0)
    Vscale = Ns_real/Ns_RC;
    Iscale = Np_real;
else
    error('Number of total cells in series must be able to be divded completely by number of simulation cells'); 
end
if(DCtype == 1) % Current drive cycle
    dc(:,2) = dc(:,2)/Iscale;
else
    error('Unknown Drive Cycle Type.');
end

dc_total_tmp = dc(:,2);
dc_total_tmp(dc_total_tmp>0)=dc_total_tmp(dc_total_tmp>0)*ChScale;
dc_total_tmp(dc_total_tmp<0)=dc_total_tmp(dc_total_tmp<0)*DisScale;
dc(:,2) = dc_total_tmp;
end