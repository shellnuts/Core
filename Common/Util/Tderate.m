function [Ilim,DegraFac]=Tderate(Ilim_RC,T1,T2,temp,Islew_max)
    SMALL = 1e-6;
    if(temp > T2)
        temp1 = T2;
    elseif(temp < T1)
        temp1 = T1;
    else
        temp1 = temp;
    end
    
    ratio = (temp1-T1)/((T2-T1)+SMALL);
    
    deltaI = ratio * Ilim_RC;
    
    if(deltaI > Islew_max)
        Ilim = Ilim_RC - Islew_max;
        DegraFac = 1-Ilim/(Ilim_RC+SMALL);
        if(Ilim < 0)
            Ilim = 0;
            DegraFac = 1;
        end
    else
        Ilim = Ilim_RC - deltaI;
        DegraFac = ratio;
    end
    

end