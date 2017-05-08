% MATLAB code to generate test files that exercise a JBIG codec

width = 1200;
img = [];
for y=1:650;
    mx = ceil(y / 5);
    line = uint8(repmat(round(rand(1,mx)),1,ceil(width/mx))) == 1;
    img = [img ; line(1:width)];
end
imwrite(img, 'mx.png', 'PNG');
