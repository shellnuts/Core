function error = Tvmodel_obj(x0,Pdata,timedata,Tmdata,Tcdata)
Rt = x0(1);
tau = x0(2);
Pavg = 0;
Tm = zeros(length(timedata),1);
dt = diff(timedata);
Tm(1) = Tmdata(1);
for i = 2:length(timedata)
    [Tm(i),Pavg] = Tcell2Tm(Rt,tau,Tcdata(i),Pdata(i),dt(i-1),Pavg);
end

error = Tm - Tmdata;


end