function [Tcell,Pavg] = Tm2Tcell(Rt,tau,Tm,ts,Pavg)
alpha = ts/tau; 
Pavg = (1-alpha)*Pavg+alpha*P;
Tcell = Tm - Rt*Pavg;

end