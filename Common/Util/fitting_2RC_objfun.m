function error = fitting_2RC_objfun(x0,timedata,Idata,Vpdata)
    dt = diff(timedata);
    Vp = zeros(2,length(Vpdata));
    tau = [x0(3);x0(4)];
    R = [x0(1);x0(2)];
    for itime = 2:length(timedata)
        Vp(:,itime) = stateSolver(dt(itime-1),Vp(:,itime-1),tau,Idata(itime)*R,'euler');
    end
    Vp = sum(Vp);
    error = Vp'-Vpdata;
    
end