%% Path defines:
SOEMIDIR = '/usr/local/include/soem/';  % Folder where SOEM include files are located
LIBDIR = '/usr/local/lib/';             % Folder where libsoem.so library is located
SRCDIR = [pwd,'/ECAT_s_functions/'];    % Folder where s-function c-source files are located to build
SLBDIR = [pwd,'/ECAT_Simulink_Lib/'];   % Folder where EtherCAT simulink library is located

%% List s-functions to compile
C_files = {'ECAT_Interface.c';
            'ec_EL3102_adc.c';
            'ec_EL4132_dac.c';
            'ec_EL1008_di.c';
            'ec_EL2008_do.c';
            'ec_EL6022_ser.c';
            'ec_EL2502_pwm.c';
            'ec_EL3008_adc.c';
            'ec_EL4038_dac.c';
            'ec_EL5101_enc.c';
            'ec_EL5152_enc.c';
%            'ec_TU0001_combi.c';
           'ec_TU_ES_030.c';
           'ec_TU_ES_030FRF.c';
           'ec_TU_ES_030old.c'
           };

%% Build sfunctions
for i=1:length(C_files)
    mex([SRCDIR,C_files{i}],'-g',['-I',SOEMIDIR],['-L',LIBDIR],'-lsoem');
end

%% Move sfunctions to SRCDIR
movefile('*.mexa64*',SRCDIR)

%% Add path to mex-files and simulink blocks to MATLAB-PATH and save
addpath(SRCDIR);
addpath(SLBDIR);
savepath;

%% Add Library to Library browser
sl_refresh_customizations;
% Do a manual refresh now!
disp('In the simulink library browser, do a manual refresh to let the EtherCAT library show!')
