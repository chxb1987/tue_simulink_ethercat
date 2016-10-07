function changesuc = init_TUeES030(inputports,outputports,modus)

%% init_TUeES030.m
%
%   File    : init_TUeES030.m
%   Version : 1.0
%   Date    : 11-01-2016
%   By      : Alex Andrien
%
%   This function initialises the Simulink blocks used to control the
%   TUeES030 slave, depending on the input and output ports defined as cell
%   arrays.

%% Settings
numIns = size(inputports,2);
numOuts = size(outputports,2);

%Define standard block sizes and offsets
x = 30;
y = 30;
w = 30;
h = 30;
offsetyIns = 60;
offsetyOuts = 30;
offsetx = 60;

%% Check if mode change is possible
% if any input/output ports are connected, they cannot be deleted.
portData=get_param(gcb,'PortConnectivity');
currentIns = find_system(gcb,'LookUnderMasks','on',...
    'FollowLinks','on','BlockType','Inport');
numcurIns = size(currentIns,1);

currentOuts = find_system(gcb,'LookUnderMasks','on',...
    'FollowLinks','on','BlockType','Outport');
numcurOuts = size(currentOuts,1);

if ((numIns == numcurIns) && (numOuts == numcurOuts))
    %Input/Outputs unchanged, same mode active, do nothing
    changesuc = 1;
    return
else
    display('Mode change requested detected')
    for ii = 1:numcurIns
        if(portData(ii).SrcBlock ~= -1)
            fprintf('Input port %d is connected, cannot change mode \n',ii)
            changesuc = 0;
            return
        end
    end
    for jj = 1:numcurOuts
        if(~isempty(portData(end-jj).DstBlock))
            fprintf('Output port %d is connected, cannot change mode \n',jj)
            changesuc = 0;
            return
        end
    end
    
end
display('Trying to change mode...')

%% Find S-function to keep it from removing
%Find S-function
TUsfunc = find_system(gcb,'LookUnderMasks','on',...
    'FollowLinks','on','Name','TUeES030sfunc');

%Find all other blocks
allblocks = find_system(gcb,'LookUnderMasks','on',...
    'FollowLinks','on');

%Set all other blocks to be deleted
ToDelete = setdiff(allblocks,TUsfunc);
ToDelete = setxor(ToDelete,gcb);

%Delete blocks in loop, catch if some cannot be deleted
for ii = 1:numel(ToDelete)
    try
        delete_block(ToDelete{ii})
    catch
        disp('Some objects couldn''t be deleted')
    end
end

%Delete all lines
delete_ALL_lines(gcb);


%% Create inputs/outputs, depending on mode
%Align S-function
set_param([gcb '/TUeES030sfunc'],'Position',...
    [(x+2*offsetx) (y+(numOuts-1)/2*offsetyOuts)-h (x+2*offsetx)+w*3 (y+(numOuts-1)/2*offsetyOuts)+h*2]);

%Add mux block inputs
pos = [(x+offsetx) y+1.25*offsetyIns-h/4 (x+offsetx)+w/4 y+1.25*offsetyIns+offsetyIns*(numIns-1)+h*1.25];
add_block('built-in/Mux',[gcb '/mux1'],'Position',pos);
set_param([gcb '/mux1'],'Inputs',num2str(numIns),'DisplayOption','bar');

%Connect mux to S-function
add_line(gcb,'mux1/1','TUeES030sfunc/1','autorouting','on')

%Create input ports in loop
for ii = 1:numIns
    pos = [x (y+1.25*offsetyIns+offsetyIns*(ii-1))+h/4 x+w (y+1.25*offsetyIns+offsetyIns*(ii-1))+h*0.75];
    add_block('built-in/Inport',[gcb '/' inputports{ii}],'Position',pos);
    %Connect input ports to mux
    add_line(gcb,[inputports{ii} '/1'],['mux1/' num2str(ii)],'autorouting','on')
end

%Add demux block outputs
pos = [(x+4*offsetx+w) y-h/4 (x+4*offsetx+w)+w/4 y+offsetyOuts*(numOuts-1)+h*1.25];
add_block('built-in/Demux',[gcb '/Demux1'],'Position',pos);
if(strcmp(modus,'Normal'))
    set_param([gcb '/Demux1'],'Outputs',num2str(numOuts),'DisplayOption','bar');
elseif((strcmp(modus,'FRF Measurement')))
    display('test')
    %set_param([gcb '/Demux1'],'Outputs',num2str(numOuts),'DisplayOption','bar');
    set_param([gcb '/Demux1'],'Outputs',mat2str([1,1,1,1,22]),...
        'DisplayOption','bar','BusSelectionMode','Off');
else
    return;
end

%Connect S-function to demux
add_line(gcb,'TUeES030sfunc/1','Demux1/1','autorouting','on')

%Create output ports in loop
for ii = 1:numOuts
    if(mod(ii,2)) %odd number
        pos = [(x+5*offsetx+w) (y+offsetyOuts*(ii-1))+h/4 (x+5*offsetx+w)+w (y+offsetyOuts*(ii-1))+h*0.75];
    elseif(~mod(ii,2)) %even number
        pos = [(x+6*offsetx+w) (y+offsetyOuts*(ii-1))+h/4 (x+6*offsetx+w)+w (y+offsetyOuts*(ii-1))+h*0.75];
    end
    add_block('built-in/Outport',[gcb '/' outputports{ii}],...
        'Position',pos);
    %Connect outpot ports to demux
    add_line(gcb,['Demux1/' num2str(ii)],[outputports{ii} '/1'],...
        'autorouting','on')
end
display('Mode change succesfull')
changesuc = 1;
end