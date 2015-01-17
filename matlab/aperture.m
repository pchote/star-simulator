% Perform aperture photometry on images in a directory.
% Demonstration code for Phys 217 photometry experiment
% Version 1.0 (2014-04-20) by Paul Chote
%
% Perform aperture reduction on a list of files that matching a given pattern.
% Arguments are:
%    pattern: file string to match, relative to the working dir.
%         bl: [bottom left] of the integration aperture
%         tr: [top right] of the integration aperture
% Two vectors are returned:
%       time: Time at the start of each integration, in seconds after start.
%      delta: Fractional intensity relative to the mean brightness
%
% Example: reduce all the fits files in the 'frames' subdirectory, using
% the square aperture x = 105:140 and y = 245:280:
%     [seconds, delta] = reduce('./frames/', [245 104], [280 140])

function [seconds, delta] = aperture(directory, bl, tr)

import matlab.io.*

% Find a list of files to reduce
olddirectory = cd(directory);
files = dir('*.fit');

% Initialize output data
count = length(files);

% Preallocate arrays to improve performance
time = zeros(1, count);
data = zeros(1, count);

for i=1:count;
    % Open a handle to the fits image.
    handle = fits.openFile(files(i).name);
    
    % Extract the observation time from the fits header, and then convert
    % it to the standard MATLAB time format.  MATLAB doesn't understand
    % the time format, so we must specify it explicitly.
    time(i) = datenum(fits.readKey(handle, 'DATE-OBS'), 'yyyy-mm-ddTHH:MM:SS');
    
    if (nargin >= 3)
        % Read the specified sub-array directly from the fits image
        aperture = fits.readImg(handle, bl, tr);
    else 
        % Read the whole image
        aperture = fits.readImg(handle); 
    end
    data(i) = sum(sum(aperture));
    
    % Clean up after ourselves
    fits.closeFile(handle);
end

% Return to the original directory
cd(olddirectory);

% Convert time to seconds after first exposure
seconds = 86400*(time - time(1));

% Convert intensity to percentage change from mean
delta = (data - mean(data)) ./ mean(data);
