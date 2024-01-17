h0 = inline('(1+0.2*sin(0.9*t)).*sin(100*t)');
h1 = inline('(1+0.2*sin(1.5*t)).*sin(100*t)');
h2 = inline('abs(sin(0.9*t)-sin(1.5*t)).*sin(100*t)');

x = 0 : pi/1000 : 6*pi;

plot(x,h2(x))
