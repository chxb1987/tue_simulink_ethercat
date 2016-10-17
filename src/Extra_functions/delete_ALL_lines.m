function delete_ALL_lines( sys )
%DELETE_UNCONNECTED_LINES  Delete unconnected lines from a Simulink system.
%   DELETE_UNCONNECTED_LINES('SYS') deletes all lines which has either no
%   source or no destination under a system. That is, lines that are not
%   fully connected to two blocks are removed (red-dotted in the GUI).
%
%   See also DELETE_LINE.

%   Developed by Per-Anders Ekstr�m, 2003-2006 Facilia AB.

if nargin<1
    sys = gcs;
end

% get handles to all lines in system
lines = find_system( sys, ...
    'LookUnderMasks','on',...
    'FollowLinks','on',...
    'FindAll', 'on', ...
    'Type', 'line' ) ;

% for each line, call delete_recursive if handle still exist
for i=1:length( lines )
    if ishandle( lines( i ) )
        delete_all( lines( i ) )
    end
end
end


function delete_all( line )
%DELETE_RECURSIVE( LINE )  Delete line if:
%   1) do not have any source-block
%   2) do not have any line-children AND no destination-block
%   otherwise go recursively through all eventual line-children

delete_line( line ) ;


end