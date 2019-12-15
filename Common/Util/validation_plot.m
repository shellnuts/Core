time_test = (Data(1,:))';
SOC_test = (Data(2,:))'/100;
Vcell_test = (Data(4:6,:))'/1000;
Tcell_test = (Data(7:9,:))';

subplot(311)
plot(time_test,SOC_test,Simdata.time,Simdata.SOC);
subplot(312)
plot(time_test,Vcell_test,Simdata.time,Simdata.Vcell);
subplot(313)
plot(time_test,Tcell_test,Simdata.time,Simdata.Tcell);