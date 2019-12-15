function y = cumAvg(time,data)
    x = cumtrapz(time,data,2);
    time = time- time(1);
    y = x(:,2:end)./(time(2:end))';
    tmp = zeros(size(y,1),1);
    y = [tmp,y];
    y = y';
end