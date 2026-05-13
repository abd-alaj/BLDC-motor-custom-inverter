mdl = 'mathematical_model.slx';

load_system(mdl);

io(1) = linio([mdl '/Plant'], 1, 'input');
io(2) = linio([mdl '/Plant'], 1, 'output');
Plant_q = linearize(mdl, io, operpoint(mdl));

io_d(1) = linio([mdl '/Plant'], 2, 'input');
io_d(2) = linio([mdl '/Plant'], 2, 'output');
Plant_d = linearize(mdl, io_d, operpoint(mdl));

[C_q] = pidtune(Plant_q, 'PI');
[C_d] = pidtune(Plant_d, 'PI');

set_param([mdl '/q controller'],  'P', num2str(C_q.Kp), 'I', num2str(C_q.Ki));
set_param([mdl '/Vd_controller'], 'P', num2str(C_d.Kp), 'I', num2str(C_d.Ki));