clear all; clc; 


% run this script immediately during matlab startup
% otherwise you will have matlab pollute the root directory

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

