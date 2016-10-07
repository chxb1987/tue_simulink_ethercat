function [] = setBlockSize(obj, scale)

% setBlockSize - Set size of a Simulink object. Height is automatically sized
% based on the number of input/output ports (whichever is greater). Width is
% based on scale factor of unit dimension size of Constant block from library
% simulink/Sources.
%
% Example 1: setBlockSize(gcb) (sets height only, keeps same width)
% Example 2: setBlockSize(gcb, 4) (sets height and width)
%
% [] = setBlockSize(obj, scale)
% obj = block path name
% scale = block width scaling factor

% Get unit dimension size of Constant block.
load_system('simulink');
load_system('simulink/Sources');

tmp = get_param('simulink/Sources/Constant', 'Position');
unit_dim = tmp(4) - tmp(2);

close_system('simulink');

% Set block size.
pos = get_param(obj, 'Position');

% Ports vector:
% 1     Inport
% 2     Outport
% 3     Enable
% 4     Trigger
% 5     State
% 6     LConn
% 7     RConn
% 8     Ifaction
ports = get_param(obj, 'Ports');
num_inputs = ports(1) + ports(6);
num_outputs = ports(2) + ports(7);

if num_inputs > num_outputs % more inputs than outputs
  num_ports = num_inputs;
else % at least as many outputs as inputs
  num_ports = num_outputs;
end

if num_ports == 0 % empty subsystem
  num_ports = 1;
end

if nargin < 2 % no scale factor
  set_param(obj, 'Position', [pos(1) pos(2) pos(3) pos(2)+num_ports*unit_dim]);
else
  if scale < 0 % invalid
    error('Invalid scale factor. Set to > 0.');
  else % valid
    set_param(obj, 'Position', [pos(1) pos(2) pos(1)+scale*unit_dim pos(2)+num_ports*unit_dim]);
  end
end
