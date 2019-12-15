function [Vp1,Vp2]=VpUpdate_2RC(dt,I,I0,Vp10,Vp20,R1,R2,C1,C2)
% time update of polarization voltages of the 2 R-C pairs
    Iavg = I;
    dtmin = 1; % if dt>dtmin, use average I between two time steps to calculate state update
    dImin = 10;
    
    if(dt>dtmin ||abs(I-I0)>dImin) 
        Iavg = (I+I0)/2; 
    end
    
   
    Vp1 = stateSolver(dt, Vp10, R1.*C1, Iavg.*R1);
    Vp2 = stateSolver(dt, Vp20, R2.*C2, Iavg.*R2);
    
end 