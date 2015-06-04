# arpmanetdc
Automatically exported from code.google.com/p/arpmanetdc

An awesome UDP-based peer-to-peer-plus-direct-connect application for distributed file distribution over a heterogeneous network environment.

This project requires the Qt4 toolkit >= 4.8, the Crypto++ libraries for TTH hashing, as well as the sqlite3 libraries for the database backend.

Progress
Project started: 9 Nov 2011

Current version: 0.1.8

Current stage: Alpha

26 Nov 2011 - The project has just passed the 10,000 lines of code mark!

September 2012

Some major restructuring around transfers is taking place, which in its currently bit-banged form can be described as barely functional.

The long term goal is to break the whole hierarchy of different types of segments down to just one (a segment :) that reads/writes through a connection, which is managed by the shiny new connection manager. Different protocols can be wrapped up inside connections, of which TCP is to be the first.

TCP can obviously not be run through our dispatch system, therefore we need to hang on to the FSTP transfers a little bit more as a failsafe alternative until we have uTP running. uTP will be our first dispatched connection based protocol, paving the way for our experimental work with FECTP transfers.
