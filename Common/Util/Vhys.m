function Vh = Vhys(dt,I,Vocv_c,Vocv_d,Cap,Vh0,alpha)
        Imin = 100e-6;
        Vhmax = 0.5*(Vocv_c - Vocv_d);  %maximum hysterisis voltage
        if(I<-Imin)
            Vhmax = - Vhmax;    %Vhmax is negative for discharge and positive for charge
        end
        beta = abs(alpha*I./(3600*Cap));
        Vh = ((1-dt*beta/2).*Vh0+dt*beta.*Vhmax)./(1+dt*beta/2);
end