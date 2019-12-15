function x = stateSolver(dt, x0, tau, u)
% Author: Wei Zhao
% Date : 06/25/2018
% The function is to update the state vector using the discretization method specified by 'method'. 
% The state update equation is  dx/dt = -x/tau + u, where tau is the time constant and u is input. x0 is
% the previous state vector.

%     if(strcmp(method,'exact'))
%         x = x0.*exp(-dt./tau)+(1-exp(-dt./tau)).*u; %see notes on cell phone
%     elseif(strcmp(method,'euler'))
        x = (1-2*dt./(2*tau+dt)).*x0 + 2*dt./(2*tau+dt).*u; %see notes on cell phone
%     else
%         error('Unknown discritezation method for state update!');
%     end
%     
end