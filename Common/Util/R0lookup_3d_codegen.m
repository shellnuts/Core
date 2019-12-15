function R0 = R0lookup_3d_codegen(RCdata,SOC,T,I,Chflag)

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
    
    Imin = 1e-3; %minimum current to consider as rest

    SOC=min(max(SOC,Xsoc_min),Xsoc_max);
    T=min(max(T,WT_min),WT_max);

    if(I>Imin)          % Charge 
        I = min(max(I,Zc_min),Zc_max);
        I = ones(length(SOC),1)*I;
        R0 = interpn(Xsoc_data,Zc_data,WT_data,R0c,SOC,I,T);   

    elseif(I<-Imin)     % Discharge
        I = abs(I);
        I = min(max(I,Zd_min),Zd_max);
        I = ones(length(SOC),1)*I;
        R0 = interpn(Xsoc_data,Zd_data,WT_data,R0d,SOC,I,T);

    else                % Rest
        if(Chflag) %Rest after charge  
            I = abs(I);
            I = min(max(I,Zc_min),Zc_max);
            I = ones(length(SOC),1)*I;
            R0 = interpn(Xsoc_data,Zc_data,WT_data,R0c,SOC,I,T); 
 
        else       %Rest after discharge
            I = abs(I);
            I = min(max(I,Zd_min),Zd_max);
            I = ones(length(SOC),1)*I;
            R0 = interpn(Xsoc_data,Zd_data,WT_data,R0d,SOC,I,T);

        end
    end
end