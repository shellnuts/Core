function I = Power2Current(I0,SOC0,T0,V0,P,RCdata,itermax,Chflag,Rint,Vscale,Iscale,Rscale)
    R0 = R0lookup_3d_SIM(RCdata,SOC0,T0,I0,Chflag);
    R0 = R0.*Rscale;
    Vf = V0 - I0*R0;
    R0 = sum(R0)*Vscale*Iscale+Iscale*Iscale*Rint;
    Vf = sum(Vf)*(Vscale*Iscale);
    for i = 1:itermax
        I = (sqrt(Vf*Vf+4*R0*P)-Vf)/(2*R0);
        if(~isreal(I))
            I = 0;
        end
        R0 = R0lookup_3d_SIM(RCdata,SOC0,T0,I,Chflag);
        R0 = R0.*Rscale;
        R0 = sum(R0)*Vscale*Iscale+Iscale*Iscale*Rint;
        if(abs(I-I0) < 0.01)
            break;
        else
            I0 = I;
        end
    end
end