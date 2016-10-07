%Find S-function
TUsfunc = find_system(gcb,'LookUnderMasks','on',...
    'FollowLinks','on','Name','TUeES030sfunc');

switch modeC
    case 'Normal'
        %Define input ports
        inputports = {'mcom1','setpoint1','ff1',...
            'mcom2','setpoint2','ff2',...
            'mcom3','setpoint3','ff3',...
            'digital_o','aout1','aout2'};
        
        %Define output ports
        outputports = {'mstate1','count1','timestamp1','velocity1','current1',...
            'mstate2','count2','timestamp2','velocity2','current2',...
            'mstate3','count3','timestamp3','velocity3','current3',...
            'digital_i','caliper1','caliper2',...
            'force1','force2','force3','pos1','pos2','pos3',...
            'analog1','analog2','linevoltage','ectime'};
        set_param([gcb '/TUeES030sfunc'],'FunctionName','ec_TU_ES_030v2');
    case 'FRF Measurement'
        %Define input ports
        inputports = {'mcom','entries_i','setpoint'};
        
        %Define output ports
        outputports = {'mstate','buffer','entries_o',...
            'ectime','current'};
        set_param([gcb '/TUeES030sfunc'],'FunctionName','ec_TU_ES_030v2FRF');
end

initsuc = init_TUeES030(inputports,outputports,modeC);

maskvals = get_param(gcb,'MaskValues');
if(initsuc == 0 && strcmp(modeC,'FRF Measurement'))
    maskvals{end} = 'Normal';
    set_param(gcb,'MaskValues',maskvals)
    display('Mode changed back to normal')
elseif(initsuc == 0 && strcmp(modeC,'Normal'))
    maskvals{end} = 'FRF Measurement';
    set_param(gcb,'MaskValues',maskvals)
    display('Mode changed back to FRF Measurement')
end

setBlockSize(gcb)
