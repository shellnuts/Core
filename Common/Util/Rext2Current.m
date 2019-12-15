function I = Rext2Current(I0,SOC0,T0,V0,Rext,RCdata,itermax,Chflag,Rint,Vscale,Iscale)
    if(Rext == 0)
        I = 0;
    elseif(Rext <0)
        error('Short-circuit Resistance must be larger than 0');
    else
        R0 = R0lookup_3d_SIM(RCdata,SOC0,T0,I0,Chflag);
        Vf = V0 - I0*R0;
        R0 = sum(R0)*Vscale+Rint*Iscale;
        Vf = sum(Vf)*Vscale;
        for i = 1:itermax
            I = -Vf/(R0+Rext);
            if(I<0)
                R0 = R0lookup_3d_SIM(RCdata,SOC0,T0,I,Chflag);
                R0 = sum(R0)*Vscale+Rint*Iscale;
            elseif(I>0)
                error('short-circuit current must be negative');
            end
            if(abs(I-I0) < 0.01)
                break;
            else
                I0 = I;
            end
        end
    end
end