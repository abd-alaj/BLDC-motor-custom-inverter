clear all; clc; 


% run this script immediately during matlab startup
% otherwise you will have matlab pollute the root directory
projectRoot = fileparts(mfilename('fullpath'));

includeFolders = {
    'models', ...
    'models/components', ...
    'scripts', ...
    'data', ...
    'src',
};

for i = 1 : length(includeFolders)
    directory = fullfile(projectRoot, includeFolders{i});
    if exist(directory, 'dir')
        addpath(genpath(directory)); 
    end
end

cd(projectRoot);

addpath(genpath(fullfile(projectRoot, 'models')));
addpath(genpath(fullfile(projectRoot, 'scripts')));
addpath(genpath(fullfile(projectRoot, 'data')));
addpath(genpath(fullfile(projectRoot, 'tests')));

workDir = fullfile(projectRoot, 'work');

if ~exist(workDir, 'dir')
    mkdir(workDir);
end

Simulink.fileGenControl('set', 'CacheFolder', workDir, 'CodeGenFolder', workDir, 'CreateDir', true);

if exist('parameters.m', 'file')
    run('parameters.m');
end

if exist('inverter_library.slx', 'file')
    load_system('inverter_library');
else
    warning('inverter_library.slx not found on path, simulation may not run!')
end

disp('project initialized succesfully.')