function [Vcell,SOC,Vp1,Vp2,Vh,Vocv,dVdT,Chflag]=TimeUpdateAll(dt,SOC0,T0,Vp10,Vp20,Vh0,I,I0,ocvdata,RCdata,capacity,Chflag0,Rscale)

    [R0,R1,R2,C1,C2,Chflag] = RClookup_3d2RC_SIM(RCdata,SOC0,T0,I,Chflag0); % update RC parameters
    R0 = R0.*Rscale;
    R1 = R1.*Rscale;
    R2 = R2.*Rscale;
    C1 = C1./Rscale;
    C2 = C2./Rscale;
    [Vp1,Vp2]=VpUpdate_2RC(dt,I,I0,Vp10,Vp20,R1,R2,C1,C2);                  % update polarization voltages
    SOC=SOCUpdate(dt,I,I0,SOC0,capacity);                                   % update soc
    [Vocv_avg,Vocv_c,Vocv_d,dVdT] = ocvModel_SIM(SOC,T0,ocvdata);           % update OCVs
    Vh = Vhys(dt,I,Vocv_c,Vocv_d,capacity,Vh0,ocvdata.alpha);               % update hysterisis
    Vocv = Vocv_avg + Vh;                                                   % calculate OCV
    Vcell = Vocv + I*R0 + Vp1 + Vp2 ;                                       % cell voltage time update
end