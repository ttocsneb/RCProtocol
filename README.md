# My project's README

## About

RC Protocol allows a remote to pair with more than 1 rc device as well as the abillity to connect add-ons: a device that gives the remote more functionality, such as a gimball controller, or a telemetry device.

The Protocol is a library. It does not connect directly to any IO devices other than the tranceiver (RF24L01(+)), and add-ons through i²c.


## Functionality

Functionality is some psuedo code to show what the protocol will do for specif tasks

**Bold** = Remote
*Italics* = Device

### Pair

**Open Writing Pipe to "Pair"**
*Open Reading Pipe to "Pair"*
**Send **ID** until acknowledge received**
*When Receive Packet, Save* ID
*Open Writing Pipe to* ID
**Open Reading Pipe to** ID
*Send* device_ID
*send add-on IDs, and whether they are required to run*
**Save all the data Received**


### Connect

**Open Reading Pipe to** ID
*Open Writing Pipe to* ID
*Send* device_ID *until acknowledge received*
**When data received, check if the add-ons required for** device_ID **are present**
    **yes: Send** True
    **GOTO** Connected
    **no: Send** False
    **Error**
*Receive Data, check if True, or False*
    *True: GOTO* Connected
    *False: ERROR*

### Connected

**Open Writing Pipe to** ID
*Open Reading Pipe to* ID
Loop:
**Send Data**
*Set AutoAck to Return Data*
*Get Data*
**Get Acknowledge data**
GOTO Loop

