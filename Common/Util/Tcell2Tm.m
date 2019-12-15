function [Tm,Pavg] = Tcell2Tm(Rt,tau,Tcell,P,ts,Pavg)
alpha = ts/tau; 
Pavg = (1-alpha)*Pavg+alpha*P;
Tm = Tcell + Rt*Pavg;

end