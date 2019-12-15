function [SOC]=SOCUpdate(dt,I,I0,SOC0,capacity)
% time update of SOC
% capacity is in Ah

    Iavg = I;
    dtmin = 1; % if dt>dtmin, use average I between two time steps to calculate state update
    dImin = 10;
    
    if(dt>dtmin ||abs(I-I0)>dImin) 
        Iavg = (I+I0)/2; 
    end
    
    
    SOC = SOC0 + dt*Iavg./(capacity*3600);
end 