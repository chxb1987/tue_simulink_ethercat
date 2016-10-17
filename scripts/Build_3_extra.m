%% General blocks
cd ../src
cd Extra_functions
cd General

disp('> General Blockset...')
% mex sf_setcpu.c
mex print_signal.c
disp(sprintf('\binstalled'))

cd ..
cd ..
%% Set PATH variable
 % Add pathnames that contain functions we might need now
% or later the S-functions
warning off
 addpath([pwd ],...
 [pwd '/Extra_functions'],...
 [pwd '/Extra_functions/General']);
savepath;
% add toolboxes to path
warning on