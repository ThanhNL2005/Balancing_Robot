% load dữ liệu mô phỏng của x và psi
modelName = 'MyControl_3SMCs_2022b_Ver';
load_system(modelName);
simOut = sim(modelName);
output = simOut.get('OutputData');

% load giá trị đặt của x và psi
ref = simOut.get('RefData');

[X, Y] = computeTrajectory(output);
[X_r, Y_r] = computeTrajectory(ref);

fontName = 'Times New Roman';
fontSize = 13;
% Vẽ đồ thị
figure;
plot(X, Y, 'b-', 'LineWidth', 1); hold on;
plot(X_r, Y_r, 'r--', 'LineWidth', 1);
xlabel('n_{1}', 'Interpreter', 'tex', 'FontName', fontName, 'FontSize', fontSize);
ylabel('n_{2}', 'Interpreter', 'tex', 'FontName', fontName, 'FontSize', fontSize);
title('Quỹ đạo điểm chuyển động của xe theo On_{1}n_{2}');
legend({'Quỹ đạo mô phỏng', 'Quỹ đạo đặt'}, 'Interpreter', 'none', 'FontSize', 13, 'Location', 'northeast');
axis equal;
grid on;
set(gca, 'FontName', fontName, 'FontSize', fontSize);

% Hàm tính quỹ đạo trả về [x,y]
function [X, Y] = computeTrajectory(data)
% COMPUTETRAJECTORY Tính quỹ đạo trong Oxy từ dữ liệu q và psi
%   data: ma trận Nx2, cột 1 là q (dịch chuyển), cột 2 là psi (góc quay)
%   X, Y: tọa độ điểm chuyển động trong hệ cố định Oxy

    q = data(:, 1);     % Dịch chuyển tích lũy
    psi = data(:, 2);   % Góc quay tích lũy

    % Khởi tạo vị trí ban đầu của O'
    x_pos = 0;
    y_pos = 0;

    % Mảng lưu các vị trí quỹ đạo
    N = length(q);
    X = zeros(N, 1);
    Y = zeros(N, 1);

    % Gán điểm đầu tiên
    X(1) = x_pos;
    Y(1) = y_pos;

    for i = 2:N
        % Tính độ thay đổi vị trí và góc giữa hai bước
        dq   = q(i)   - q(i-1);
        psi_ = psi(i-1);   % Hướng của trục tại thời điểm trước khi quay

        % Cập nhật vị trí mới của O' bằng cách dịch dq theo hướng psi_
        dx = dq * cos(psi_);
        dy = dq * sin(psi_);
        x_pos = x_pos + dx;
        y_pos = y_pos + dy;

        % Ghi nhận tọa độ mới
        X(i) = x_pos;
        Y(i) = y_pos;
    end
end
