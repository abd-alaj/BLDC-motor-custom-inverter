% this script is mainly an easy way to create rough K_p and K_i values for
% both torque and speed controllers

% this script makes the following assumptions: 
% 1 - the mathematical model is linear

%% TORQUE CONTROL

omega_c = (2 * pi * f_sw) / 10; 
tau_c = 1 / omega_c;

% for q controller

tau_q = Lq / Rs;
K_pq = Lq / tau_c; 
K_iq = Rs / tau_c; 


% for d controller 

tau_d = Ld / Rs; 
K_pd = Ld / tau_c; 
K_id = Rs / tau_c; 

%% SPEED CONTROL

K_ps = 1.094365863826970e-04;
K_is = 1.530437895709333;

% used for tuning the above values
% omega_speed_target = omega_c / 10; 

% s = tf('s');
% G_plant = 1 / (J * s + B);

% [C_speed, info] = pidtune(G_plant, 'PI', omega_speed_target);

% pidTuner(G_plant, C_speed);

% K_ps = C_speed.Kp;
% K_is = C_speed.Ki;

% values retrieved from PIDTuner app