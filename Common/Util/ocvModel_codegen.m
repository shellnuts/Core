function [Vocv_avg,Vocv_c,Vocv_d,dVdT] = ocvModel_codegen(soc,T,ocvdata)
    soc = max(min(soc,1),0);
    
    dVdTdata = ocvdata.dVdT;
    OCVcdata = ocvdata.OCVc;
    OCVddata = ocvdata.OCVd;

    
    dVdT = interp1(dVdTdata(:,1),dVdTdata(:,2),soc);
    Tref = ocvdata.Tref;
    Vocv_c = interp1(OCVcdata(:,1),OCVcdata(:,2),soc)+dVdT.*(T-Tref);
    Vocv_d = interp1(OCVddata(:,1),OCVddata(:,2),soc)+dVdT.*(T-Tref);

    Vocv_avg = 0.5*(Vocv_c+Vocv_d);
end