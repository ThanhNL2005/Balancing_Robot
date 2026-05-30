clc; clear; 
close all;

load('x_kb1.mat');load('theta_kb1.mat');
load('psi_kb1.mat');load('torques_kb1.mat');

load('x_kb2.mat');load('theta_kb2.mat');
load('psi_kb2.mat');load('torques_kb2.mat');

load('x_kb3.mat');load('theta_kb3.mat');
load('psi_kb3.mat');load('torques_kb3.mat');

load('x_kb4.mat');load('theta_kb4.mat');
load('psi_kb4.mat');load('torques_kb4.mat');

load('x_kb5.mat');load('theta_kb5.mat');
load('psi_kb5.mat');load('torques_kb5.mat');

load('x_kb6.mat');load('theta_kb6.mat');
load('psi_kb6.mat');load('torques_kb6.mat');

%% Plot
% Plot configures
fontName = 'Times New Roman';
fontSize = 13;

fig_position = [100 100];           % Window position [x y]
fig_size = [13 6] * 70;            % Window size [width height]

% Figure data
% t = x_kb1(1,:);val = x_kb1(2,:);val_ref = x_kb1(3,:);
% t = theta_kb1(1,:);val = theta_kb1(2,:);val_ref = theta_kb1(3,:);
% t = psi_kb1(1,:);val = psi_kb1(2,:);val_ref = psi_kb1(3,:);
% t = torques_kb1(1,:);val = torques_kb1(2,:);val_ref = torques_kb1(3,:);

% t = x_kb2(1,:);val = x_kb2(2,:);val_ref = x_kb2(3,:);
% t = theta_kb2(1,:);val = theta_kb2(2,:);val_ref = theta_kb2(3,:);
% t = psi_kb2(1,:);val = psi_kb2(2,:);val_ref = psi_kb2(3,:);
% t = torques_kb2(1,:);val = torques_kb2(2,:);val_ref = torques_kb2(3,:);

t = x_kb4(1,:);val = x_kb4(2,:);val_ref = x_kb4(3,:);
% t = theta_kb4(1,:);val = theta_kb4(2,:);val_ref = theta_kb4(3,:);
% t = psi_kb4(1,:);val = psi_kb4(2,:);val_ref = psi_kb4(3,:);
% t = torques_kb4(1,:);val = torques_kb4(2,:);val_ref = torques_kb4(3,:);

% t = x_kb6(1,:);val = x_kb6(2,:);val_ref = x_kb6(3,:);
% t = psi_kb6(1,:);val = psi_kb6(2,:);val_ref = psi_kb6(3,:);
% t = torques_kb6(1,:);val = torques_kb6(2,:);val_ref = torques_kb6(3,:);

% Figure plot
figure('position', [fig_position fig_size], 'Color', [1 1 1], 'Name', 'Tracking Trajectory');
% plot(t, val, 'b', 'LineWidth', 1.5); hold on;
% plot(t, val_ref, 'r--', 'LineWidth', 1.5);
plot(t, val, 'b', 'LineWidth', 2); hold on;
plot(t, val_ref, 'r--', 'LineWidth', 2);

% Add label 
xlabel('Thời gian (s)', 'Interpreter', 'tex', 'FontName', fontName, 'FontSize', fontSize);
ylabel('Tọa độ x (m)', 'Interpreter', 'tex', 'FontName', fontName, 'FontSize', fontSize);
% ylabel('Góc nghiêng \theta (rad)', 'Interpreter', 'tex', 'FontName', fontName, 'FontSize', fontSize);
% ylabel('Góc quay \psi (rad)', 'Interpreter', 'tex', 'FontName', fontName, 'FontSize', fontSize);
% ylabel('Mô-men hai bánh (N.m)', 'Interpreter', 'tex', 'FontName', fontName, 'FontSize', fontSize);

% Add legend
legend({'$x$', '$x_{ref}$'}, 'Interpreter', 'latex', 'FontSize', 13, 'Location', 'northeast');
% legend({'$\theta$', '$\theta_{ref}$'}, 'Interpreter', 'latex', 'FontSize', 13, 'Location', 'northeast');
% legend({'$\psi$', '$\psi_{ref}$'}, 'Interpreter', 'latex', 'FontSize', 13, 'Location', 'northeast');
% legend({'Mô-men bánh trái', 'Mô-men bánh phải'}, 'Interpreter', 'latex', 'FontSize', 13, 'Location', 'northeast');

grid on;
box on;
axis tight;

% % Tạo inset zoom
% axes('position',[.2 .45 .15 .25])
% box on
% plot(t, x, 'b', 'LineWidth', 2); hold on;
% plot(t, xd, 'r--', 'LineWidth', 2);
% xlim([0 12]); ylim([0 5]);
% set(gca, 'FontSize', 10);
% 
% % Vẽ mũi tên chỉ vùng zoom
% annotation('textarrow', [0.28, 0.21], [0.58, 0.65]);

% Tăng cỡ font toàn đồ thị
set(gca, 'FontName', fontName, 'FontSize', fontSize);