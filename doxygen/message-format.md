Host <-> Device Communication
========================================

[TOC]

# Overview {#message-overview}

The Host and Device communicate by exchanging messages. Each message
represents an operation or command. All communication is
initiated by the Host and the Device sends a single message in reply to
each command.

Messages sent from the Host to the Device are *Requests*, messages sent from
the Device to the Host are *Responses*.

# Byte Ordering {#message-endian}

All multi-byte fields are sent little endian (LSB first) unless otherwise
specified.

# Message Format {#message-format}

## Request {#message-format-request}

Each request message begins with a common header, zero or more byte of
payload data, an end-of-message marker and then zero or more bytes of
padding. The total message size must not exceed @ref USB_READ_BUFFER_SIZE.

<pre>
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |      SOM      |     Token     |            Command            |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |            Length             |       Payload (if any)        \
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 /                                                               /
 \                        Payload (if any)                       \
 /                                                               /
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |      EOM      |               Padding (optional)              \
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param SOM The start of message identifier: @ref START_OF_MESSAGE_ID
@param Token A token for the request. The same token will be returned in
the response. Typically the host will increment the token with each
request.
@param Command The @ref Command identifier.
@param Length The length of the data included in the request. The valid
range is 0 - 579 bytes.
@param Payload The payload data associated with the request. See each
command type below for the specific format of the payload data.
@param EOM The end of message identifier: @ref END_OF_MESSAGE_ID
@param Padding Extra padding. If present this should be filled with 0s.
Padding can be added as long as the total message does not exceed
@ref USB_READ_BUFFER_SIZE.

## Response {#message-format-reply}

Responses use the same header as Requests, with an additional 2 bytes to
indicate the return code and status flags.

<pre>
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |      SOM      |     Token     |            Command            |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |            Length             |  Return_Code   |     Status   |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 /                                                               /
 \                        Payload (if any)                       \
 /                                                               /
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |      EOM      |               Padding (optional)              \
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param SOM The start of message identifier: @ref START_OF_MESSAGE_ID
@param Token The token that was provided in the corresponding request.
@param Command The @ref Command identifier.
@param Return_Code The @ref ReturnCode of the response.
@param Status The status bitfield.
@param Length The length of the data included in the command. The valid
range is 0 - 579 bytes.
@param Payload The payload data associated with the command. See each
command type below for the specific format of the payload data.
@param EOM The end of message identifier: @ref END_OF_MESSAGE_ID
@param Padding Extra padding. If present this should be filled with 0s.
Padding can be added as long as the total message does not exceed
@ref USB_READ_BUFFER_SIZE.

# Commands {#message-commands}

## Echo {#message-commands-echo}

Echo data back from the device. This serves as a basic test that Host to
Device communications are working correctly.

### Request Payload {#message-commands-echo-req}

<pre>
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 \                     Data (variable size)                      \
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Data 0 or more bytes of data.

### Response Payload {#message-commands-echo-res}

<pre>
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 \                     Data (variable size)                      \
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Data 0 or more bytes of data, to match what was provided in the
request.
@returns @ref RC_OK.

## Set Mode  {#message-commands-setmode}

Set the operating mode of the device. The device can operate as either a
controller or a responder.

### Request Payload {#message-commands-setmode-req}

<pre>
  0
  0 1 2 3 4 5 6 7 8
 +-+-+-+-+-+-+-+-+-+
 |   Mode          |
 +-+-+-+-+-+-+-+-+-+
</pre>

@param Mode The new mode to operate in. 0 for controller, 1 for responder.

### Response Payload {#message-commands-setmode-res}

The response contains no data.

@returns @ref RC_OK.

## Get Hardware Information  {#message-commands-gethardware}

Get the hardware infomation for the device.

### Request Payload {#message-commands-gethardware-req}

The request contains no data.

### Response Payload {#message-commands-gethardware-res}

<pre>
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |          Model ID               |             UID             |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                             UID (cont.)                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                            MAC Address                        |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |       MAC Address (cont.)       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Model_Id The \ref JaRuleModel of the device.
@param UID The UID of the device, in network byte order.
@param MAC The MAC address of the device, in network byte order. If the device
does not have a MAC this will be 0.

@returns @ref RC_OK.

## Run Self Test {#message-commands-selftest}

Run the loopback self test. The device must be in self test mode to perform
this.

### Request Payload {#message-commands-selftest-req}

The request contains no data.

### Response Payload {#message-commands-selftest-res}

The response contains no data.

@returns @ref RC_OK or @ref RC_TEST_FAILED.

## Reset  {#message-commands-reset}

Resets the device. This can be used to recover from failures.

### Request Payload {#message-commands-reset-req}

The request contains no data.

### Response Payload {#message-commands-reset-res}

The response contains no data.

@returns @ref RC_OK.

## Get Break Time  {#message-commands-getbreaktime}

Gets the current break time for outgoing DMX512 / RDM messages.

### Request Payload {#message-commands-getbreaktime-req}

The request contains no data.

### Response Payload {#message-commands-getbreaktime-res}

<pre>
  0                   1
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |          Break_Time           |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Break_Time The current break time in microseconds.
@returns @ref RC_OK.

## Set Break Time  {#message-commands-setbreaktime}

Sets the break time for outgoing DMX512 / RDM messages.

### Request Payload {#message-commands-setbreaktime-req}

<pre>
  0                   1
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |           Break_Time          |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Break_Time The new break time in microseconds. See
Transceiver_SetBreakTime() for the range of values allowed.

### Response Payload {#message-commands-setbreaktime-res}

The response contains no data.

@returns @ref RC_OK or @ref RC_BAD_PARAM if the value was out of range.

## Get Mark Time {#message-commands-getmarktime}

Gets the current mark-after-break time for outgoing DMX512 / RDM messages.

### Request Payload {#message-commands-getmarktime-req}

The request contains no data.

### Response Payload {#message-commands-getmarktime-res}

<pre>
  0                   1
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |         Mark_Time             |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Mark_Time The current mark time in microseconds.
@returns @ref RC_OK.

## Set Mark Time {#message-commands-setmarktime}

Sets the mark time for outgoing DMX512 / RDM messages.

### Request Payload {#message-commands-setmarktime-req}

<pre>
  0                   1
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |         Mark_Time             |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Mark_Time The new mark time in microseconds. See
Transceiver_SetMarkTime() for the range of values allowed.

### Response Payload {#message-commands-setmarktime-res}

The response contains no data.

@returns @ref RC_OK or @ref RC_BAD_PARAM if the value was out of range.

## Get RDM Broadcast Timeout {#message-commands-getbcasttimeout}

Get the time the controller will wait for an RDM Response after sending a
broadcast command.

### Request Payload {#message-commands-getbcasttimeout-req}

The request contains no data.

### Response Payload {#message-commands-getbcasttimeout-res}

<pre>
  0                   1
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |             Timeout           |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Timeout The current RDM broadcast timeout, in 10ths of a
millisecond.
@returns @ref RC_OK.

## Set RDM Broadcast Timeout {#message-commands-setbcasttimeout}

Sets the time to wait for an RDM response after sending a broadcast RDM
command. When set to a non-0 value, this allows us to detect responders that
incorrectly respond to broadcast commands.

### Request Payload {#message-commands-setbcasttimeout-req}

<pre>
  0                   1
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |             Timeout           |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Timeout The time to wait for a RDM response after a broadcast command,
in 10ths of a millisecond. See Transceiver_SetRDMBroadcastTimeout().

### Response Payload {#message-commands-setbcasttimeout-res}

The response contains no data.

@returns @ref RC_OK or @ref RC_BAD_PARAM if the value was out of range.

## Get RDM Response Timeout {#message-commands-getresponsetimeout}

Get the time that the controller will wait for an RDM Response after sending
an RDM command.

### Request Payload {#message-commands-getresponsetimeout-req}

The request contains no data.

### Response Payload {#message-commands-getresponsetimeout-res}

<pre>
  0                   1
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |             Timeout           |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Timeout The current RDM response timeout, in 10ths of a
millisecond.
@returns @ref RC_OK.

## Set RDM Response Timeout {#message-commands-setresponsetimeout}

Sets the time to wait for a reply after a sending an RDM command.

### Request Payload {#message-commands-setresponsetimeout-req}

<pre>
  0                   1
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |             Timeout           |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Timeout The time to wait for a RDM response after a broadcast command,
in 10ths of a millisecond. See Transceiver_SetRDMResponseTimeout().

### Response Payload {#message-commands-setresponsetimeout-res}

The response contains no data.

@returns @ref RC_OK or @ref RC_BAD_PARAM if the value was out of range.

## Get RDM DUB Response Limit {#message-commands-getdublimit}

Get the maximum time a DUB response can take.

### Request Payload {#message-commands-getdublimit-req}

The request contains no data.

### Response Payload {#message-commands-getdublimit-res}

<pre>
  0                   1
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |              Limit            |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Limit The maximum time a DUB response can take, in 10ths of a
microsecond.
@returns @ref RC_OK.

## Set RDM DUB Response Limit {#message-commands-setdublimit}

Sets the time to wait for a reply after a sending an RDM command.

### Request Payload {#message-commands-setdublimit-req}

<pre>
  0                   1
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |              Limit            |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Limit The maximum time a DUB response can take, in 10ths of a
millisecond. See Transceiver_SetRDMDUBResponseLimit().

### Response Payload {#message-commands-setdublimit-res}

The response contains no data.

@returns @ref RC_OK or @ref RC_BAD_PARAM if the value was out of range.

## Get RDM Responder Delay {#message-commands-getresponderdelay}

Get the minimum time the responder waits before sending an RDM response.

### Request Payload {#message-commands-getresponderdelay-req}

The request contains no data.

### Response Payload {#message-commands-getresponderdelay-res}

<pre>
  0                   1
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |              Delay            |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Delay The current RDM responder delay, in 10ths of a microsecond.
@returns @ref RC_OK.

## Set RDM Responder Delay {#message-commands-setresponderdelay}

Sets the minimum time the responder waits before sending an RDM response.

### Request Payload {#message-commands-setresponderdelay-req}

<pre>
  0                   1
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |              Delay            |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Delay The minimum time to wait before sending an RDM response,
in 10ths of a microsecond. See Transceiver_SetRDMResponderDelay().

### Response Payload {#message-commands-setresponderdelay-res}

The response contains no data.

@returns @ref RC_OK or @ref RC_BAD_PARAM if the value was out of range.

## Get RDM Responder Jitter {#message-commands-getresponderjitter}

Get the maximum jitter to use when sending RDM responses.

### Request Payload {#message-commands-getresponderjitter-req}

The request contains no data.

### Response Payload {#message-commands-getresponderjitter-res}

<pre>
  0                   1
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |              Jitter           |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Jitter The current RDM responder jitter, in 10ths of a microsecond.
@returns @ref RC_OK.

## Set RDM Responder Jitter {#message-commands-setresponderjitter}

Set the jitter to use when sending RDM responses.

### Request Payload {#message-commands-setresponderjitter-req}

<pre>
  0                   1
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |              Jitter           |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Delay The jitter to use when sending an RDM response,
in 10ths of a microsecond. See Transceiver_SetRDMResponderJitter().

### Response Payload {#message-commands-setresponderjitter-res}

The response contains no data.

@returns @ref RC_OK or @ref RC_BAD_PARAM if the value was out of range.

## Transmit DMX512 {#message-commands-txdmx}

Sends a single DMX512, Null Start Code frame.

### Request Payload {#message-commands-txdmx-req}

<pre>
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 \                   DMX_Data (variable size)                    \
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param DMX_Data The DMX512 slot data, excluding the start code. The number
of slots may be 0 - 512.

### Response Payload {#message-commands-txdmx-res}

The response contains no data.

@returns
- @ref RC_OK if the frame was sent correctly.
- @ref RC_BUFFER_FULL if the transmit buffer is full.
- @ref RC_TX_ERROR if a transmit error occurred.

## Transmit RDM DUB {#message-commands-txrdmdub}

Sends a RDM discovery unique branch command and then listens for a response.
If any data is received, it is returned in the response payload. If no data
is received a @ref RC_RDM_TIMEOUT is returned.

The response payload includes timing data.

### Request Payload {#message-commands-txrdmdub-req}

<pre>
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 \                          DUB_Command                          \
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param DUB_Command The RDM Discovery Unique Branch command, excluding the
start code. The command should be 37 bytes.

### Response Payload {#message-commands-txrdmdub-res}

<pre>
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |        Discovery_Start        |        Discovery_End          |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 \               RDM_DUB_Response (variable size)                \
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Discovery_Start the time from the end of the transmitted DUB frame to
the start of the DUB response, in 10ths of a microsecond. Undefined unless
RC_OK was returned.
@param Discovery_End the time from the end of the transmitted DUB frame to
the end of the DUB response, in 10ths of a microsecond. Undefined unless
RC_OK was returned.
@param RDM_DUB_Response The raw response, if any was received.
@returns
- @ref RC_OK if the frame was sent correctly and data was received.
- @ref RC_BUFFER_FULL if the transmit buffer is full.
- @ref RC_TX_ERROR if a transmit error occurred.
- @ref RC_RDM_TIMEOUT if no response was received.

## Transmit Broadcast RDM Get / Set {#message-commands-txrdmbroadcast}

Sends a broadcast RDM Get / Set command. If the Broadcast Listen Delay is not 0,
the transceiver will then listen for a response. If a response is received,
it will be returned in the response payload.

### Request Payload {#message-commands-txrdmbroadcast-req}

<pre>
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 \                        RDM_Command                            \
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param RDM_Command The broadcast RDM Get / Set command, excluding the start
code.

### Response Payload {#message-commands-txrdmbroadcast-res}

<pre>
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |          Break_Start           |           Break_End          |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |           Mark_End             | RDM_Response (variable size) \
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 \                 RDM_Response (variable size)                  \
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Break_Start the time from the end of the transmitted RDM frame to
the start of the break, in 10ths of a microsecond. Undefined unless
RC_RDM_BCAST_RESPONSE was returned.
@param Break_End the time from the end of the transmitted RDM frame to
the end of the break / start of the mark, in 10ths of a microsecond.
Undefined unless RC_RDM_BCAST_RESPONSE was returned.
@param Mark_End the time from the end of the transmitted RDM frame to
the start of the mark, in 10ths of a microsecond. Undefined unless
RC_RDM_BCAST_RESPONSE was returned.
@param RDM_Response The RDM response, if any was received.
@returns
- @ref RC_OK if the frame was broadcast correctly and the broadcast listen
  delay was 0 or the delay was non-0 and no data was received.
- @ref RC_BUFFER_FULL if the transmit buffer is full.
- @ref RC_TX_ERROR if a transmit error occurred.
- @ref RC_RDM_BCAST_RESPONSE if a response was received.

## Transmit RDM Get / Set {#message-commands-txrdm}

Send a RDM Get / Set command and listen for a response. If a response is
received, it will be returned in the response payload.

### Request Payload {#message-commands-txrdm-req}

<pre>
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 \                        RDM_Command                            \
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param RDM_Command The RDM Get / Set command, excluding the start
code.

### Response Payload {#message-commands-txrdm-res}

<pre>
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |          Break_Start           |           Break_End          |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |           Mark_End             | RDM_Response (variable size) \
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 \                 RDM_Response (variable size)                  \
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

@param Break_Start the time from the end of the transmitted RDM frame to
the start of the break, in 10ths of a microsecond. Undefined unless RC_OK
was returned.
@param Break_End the time from the end of the transmitted RDM frame to
the end of the break / start of the mark, in 10ths of a microsecond.
Undefined unless RC_OK was returned.
@param Mark_End the time from the end of the transmitted RDM frame to
the start of the mark, in 10ths of a microsecond. Undefined unless RC_OK was
returned.
@param RDM_Response The RDM response, if any was received.
@returns
- @ref RC_OK if the frame was sent correctly and a response was received.
- @ref RC_BUFFER_FULL if the transmit buffer is full.
- @ref RC_TX_ERROR if a transmit error occurred.
- @ref RC_RDM_TIMEOUT if no response was received.

## Unrecognised Commands {#message-cmd-unknown}

If the device receives a command ID that is doesn't recognize it will return
@ref RC_UNKNOWN.

# Transport Considerations {#message-transport}
## USB {#message-transport-usb}

Communication with the Ja Rule device happens over a custom endpoint.

The maximum packet size (wMaxPacketSize) for the USB device is 64 bytes.
This is the largest packet size for a full speed, bulk endpoint. The RX
buffer must be a multiple of the maxPacketSize. Since the largest payload
data size is 513 bytes, we'll set the RX buffer size to 640 (64 * 10).

On the Microchip RX side, the USB_DEVICE_EVENT_ENDPOINT_READ_COMPLETE event
will be triggered if either:
- the host sends less than maxPacketSize data in any transaction
- the host sends maxPacketSize data, and the total size matches the rx
  buffer size.

libusb on Linux supports a Zero Length Packet (ZLP) flag, which will
automatically send a zero length packet if a transfer ends on a
wMaxPacketSize boundary. Other host OS's don't seem to support this, so the
host side will need to manually pad the message to trigger the
USB_DEVICE_EVENT_ENDPOINT_READ_COMPLETE event.
