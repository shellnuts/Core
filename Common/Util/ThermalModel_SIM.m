function T = ThermalModel_SIM(dt,Rt,C,T0,Tground,Rground,Q,Q0)
% Thermal network model using C-N time discrtezation method
% Rt is time constatnt matrix between each two nodes, which is set up
% outside of the function
Q = 0.5*(Q+Q0);
if(isempty(Rt))
    % single node model
    dtdtau = dt./(2*C*Rground);
    T = ((1-sum(dtdtau))*T0+sum(2*dtdtau.*Tground)+Q*dt./C)/(1+sum(dtdtau));
else
    % Rt is matrix of thermal resistances between components. Diagonal
    % values should be zero. Rt is set up before simulation and does not
    % change with time. 
    % Rground :  row i are the thermal resistances between component i and
    % and all the Tgrounds.

    Aig =2*C.*Rground;
    Aig = dt./Aig;
    Aig(isinf(Aig)) = 0;
    B = Q./C*dt;

    Aij = 2*C.*Rt;
    Aij = -dt./Aij;
    Aij(isinf(Aij)) = 0;
    A=Aij;
    Adiag = 1-sum(Aij,2)+sum(Aig,2);
    N = length(C);
    A(1:N+1:end) = Adiag; %set diagnal values of matrix A
    B = B + T0.*(1+sum(Aij,2)-sum(Aig,2))-Aij*T0+2*Aig*Tground;

    T = A\B;
end

end