Tmdata = (Data(8,:))';
Tcdata = (mean(Simdata.Tcell))';
Pdata = abs(Simdata.Ppack);
timedata = Simdata.time;

x0 = [0.01, 500];
lb = x0/10;
ub = x0*10;

options = optimoptions('lsqnonlin','MaxFunctionEvaluations',1e5,'MaxIterations',1e5,'StepTolerance',1e-14);

f=@(x) Tvmodel_obj(x,Pdata,timedata,Tmdata,Tcdata);

[x,resnorm,residual] = lsqnonlin(f, x0,lb,ub,options);

Tm = residual + Tmdata;

plot(timedata,[Tmdata,Tcdata]);hold on;
plot(timedata,Tm,'LineWidth',3);

%% validation
Tmdata = (Data(8,:))';
Tcdata = (mean(Simdata.Tcell))';
Tcdata = Tcdata(2:end);
Pdata = abs(Simdata.Ppack);
Pdata = Pdata(2:end);
timedata = Simdata.time;
timedata = timedata(2:end);

Tm = zeros(length(timedata),1);
Tm(1) = Tmdata(1);
Pavg = 0;
dt = diff(timedata);
%Tm(1) = Tmdata(1);

for i = 2:length(timedata)
    [Tm(i),Pavg]= Tcell2Tm(x(1),x(2),Tcdata(i),Pdata(i),dt(i-1),Pavg);

end

plot(timedata,[Tm,Tmdata,Tcdata]);