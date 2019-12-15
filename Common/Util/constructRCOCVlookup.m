function [RCdata_SIM,ocvdata_SIM] = constructRCOCVlookup(RCdata,ocvdata)
    Xsoc = RCdata.Xsoc;
    Zd = RCdata.Zd;
    Zc = RCdata.Zc;
    WT = RCdata.WT;
    Rpd1 = RCdata.Rpd1;
    Rpc1 = RCdata.Rpc1;
    Cpd1 = RCdata.Cpd1;
    Cpc1 = RCdata.Cpc1;
    Rpd2 = RCdata.Rpd2;
    Rpc2 = RCdata.Rpc2;
    Cpd2 = RCdata.Cpd2;
    Cpc2 = RCdata.Cpc2;
    
    R0d = RCdata.R0d;
    R0c = RCdata.R0c;
    
    [X1,Y1,Z1] =ndgrid(Xsoc,Zd,WT);
    [X2,Y2,Z2] =ndgrid(Xsoc,Zc,WT);
    

    RCdata_SIM.F_Rpd1 = griddedInterpolant(X1,Y1,Z1,Rpd1);
    RCdata_SIM.F_Cpd1 = griddedInterpolant(X1,Y1,Z1,Cpd1);
    RCdata_SIM.F_Rpc1 = griddedInterpolant(X2,Y2,Z2,Rpc1);
    RCdata_SIM.F_Cpc1 = griddedInterpolant(X2,Y2,Z2,Cpc1);
    RCdata_SIM.F_Rpd2 = griddedInterpolant(X1,Y1,Z1,Rpd2);
    RCdata_SIM.F_Cpd2 = griddedInterpolant(X1,Y1,Z1,Cpd2);
    RCdata_SIM.F_Rpc2 = griddedInterpolant(X2,Y2,Z2,Rpc2);
    RCdata_SIM.F_Cpc2 = griddedInterpolant(X2,Y2,Z2,Cpc2);
    
    RCdata_SIM.F_R0d = griddedInterpolant(X1,Y1,Z1,R0d);
    RCdata_SIM.F_R0c = griddedInterpolant(X2,Y2,Z2,R0c);
    RCdata_SIM.Xsoc = RCdata.Xsoc;
    RCdata_SIM.Zd = RCdata.Zd;
    RCdata_SIM.Zc = RCdata.Zc;
    RCdata_SIM.WT = RCdata.WT;

    

    ocvdata_SIM.F_OCVc = griddedInterpolant(ocvdata.OCVc(:,1),ocvdata.OCVc(:,2));
    ocvdata_SIM.F_OCVd = griddedInterpolant(ocvdata.OCVd(:,1),ocvdata.OCVd(:,2));
    ocvdata_SIM.F_dVdT = griddedInterpolant(ocvdata.dVdT(:,1),ocvdata.dVdT(:,2));
    ocvdata_SIM.alpha = ocvdata.alpha;
    ocvdata_SIM.Tref = ocvdata.Tref;
end