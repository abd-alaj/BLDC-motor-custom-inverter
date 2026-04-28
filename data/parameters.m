% PARAMETERS OF THE ROB27478 BLDC MOTOR FROM SPARKFUN ELECTRONICS

% all of these parameters were taken from the datasheet
% ensure at one point to do your own testing on the BLDC motor and update these values

P = 11; 
R_phi = 6.34;
L_phi = 1.08e-3;
Kt = 0.08;
kn = 117; 
J_intert = 56e-7;

% switch this out with actual experimentally determined K_e once BLDC motor is properly sourced
K_e = (1 / kn) * (2 * pi / 60); 

% operational safety limits
V_nom = 12;
I_max = 1.9; 
I_nom = 1.1; 
T_stall = 0.15; 
max_rpm = 1610; 

% chosen parameters
Vdc = 12;
f_sw = 30e3;
T_s = 1 / f_sw;
V_dc = 12; 

lambda = Kt / (1.5 * P);

% controller parameters
target_speed = 400;