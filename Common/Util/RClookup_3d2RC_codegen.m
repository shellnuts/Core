function [R0,R1,R2,C1,C2,Chflag] = RClookup_3d2RC_codegen(RCdata,SOC,T,I,Chflag)
    
%   2-RC model    
    Rpc1 = RCdata.Rpc1;
    Rpd1 = RCdata.Rpd1;
    Cpc1 = RCdata.Cpc1;
    Cpd1 = RCdata.Cpd1;
    Rpc2 = RCdata.Rpc2;
    Rpd2 = RCdata.Rpd2;
    Cpc2 = RCdata.Cpc2;
    Cpd2 = RCdata.Cpd2;
    
    R0c = RCdata.R0c;
    R0d = RCdata.R0d;

    Xsoc_data = RCdata.Xsoc;
    Zc_data = RCdata.Zc;
    Zd_data = RCdata.Zd;
    WT_data = RCdata.WT;
    
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
    if(I>Imin)          %   Charge
        Chflag = 1;  
        I = min(max(I,Zc_min),Zc_max);
        I = ones(length(SOC),1)*I;

        R1 = interpn(Xsoc_data,Zc_data,WT_data,Rpc1,SOC,I,T);
        C1 = interpn(Xsoc_data,Zc_data,WT_data,Cpc1,SOC,I,T); 
        R2 = interpn(Xsoc_data,Zc_data,WT_data,Rpc2,SOC,I,T);
        C2 = interpn(Xsoc_data,Zc_data,WT_data,Cpc2,SOC,I,T); 

        R0 = interpn(Xsoc_data,Zc_data,WT_data,R0c,SOC,I,T); 
        
    elseif(I<-Imin)     %   Discharge
        Chflag = 0;
        I = abs(I);
        I = min(max(I,Zd_min),Zd_max);
        I = ones(length(SOC),1)*I;

        R1 = interpn(Xsoc_data,Zd_data,WT_data,Rpd1,SOC,I,T);
        C1 = interpn(Xsoc_data,Zd_data,WT_data,Cpd1,SOC,I,T);    
        R2 = interpn(Xsoc_data,Zd_data,WT_data,Rpd2,SOC,I,T);
        C2 = interpn(Xsoc_data,Zd_data,WT_data,Cpd2,SOC,I,T);  


        R0 = interpn(Xsoc_data,Zd_data,WT_data,R0d,SOC,I,T);

    else                %   Rest
        if(Chflag)      %   Rest after charge  
            I = abs(I);
            I = min(max(I,Zc_min),Zc_max);
            I = ones(length(SOC),1)*I;

            R1 = interpn(Xsoc_data,Zc_data,WT_data,Rpc1,SOC,I,T);
            C1 = interpn(Xsoc_data,Zc_data,WT_data,Cpc1,SOC,I,T); 
            R2 = interpn(Xsoc_data,Zc_data,WT_data,Rpc2,SOC,I,T);
            C2 = interpn(Xsoc_data,Zc_data,WT_data,Cpc2,SOC,I,T); 

            R0 = interpn(Xsoc_data,Zc_data,WT_data,R0c,SOC,I,T); 
        else             %  Rest after discharge
            I = abs(I);
            I = min(max(I,Zd_min),Zd_max);
            I = ones(length(SOC),1)*I;

            R1 = interpn(Xsoc_data,Zd_data,WT_data,Rpd1,SOC,I,T);
            C1 = interpn(Xsoc_data,Zd_data,WT_data,Cpd1,SOC,I,T);    
            R2 = interpn(Xsoc_data,Zd_data,WT_data,Rpd2,SOC,I,T);
            C2 = interpn(Xsoc_data,Zd_data,WT_data,Cpd2,SOC,I,T);  


            R0 = interpn(Xsoc_data,Zd_data,WT_data,R0d,SOC,I,T);
        end
    end
end