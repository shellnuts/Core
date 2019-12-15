load('./Cell Model/8Ah_Bsample.mat');
load('./Fitting/testdata.mat');

ocvdata = model.ocvdata;
x0 = [1.0e-3;1.0e-3;1;20];
lb = [1.0e-4;1.0e-4;0.1;10];
ub = [1.0e-2;1.0e-2;10;100];
lb = x0/100;
ub = x0*100;
Idata = I;
timedata = time;
Vdata = Vcell';
ocv0 = Vdata(1);
mode = 'Discharge';
soc0 = interp1(ocvdata.OCVd(:,2),ocvdata.OCVd(:,1),ocv0);
Tdata = 25*ones(length(timedata),1);
capacity = 8;

[R0,x,soc_avg,dsoc,Tavg,Vsim,Vocv,Vdata1,timedata1,Idata1,resnorm]=RCpulsefitting(ocvdata,timedata,Idata,Vdata,soc0,Tdata,capacity,mode,x0,lb,ub);

plot(timedata,Vdata);hold on;
plot(timedata1,Vsim,'o')