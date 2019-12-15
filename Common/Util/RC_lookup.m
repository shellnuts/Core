function y = RC_lookup(current,soc,temp,RCdata,Xsoc,Z,WT)
soc0 = min(Xsoc);
soc1 = max(Xsoc);
soc = min(max(soc0,soc),soc1);
current0 = min(Z);
current1 = max(Z);
current = min(max(current0,current),current1);
temp0 = min(WT);
temp1 = max(WT);
temp = min(max(temp0,temp),temp1);
y = interpn(Xsoc,Z,WT,RCdata,soc,current,temp);
end
