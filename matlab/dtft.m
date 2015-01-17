
% Demonstration code for Phys 217 photometry experiment
% Version 1.0 (2014-04-20) by Paul Chote
%
% Calculate the discrete-time Fourier transform of specified data at
% specified frequencies.
% Arguments are:
%    time: Vector (assumed 1xN) of measurement times
%    data: Vector (assumed 1xN) of measurement intensities
%    freq: Vector (assumed 1xM) of frequencies to evalute
% Returns: a 1xM vector of complex amplitudes evaluated at freq.

function ampl = dtft(time, data, freq)
 
% TODO: Write this again in long form with explanatory comments
% This is essentially the same as the FFT, but without the decimation step
ampl = data * exp(-2i * pi * time' * freq);

