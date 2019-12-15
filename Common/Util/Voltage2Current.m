function I = Voltage2Current(I0,SOC0,T0,V0,V,RCdata,itermax,Chflag,Rint,Vscale)
    if(V==0)
        I = 0;
    elseif(V<0)
        error('Applied voltage must be greater than 0!');
    else
        R0 = R0lookup_3d_SIM(RCdata,SOC0,T0,I0,Chflag);
        Vf = V0 - I0*R0;
        R0 = sum(R0)*Vscale+Rint*Iscale;
        Vf = sum(Vf)*Vscale;
        for i = 1:itermax
            I = (V-Vf)/R0;
            R0 = R0lookup_3d_SIM(RCdata,SOC0,T0,I,Chflag);
            R0 = sum(R0)*Vscale+Rint*Iscale;
            if(abs(I-I0) < 1e-3)
                break;
            else
                I0 = I;
            end
        end
    end
end