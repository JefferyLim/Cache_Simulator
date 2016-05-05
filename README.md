Code to simulate a Cache with an L1 and L2 cache with an 8 way fully associative victim cache. These simulations were run on 9 different cache configurations on 6 different traces.

To run this code requires gzip. Trace filed must be in .gz format.

Original execution line was:
zcat (trace file)|./main (config file)

This one is:
zcat (tracefile)|./main (trace file NAME) (config file)

This was changed in order to make it easier to automate the report outputs by using fprint.
