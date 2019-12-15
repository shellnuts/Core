function [Vocv_avg,Vocv_c,Vocv_d,dVdT] = ocvModel_SIM(soc,T,ocvdata_SIM)
    soc = max(min(soc,1),0);

    F_OCVd = ocvdata_SIM.F_OCVd;
    F_OCVc = ocvdata_SIM.F_OCVc;
    F_dVdT = ocvdata_SIM.F_dVdT;
    
    dVdT = F_dVdT(soc);
    Tref = ocvdata_SIM.Tref;

    Vocv_c = F_OCVc(soc)+dVdT.*(T-Tref);
    Vocv_d = F_OCVd(soc)+dVdT.*(T-Tref);
    
    Vocv_avg = 0.5*(Vocv_c+Vocv_d);
end