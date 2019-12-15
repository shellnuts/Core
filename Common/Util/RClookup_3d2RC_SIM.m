function [R0,R1,R2,C1,C2,Chflag] = RClookup_3d2RC_SIM(RCdata_SIM,SOC,T,I,Chflag)
    
%   2-RC model    

    F_Rpd1 = RCdata_SIM.F_Rpd1;
    F_Rpc1 = RCdata_SIM.F_Rpc1;
    F_Cpd1 = RCdata_SIM.F_Cpd1;
    F_Cpc1 = RCdata_SIM.F_Cpc1;
    F_Rpd2 = RCdata_SIM.F_Rpd2;
    F_Rpc2 = RCdata_SIM.F_Rpc2;
    F_Cpd2 = RCdata_SIM.F_Cpd2;
    F_Cpc2 = RCdata_SIM.F_Cpc2;
    
    F_R0d = RCdata_SIM.F_R0d;
    F_R0c = RCdata_SIM.F_R0c;
    
    Xsoc_data = RCdata_SIM.Xsoc;
    Zc_data = RCdata_SIM.Zc;
    Zd_data = RCdata_SIM.Zd;
    WT_data = RCdata_SIM.WT;
    
    Xsoc_max = max(Xsoc_data);
    Xsoc_min = min(Xsoc_data);
    Zc_max = max(Zc_data);
    Zc_min = min(Zc_data);
    Zd_max = max(Zd_data);
    Zd_min = min(Zd_data);
    WT_max = max(WT_data);
    WT_min = min(WT_data);
    
    Imin = 1e-3;
    
    SOC=min(max(SOC,Xsoc_min),Xsoc_max);
    T=min(max(T,WT_min),WT_max);
    if(I>Imin)          % Charge
        Chflag = 1;  
        I = min(max(I,Zc_min),Zc_max);
        I = ones(length(SOC),1)*I;

        R1 = F_Rpc1(SOC,I,T);
        R2 = F_Rpc2(SOC,I,T);
        C1 = F_Cpc1(SOC,I,T);
        C2 = F_Cpc2(SOC,I,T);
  
        R0 = F_R0c(SOC,I,T);
    elseif(I<-Imin)     % Discharge
        Chflag = 0;
        I = abs(I);
        I = min(max(I,Zd_min),Zd_max);
        I = ones(length(SOC),1)*I;


        R1 = F_Rpd1(SOC,I,T);
        R2 = F_Rpd2(SOC,I,T);
        C1 = F_Cpd1(SOC,I,T);
        C2 = F_Cpd2(SOC,I,T);

        R0 = F_R0d(SOC,I,T);
    else                % Rest
        if(Chflag) %Rest after charge  
            I = abs(I);
            I = min(max(I,Zc_min),Zc_max);
            I = ones(length(SOC),1)*I;

            R1 = F_Rpc1(SOC,I,T);
            R2 = F_Rpc2(SOC,I,T);
            C1 = F_Cpc1(SOC,I,T);
            C2 = F_Cpc2(SOC,I,T);

            R0 = F_R0c(SOC,I,T);
        else       %Rest after discharge
            I = abs(I);
            I = min(max(I,Zd_min),Zd_max);
            I = ones(length(SOC),1)*I;

            R1 = F_Rpd1(SOC,I,T);
            R2 = F_Rpd2(SOC,I,T);
            C1 = F_Cpd1(SOC,I,T);
            C2 = F_Cpd2(SOC,I,T);

            R0 = F_R0d(SOC,I,T);
        end
    end
end