% Đọc dữ liệu từ file Excel
data = readtable('DuLieuCuoiCung2.xlsx');
% Trích xuất các cột
t = data{:,1};
x_deg = data{:,2};
z_deg = data{:,3};
x_v = data{:,4};
z_v = data{:,5};
x = data{:,6};
T_l = data{:,7};
T_r = data{:,8};

% Vẽ đồ thị
fontName = 'Times New Roman';
fontSize = 13;

subplot(2,1,1);
plot(t, T_l, 'Color', [1.0 0.5 0.0], 'LineWidth', 1);
xlabel('Thời gian (s)');
ylabel('Mô men (N.m)');
legend({'Mô men bánh trái'},'Location', 'northeast');
grid on;
set(gca, 'FontName', fontName, 'FontSize', fontSize);

subplot(2,1,2);
plot(t, T_r, 'Color', [1.0 0.08 0.58], 'LineWidth', 1);
xlabel('Thời gian (s)');
ylabel('Mô men (N.m)');
legend({'Mô men bánh phải'},'Location', 'northeast');
grid on;
set(gca, 'FontName', fontName, 'FontSize', fontSize);