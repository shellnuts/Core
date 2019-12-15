function Q = calQcell(I,V,T,ocv,dVdT)
    Qire = abs(I.*(ocv-V));
    Qrev = I*T.*dVdT;
    Q = Qire + Qrev;
end