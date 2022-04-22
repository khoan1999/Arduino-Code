clear
clc
close all
tot_file =importdata('one_file.txt');
time = tot_file(:,1);
speed = tot_file(:,2).*2*pi*0.33*3.6/60;
voltage = tot_file(:,3);
current = tot_file(:,4);
angle = tot_file(:,5);
time(1) = time(1)/1000;
for i = 2:length(time+1)
   time(i)= time(i-1)+time(i)*1e-3;
end
% figure(1)
% plot(time,angle(1:length(time)),'-x')
% xlabel('time [s]');
% ylabel('angle [deg]');
% title('angle in function of the time');
% 
% figure(2)
% plot(time,current(1:length(time)),'-x')
% xlabel('time [s]');
% ylabel('current [A]');
% title('current in function of the time');
% 
% figure(3)
% plot(time,voltage(1:length(time)))
% xlabel('time [s]');
% ylabel('voltage [V]');
% title('Voltage in function of the time');
% 
% figure(4)
% plot(time,speed(1:length(time)),'-x')
% xlabel('time [s]');
% ylabel('speed [RPM]');
% title('Speed in function of the time');

figure(1)
subplot(2,2,1)
plot(time,speed(1:length(time)),'-x')
xlabel('time [s]');
ylabel('speed [km/h]');
title('Speed in function of the time');

subplot(2,2,2)
plot(time,voltage(1:length(time)))
xlabel('time [s]');
ylabel('voltage [V]');
title('Voltage in function of the time');

subplot(2,2,3)
plot(time,current(1:length(time)),'-x')
xlabel('time [s]');
ylabel('current [A]');
title('current in function of the time');

subplot(2,2,4)
anglesmooth = smooth(time,angle,0.25,'rloess');
plot(time,anglesmooth(1:length(time)),'-x');
hold on
plot(time,angle);
xlabel('time [s]');
ylabel('angle [deg]');
title('angle in function of the time');

