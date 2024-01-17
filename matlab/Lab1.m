

## Headline title
#
# Some *bold*, _italic_, or |monospaced| Text with
# a <https://www.octave.org link to *GNU Octave*>.
##

##
#*These part generates the sine function by taking*
#_Program for sine wave_
##
t=0:0.1:10;  %Generates
y=sin(2*pi*t);
subplot(3,3,1);
plot(t,y,'k');
xlabel('Time');
ylabel('Amplitude');
title('Sine wave');

%Program for cosine wave
t=0:0.1:10;
y=cos(2*pi*t);
subplot(3,3,2);
plot(t,y,'k');
xlabel('Time');
ylabel('Amplitude');
title('Cosine wave');

%Program for square wave
t=0:0.001:10;
y=square(t);
subplot(3,3,3);
plot(t,y,'k');
xlabel('Time');
ylabel('Amplitude');
title('Square wave');

%Program for sawtooth wave
t=0:0.1:10;
y=sawtooth(t);
subplot(3,3,4);
plot(t,y,'k');
xlabel('Time');
ylabel('Amplitude');
title('Sawtooth wave');

%Program for Triangular wave
t=0:.0001:20;
y=sawtooth(t,.5); % sawtooth with 50% duty cycle(triangular)
subplot(3,3,5);
plot(t,y);
ylabel ('Amplitude');
xlabel ('Time Index');
title('Triangular waveform');

%Program for Sinc Pulse
t=-10:.01:10;
y=sinc(t);
axis([-10 10 -2 2]);
subplot(3,3,6)
plot(t,y)
ylabel ('Amplitude');
xlabel ('Time Index');
title('Sinc Pulse');

% Program for Exponential Growing signal
t=0:.1:8;
a=2;
y=exp(a*t);
subplot(3,3,7);
plot(t,y);


ylabel ('Amplitude');
xlabel ('Time Index');
title('Exponential growing Signal');

% Program for Exponential Growing signal
t=0:.1:8;
a=2;
y=exp(-a*t);
subplot(3,3,8);
plot(t,y);
ylabel ('Amplitude');
xlabel ('Time Index');
title('Exponential decaying Signal');

