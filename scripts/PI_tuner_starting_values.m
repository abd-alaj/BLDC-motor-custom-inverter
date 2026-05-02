% this script is mainly an easy way to create rough K_p and K_i values for
% both controllers

% this script makes the following assumptions: 
% 1 - the mathematical model is linear
% 2 - there is no decoupling terms
% 3 - there is no cross coupling terms 

omega_c = (2 * pi * f_sw) / 20; 
tau_c = 1 / omega_c;

% for q controller

tau_q = Lq / Rs;
K_pq = Lq / tau_c; 
K_iq = Rs / tau_c; 


% for d controller 

tau_d = Ld / Rs; 
K_pd = Ld / tau_c; 
K_id = Rs / tau_c; 
