function [SOC2,Vp12,Vp22,Vh2,Vest,Vocv,P,K1,K2,K3,Chflag] = EKF(dt,I,Vmes,T,RCdata,ocvdata,Cap,SOC0,Vp10,Vp20,Vh0,P,Chflag,Q,R)
% Step 0: RC paramter and ocv look up
    Imin = 0;
    if(I>Imin)  %Charge
        Chflag =1;
        [R0,R1,R2,C1,C2] = RClookup_3d2RC_C_mex(RCdata,SOC0,T,I);
    elseif(I<=-Imin) %Discharge
        Chflag = 0;
        [R0,R1,R2,C1,C2] = RClookup_3d2RC_D_mex(RCdata,SOC0,T,I);
    else    %Rest
        if(Chflag == 1) % Rest after charge
            [R0,R1,R2,C1,C2] = RClookup_3d2RC_C_mex(RCdata,SOC0,T,I);
        elseif(Chflag == 0) % Rest after discharge
            [R0,R1,R2,C1,C2] = RClookup_3d2RC_D_mex(RCdata,SOC0,T,I);
        end
    end
    [Vocv_avg,Vocv_c,Vocv_d,alpha,dUdSOC] = ocvModel_mex(SOC0,T,ocvdata);

% Step 1: state update
    A1 = 1;
    A2 = 1-dt/(R1*C1);
    A3 = 1-dt/(R2*C2);
    B1 = dt/(Cap*3600);
    B2 = dt/C1;
    B3 = dt/C2;
    
    SOC1 = A1*SOC0 + B1*I;    
    Vp11 = A2*Vp10 + B2*I;
    Vp21 = A3*Vp20 + B3*I;
    
    Vhmax = 0.5*(Vocv_c - Vocv_d);  %maximum hysterisis voltage
    if(I<-Imin)
        Vhmax = - Vhmax;    %Vhmax is negative for discharge and positive for charge
    end
    beta = abs(alpha*I./(3600*Cap));
    Vh2 = ((1-dt*beta/2).*Vh0+dt*beta.*Vhmax)./(1+dt*beta/2); % no correction for Vh for now
    
% Step 2: Covariance matrix(P) update    
    P(1) = A1^2*P(1);
    P(2) = A1*A2*P(2);
    P(3) = A1*A3*P(3);
    P(4) = A1*A2*P(4);
    P(5) = A2^2*P(5);
    P(6) = A2*A3*P(6);
    P(7) = A1*A3*P(7);
    P(8) = A2*A3*P(8);
    P(9) = A3^2*P(9);
    P(1) = P(1) + Q(1);
    P(5) = P(5) + Q(2);
    P(9) = P(9) + Q(3);
    
% Step 3: Voltage estimation
    Vocv = Vocv_avg + Vh2;
    Vest = Vocv + I*R0 + Vp11 + Vp21;
    
% Step 4: Calculate Karman gain
    C1 = dUdSOC;
    C2 = 1;
    C3 = 1;

    PCT1 = P(1)*C1+P(2)*C2+P(3)*C3;
    PCT2 = P(4)*C1+P(5)*C2+P(6)*C3;
    PCT3 = P(7)*C1+P(8)*C2+P(9)*C3;
    CPCT = C1*PCT1+C2*PCT2+C3*PCT3;
    CPCT = CPCT + R;
    K1 = PCT1/CPCT;
%    K1 = 0.99*K0+0.01*K1;
    K1 = max(0,K1);
    K2 = PCT2/CPCT;
    K3 = PCT3/CPCT;
%    K1=0;
%    K2=0;
%    K3=0;

% Step 5: State corrections
    dSOC = K1*(Vmes-Vest);
    SOC2 = SOC1 + dSOC;
    Vp12 = Vp11 + K2*(Vmes-Vest);
    Vp22 = Vp21 + K3*(Vmes-Vest);
    
% Step 6: Covariance matrix(P) correction
    CP1 = C1*P(1)+C2*P(4)+C3*P(7);
    CP2 = C1*P(2)+C2*P(5)+C3*P(8);
    CP3 = C1*P(3)+C2*P(6)+C3*P(9);
    P(1) = P(1) - K1*CP1;
    P(2) = P(2) - K1*CP2;
    P(3) = P(3) - K1*CP3;
    P(4) = P(4) - K2*CP1;
    P(5) = P(5) - K2*CP2;
    P(6) = P(6) - K2*CP3;
    P(7) = P(7) - K3*CP1;
    P(8) = P(8) - K3*CP2;
    P(9) = P(9) - K3*CP3;
end