function [Ilim,exitFlag]= Ilim_RC(dt,SOC,Temp,Vlim,Vp1,Vp2,Cap,Isign,Xsoc_ocv,WT_ocv,...
                                  OCVdata,Xsoc,Z,WT,R0data,R1data,R2data,C1data,C2data,Rint)
    persistent Ilim0;
    if isempty(Ilim0)
        Ilim0 = 0;
    end
    options = optimset('Display','iter');
    fun = @(x) Ilim_nonLinear_costFun(x,dt,SOC,Temp,Vlim,Vp1,Vp2,Cap,Isign,Xsoc_ocv,WT_ocv, ...
        OCVdata,Xsoc,Z,WT,R0data,R1data,R2data,C1data,C2data,Rint);
    [Ilim,~,exitFlag] = fzero(fun,Ilim0,options);

end

function y = Ilim_nonLinear_costFun(Ilim0,dt,SOC,temp,Vlim,Vp1,Vp2,Cap,Isign,Xsoc_ocv,WT_ocv, ...
            OCVdata,Xsoc,Z,WT,R0data,R1data,R2data,C1data,C2data,Rint)
    soc_end = coulomb_counting(Ilim0*Isign,dt,Cap,SOC);
    ocv_end = OCV_lookup(Xsoc_ocv,WT_ocv,OCVdata,soc_end,temp);
    R0 = RC_lookup(Ilim0,soc_end,temp,R0data,Xsoc,Z,WT);
    R1 = RC_lookup(Ilim0,soc_end,temp,R1data,Xsoc,Z,WT);
    R2 = RC_lookup(Ilim0,soc_end,temp,R2data,Xsoc,Z,WT);
    C1 = RC_lookup(Ilim0,soc_end,temp,C1data,Xsoc,Z,WT);
    C2 = RC_lookup(Ilim0,soc_end,temp,C2data,Xsoc,Z,WT);
    tau1 = R1*C1;
    tau2 = R2*C2;
    y = ocv_end + Vp1*exp(-dt/tau1) + Vp2*exp(-dt/tau2) + Isign*(Ilim0*(R0+Rint) + ...
    Ilim0*R1*(1-exp(-dt/tau1)) + Ilim0*R2*(1-exp(-dt/tau2))) - Vlim;
end