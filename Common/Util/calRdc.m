function Rdc = calRdc(OCV,Vcell,I)
      Rdc = abs((Vcell-OCV)./I);
      Rdc(isinf(Rdc)) = 0;
      Rdc(isnan(Rdc)) = 0;
end