disp('%%%%%%%%%%%%% Installing SOEM and Simulink Blockset %%%%%%%%%%%%%%%%')
%% SOEM
cd ../src/soem

disp('> building libsoem...')
!chmod a+x build_lib
!./build_lib
disp(sprintf('\binstalled'))

cd ../../scripts
