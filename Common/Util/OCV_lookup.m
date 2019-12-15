function y = OCV_lookup(Xsoc,WT,OCVdata,soc,temp)
    soc0 = min(Xsoc);
    soc1 = max(Xsoc);
    soc = min(max(soc0,soc),soc1);
    temp0 = min(WT);
    temp1 = max(WT);
    temp = min(max(temp0,temp),temp1);
    y = interp2(Xsoc,WT,OCVdata,soc,temp);
end