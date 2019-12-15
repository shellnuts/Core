function [R0,x,soc_avg,dsoc,Tavg,Vsim,Vocv,Vdata,timedata,Idata,resnorm]=RCpulsefitting(ocvdata,timedata,Idata,Vdata,soc0,Tdata,capacity,mode,x0,lb,ub)


ocvdata.F_OCVc = griddedInterpolant(ocvdata.OCVc(:,1),ocvdata.OCVc(:,2));
ocvdata.F_OCVd = griddedInterpolant(ocvdata.OCVd(:,1),ocvdata.OCVd(:,2));
ocvdata.F_dVdT = griddedInterpolant(ocvdata.dVdT(:,1),ocvdata.dVdT(:,2));


soc = soc0 + cumtrapz(timedata,Idata)/(capacity*3600);
soc_avg = mean(soc);
dsoc = soc(end) -soc(1);
Tavg = mean(Tdata);

[Vocv_avg,Vocv_c,Vocv_d,dVdT] = ocvModel(soc,Tdata,ocvdata);


if(strcmp(mode,'Charge'))
    Vocv = Vocv_c;
elseif(strcmp(mode,'Discharge'))
    Vocv = Vocv_d;
else
    error('unknown fitting mode. The fitting mode must be either Charge or Discharge');
end


Imin = 0.1; %maximum current a pulse to be considered as rest

indx_pulse_start = find(abs(Idata)>Imin,1);
indx_pulse_end = find(abs(Idata)>Imin,1,'last');
indx_data = indx_pulse_start:indx_pulse_end;
Vdata = Vdata(indx_data);
Idata = Idata(indx_data);
timedata = timedata(indx_data);
Vocv = Vocv(indx_data);
dV = Vdata - Vocv;
R0 = dV(1)/Idata(1);
Vpdata = dV-dV(1);

options = optimoptions('lsqnonlin','MaxFunctionEvaluations',1e5,'MaxIterations',1e5,'StepTolerance',1e-14);

f=@(x) fitting_2RC_objfun(x,timedata,Idata,Vpdata);

[x,resnorm,residual] = lsqnonlin(f, x0,lb,ub,options);

Vsim = Vpdata+residual+Idata(1)*R0+Vocv;

end

