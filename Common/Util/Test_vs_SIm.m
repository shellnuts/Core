timetest = Data(1,:);
vcelltest = Data(5,:);
soctest = Data(2,:)/100;
soctest = soctest';
timetest = timetest';
%timetest = timetest - timetest(1);

vcelltest = vcelltest'/1000;
Itest = Data(3,:);
Itest = -Itest';
Ttest = Data(8,:);
Ttest = Ttest';

timesim = Simdata.time;
vcellsim_max = max(Simdata.Vcell,[],1);
vcellsim_min = min(Simdata.Vcell,[],1);
vcellsim_avg = mean(Simdata.Vcell,1);
vcellsim = [vcellsim_max',vcellsim_min',vcellsim_avg'];
socsim = Simdata.SOC(1,:);
socsim = socsim';
Tsim = mean(Simdata.Tcell,1);
Vocvsim = mean(Simdata.Vocv_cell,1);


figure;
plot(timetest,vcelltest,timesim,vcellsim_avg);
figure;
plot(timetest,soctest,timesim,socsim);
figure;
plot(timetest,Ttest,timesim,Tsim);
figure;
plot(timetest,Itest);


%vcelltest_inter = interp1(timetest,vcelltest,timesim);
vcellsim_avg=vcellsim_avg';
%plot(timetest,vcelltest-vcellsim_avg(2:end));
