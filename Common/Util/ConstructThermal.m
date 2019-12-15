function [C,Qext_other,Rt,Rground] = ConstructThermal(Ncell_thermal,Nother_thermal,Cellcp,Cellmass,mcp_other,Vscale,Iscale,Qext_other,Rt_btw_cells,Rtcell2other,Rground)

Nthermal = Ncell_thermal + Nother_thermal;
C = zeros(Nthermal,1);
C(1:Ncell_thermal) = Cellcp*Cellmass;

if(Nother_thermal~=0)
    C(Ncell_thermal+1:end) = mcp_other/(Vscale*Iscale);
    Qext_other = Qext_other/(Vscale*Iscale);
end
Rt = zeros(Nthermal,Nthermal); % thermal resistance matrix

%thermal resistance btw cells
for i = 1:Ncell_thermal
   for j = 1:Ncell_thermal
    if(j-i==1)
        Rt(i,j) = Rt_btw_cells;
    end
   end
end
%thermal resistance btw cell and other components
if(Nother_thermal~=0)
    Rt(1:Ncell_thermal,Ncell_thermal+1:end) = Rtcell2other;
end
%make the matrix symmetric
for i = 1:Nthermal
    for j=1:i
        Rt(i,j) = Rt(j,i);
    end
end

if(Nother_thermal~=0)
    Rground(Ncell_thermal+1:end,:)=Rground(Ncell_thermal+1:end,:)*(Vscale*Iscale);
end

end