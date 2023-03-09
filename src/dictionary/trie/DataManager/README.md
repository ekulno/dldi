# Data manager 

This folder contains code for dealing with the data layer. 

This code aims to abstract away the distinction between storage location (read from mmap vs loaded in memory). 

Things I don't like: 

 - `save` operation is destructive. Calling `save` makes the trie unqueriable until it's reloaded. 
 - extensive use of raw pointers
 - binary search is implemented at least three times. should only need to be implemented once, and it might be present in stdlib. 
 - out-edge-iterator should be put within datamanager. 