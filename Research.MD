The TSC on our FX-8350 machine
https://www.amd.com/system/files/TechDocs/48751_16h_bkdg.pdf
(page 495) 

-- We have:
1) invariant TSC
    a) constant_tsc
    b) nonstop_tsc
    c) rdtscp

The AMD FX-8350 features a base clock of 4.0GHz which is accomplished using a 200MHz Bclk and a 20x multiplier.

Our TSC is based off of the P0 frequency
Stock AMD logics: P0 4.2 GHz>P1 4.1 GHz>P2 4.0 GHz

---------------

How we get accurate timing.
http://btorpey.github.io/blog/2014/02/18/clock-sources-in-linux/
