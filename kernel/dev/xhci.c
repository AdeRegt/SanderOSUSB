#include "../kernel.h"
#define XHCI_DEVICE_BOCHS 0x15
#define XHCI_SPEED_FULL   1
#define XHCI_SPEED_LOW    2
#define XHCI_SPEED_HI     3
#define XHCI_SPEED_SUPER  4

#define XHCI_TRB_SET_CYCLE_BIT(x)      				(((x) & 0x0F) << 00)
#define XHCI_TRB_SET_TRB_TYPE(x)      				(((x) & 0x3F) << 10)
#define XHCI_TRB_SET_SLOT(x)      				(((x) & 0xFF) << 24)

#define XHCI_TRB_ENABLE_SLOT_SLOTTYPE(x)      			(((x) & 0x1F) << 16)

#define XHCI_TRB_SET_ADDRESS_POINTER(x)      			(((x) & 0xFFFFFFF0))
#define XHCI_TRB_SET_ADDRESS_BSR(x)      			(((x) & 0x1) << 9)

#define XHCI_INPUT_CONTROL_CONTEXT_CONFIGURATION_VALUE(x) 	(((x) & 0xFF))
#define XHCI_INPUT_CONTROL_CONTEXT_INTERFACE_NUMBER(x)		(((x) & 0xFF) << 8)
#define XHCI_INPUT_CONTROL_CONTEXT_ALTERNATE_SETTING(x)		(((x) & 0xFF) << 16)

#define XHCI_SLOT_CONTEXT_ROUTE_STRING(x)			(((x) & 0xFFFFF))
#define XHCI_SLOT_CONTEXT_SPEED(x)				(((x) & 0xF) << 20)
#define XHCI_SLOT_CONTEXT_MTT(x)				(((x) & 0x1) << 25)
#define XHCI_SLOT_CONTEXT_HUB(x)				(((x) & 0x1) << 26)
#define XHCI_SLOT_CONTEXT_CONTEXT_ENTRIES(x)			(((x) & 0x1F) << 27)
#define XHCI_SLOT_CONTEXT_MAX_EXIT_LATENCY(x)			(((x) & 0xFFFF))
#define XHCI_SLOT_CONTEXT_ROOT_HUB_PORT_NUMBER(x)		(((x) & 0xFF) << 16)
#define XHCI_SLOT_CONTEXT_NUM_OF_PORTS(x)			(((x) & 0xFF) << 24)
#define XHCI_SLOT_CONTEXT_TT_HUB_SLOT_ID(x)			(((x) & 0xFF) )
#define XHCI_SLOT_CONTEXT_TT_PORT_NUMBER(x)			(((x) & 0xFF) << 8)
#define XHCI_SLOT_CONTEXT_TTT(x)				(((x) & 0x3) << 16)
#define XHCI_SLOT_CONTEXT_INTERRUPTER_TARGET(x)			(((x) & 0x3FF) << 22)
#define XHCI_SLOT_CONTEXT_USB_DEVICE_ADDRESS(x)			(((x) & 0xFF) )
#define XHCI_SLOT_CONTEXT_SLOT_STATE(x)				(((x) & 0x1F) << 27)

#define XHCI_ENDPOINT_CONTEXT_ENDPOINT_STATE(x)			(((x) & 0b11) << 0)
#define XHCI_ENDPOINT_CONTEXT_MULT(x)				(((x) & 0b11) << 8)
#define XHCI_ENDPOINT_CONTEXT_MAX_PRIMAIRY_STREAMS(x)		(((x) & 0b11111) << 10)
#define XHCI_ENDPOINT_CONTEXT_LINEAR_STREAM_ARRAY(x)		(((x) & 0x1) << 15)
#define XHCI_ENDPOINT_CONTEXT_INTERVAL(x)			(((x) & 0b11111111) << 16)
#define XHCI_ENDPOINT_CONTEXT_MAX_ENDPOINT_SERVICE(x)		(((x) & 0xFF) << 24)
#define XHCI_ENDPOINT_CONTEXT_ERR_CNT(x)			(((x) & 0b11) << 1)
#define XHCI_ENDPOINT_CONTEXT_ENDPOINT_TYPE(x)			(((x) & 0b111) << 3)
#define XHCI_ENDPOINT_CONTEXT_HOST_INITIATE_DISABLE(x)		(((x) & 0x1) << 7)
#define XHCI_ENDPOINT_CONTEXT_MAX_BURST_SIZE(x)			(((x) & 0b11111111) << 8)
#define XHCI_ENDPOINT_CONTEXT_MAX_PACKET_SIZE(x)		(((x) & 0b11111111111111111) << 16)
#define XHCI_ENDPOINT_CONTEXT_DSC(x)				(((x) & 0x1) << 0)
#define XHCI_ENDPOINT_CONTEXT_DP(x)				(((x) & 0b1111111111111111111111111111) << 4)
#define XHCI_ENDPOINT_CONTEXT_AVG_TRB_LENGTH(x)			(((x) & 0b1111111111111111) << 0)
#define XHCI_ENDPOINT_CONTEXT_MAX_ENDPOINT_SERVICE_LO(x)	(((x) & 0b1111111111111111) << 16)
	
typedef struct{
	// Cycle bit (C). This bit is used to mark the Enqueue Pointer of a Command Ring.
	unsigned char cyclebit;
	// TRB Type. This field identifies the type of the TRB.
	unsigned char trbtype; 
	// Slot Type. This field identifies the type of Slot that will be enabled by this command.
	unsigned char slottype;
}XHCI_TRB_ENABLE_SLOT;

typedef struct{

	// Input Context Pointer Hi and Lo. This field represents the high order bits of the 64-bit base
	// address of the Input Context data structure associated with this command. Refer to section 6.2.5
	// for more information on the Input Context data structure.
	// The memory structure referenced by this physical memory pointer shall be aligned on a 16-byte
	// address boundary.
	unsigned long long input_context;
	
	// Cycle bit (C). This bit is used to mark the Enqueue Pointer of a Command Ring.
	unsigned char cyclebit;
	
	// Block Set Address Request (BSR). When this flag is set to ‘0’ the Address Device Command shall
	// generate a USB SET_ADDRESS request to the device. When this flag is set to ‘1’ the Address
	// Device Command shall not generate a USB SET_ADDRESS request. Refer to section 4.6.5 for
	// more information on the use of this flag.
	unsigned char bsr;
	
	// TRB Type. This field identifies the type of the TRB. Refer to Table 6-86 for the definition of the
	// Address Device Command TRB type ID.
	unsigned char trbtype;
	
	// Slot ID. The ID of the Device Slot that is the target of this command.
	unsigned char slotid;
	
}XHCI_TRB_SET_ADDRESS;

typedef struct{
	
	// Drop Context flags (D2 - D31). These single bit fields identify which Device Context data
	// structures should be disabled by command. If set to ‘1’, the respective Endpoint Context shall be
	// disabled. If cleared to ‘0’, the Endpoint Context is ignored.
	unsigned long D;
	
	// Add Context flags (A0 - A31). These single bit fields identify which Device Context data
	// structures shall be evaluated and/or enabled by a command. If set to ‘1’, the respective Context
	// shall be evaluated. If cleared to ‘0’, the Context is ignored.
	unsigned long A;
	
	// Configuration Value. If CIC = ‘1’, CIE = ‘1’, and this Input Context is associated with a Configure
	// Endpoint Command, then this field contains the value of the Standard Configuration Descriptor
	// bConfigurationValue field associated with the command, otherwise the this field shall be
	// cleared to ‘0’.
	unsigned char configuration_value;
	
	// Interface Number. If CIC = ‘1’, CIE = ‘1’, this Input Context is associated with a Configure
	// Endpoint Command, and the command was issued due to a SET_INTERFACE request, then this
	// field contains the value of the Standard Interface Descriptor bInterfaceNumber field associated
	// with the command, otherwise the this field shall be cleared to ‘0’.
	unsigned char interface_number;
	
	// Alternate Setting. If CIC = ‘1’, CIE = ‘1’, this Input Context is associated with a Configure
	// Endpoint Command, and the command was issued due to a SET_INTERFACE request, then this
	// field contains the value of the Standard Interface Descriptor bAlternateSetting field associated
	// with the command, otherwise the this field shall be cleared to ‘0’.
	unsigned char alternate_setting;
	
}XHCI_INPUT_CONTROL_CONTEXT;

typedef struct{
	
	// Route String. This field is used by hubs to route packets to the correct downstream port. The
	// format of the Route String is defined in section 8.9 the USB3 specification.
	// As Input, this field shall be set for all USB devices, irrespective of their speed, to indicate their
	// location in the USB topology 105 .
	unsigned long route_string;
	
	// Speed. This field indicates the speed of the device. Refer to the PORTSC Port Speed field in
	// Table 5-26 for the definition of the valid values.
	unsigned char speed;
	
	// Multi-TT (MTT) 106 . This flag is set to '1' by software if this is a High-speed hub (Speed = ‘3’ and
	// Hub = ‘1’) that supports Multiple TTs and the Multiple TT Interface has been enabled by
	// software, or if this is a Low-/Full-speed device (Speed = ‘1’ or ‘2’, and Hub = ‘0’) and connected
	// to the xHC through a parent 107 High-speed hub that supports Multiple TTs and the Multiple TT
	// Interface of the parent hub has been enabled by software, or ‘0’ if not.
	unsigned char mtt;
	
	// Hub. This flag is set to '1' by software if this device is a USB hub, or '0' if it is a USB function.
	unsigned char hub;
	
	// Context Entries. This field identifies the index of the last valid Endpoint Context within this
	// Device Context structure. The value of ‘0’ is Reserved and is not a valid entry for this field. Valid
	// entries for this field shall be in the range of 1-31. This field indicates the size of the Device
	// Context structure. For example, ((Context Entries+1) * 32 bytes) = Total bytes for this structure.
	// Note, Output Context Entries values are written by the xHC, and Input Context Entries values are
	// written by software.
	unsigned char context_entries;
	
	// Max Exit Latency. The Maximum Exit Latency is in microseconds, and indicates the worst case
	// time it takes to wake up all the links in the path to the device, given the current USB link level
	// power management settings.
	// Refer to section 4.23.5.2 for more information on the use of this field.
	unsigned char max_exit_latency;
	
	// Root Hub Port Number. This field identifies the Root Hub Port Number used to access the USB
	// device. Refer to section 4.19.7 for port numbering information.
	// Note: Ports are numbered from 1 to MaxPorts.
	unsigned char root_hub_port_number;
	
	// Number of Ports. If this device is a hub (Hub = ‘1’), then this field is set by software to identify
	// the number of downstream facing ports supported by the hub. Refer to the bNbrPorts field
	// description in the Hub Descriptor (Table 11-13) of the USB2 spec. If this device is not a hub (Hub
	// = ‘0’), then this field shall be ‘0’.
	unsigned char number_of_ports;
	
	// TT Hub Slot ID. If this device is Low-/Full-speed and connected through a High-speed hub, then
	// this field shall contain the Slot ID of the parent High-speed hub 108 . If this device is attached to a
	// Root Hub port or it is not Low-/Full-speed then this field shall be '0'.
	unsigned char tt_hub_slot_id;
	
	// TT Port Number. If this device is Low-/Full-speed and connected through a High-speed hub,
	// then this field contains the number of the downstream facing port of the parent High-speed 108
	// hub. If this device is attached to a Root Hub port or it is not Low-/Full-speed then this field shall
	// be '0'.
	unsigned char tt_port_number;
	
	// TT Think Time (TTT). If this is a High-speed hub (Hub = ‘1’ and Speed = High-Speed), then this
	// field shall be set by software to identify the time the TT of the hub requires to proceed to the
	// next full-/low-speed transaction.
	// Value Think Time
	// 0 TT requires at most 8 FS bit times of inter-transaction gap on a full-/low-speed
	// downstream bus.
	// 1 TT requires at most 16 FS bit times.
	// 2 TT requires at most 24 FS bit times.
	// 3 TT requires at most 32 FS bit times.
	// Refer to the TT Think Time sub-field of the wHubCharacteristics field description in the Hub
	// Descriptor (Table 11-13) and section 11.18.2 of the USB2 spec for more information on TT
	// Think Time. If this device is not a High-speed hub (Hub = ‘0’ or Speed != High-speed), then this
	// field shall be ‘0’.
	unsigned char tt_think_time;
	
	// Interrupter Target. This field defines the index of the Interrupter that will receive Bandwidth
	// Request Events and Device Notification Events generated by this slot, or when a Ring Underrun
	// or Ring Overrun condition is reported (refer to section 4.10.3.1). Valid values are between 0 and
	// MaxIntrs-1.
	unsigned char interrupter_target;
	
	// USB Device Address. This field identifies the address assigned to the USB device by the xHC,
	// and is set upon the successful completion of a Set Address Command. Refer to the USB2 spec
	// for a more detailed description.
	// As Output, this field is invalid if the Slot State = Disabled or Default.
	// As Input, software shall initialize the field to ‘0’.
	unsigned char usb_device_address;
	
	// Slot State. This field is updated by the xHC when a Device Slot transitions from one state to
	// another.
	// Value Slot State
	// 0 Disabled/Enabled
	// 1 Default
	// 2 Addressed
	// 3 Configured
	// 31-4 Reserved
	// Slot States are defined in section 4.5.3.
	// As Output, since software initializes all fields of the Device Context data structure to ‘0’, this field
	// shall initially indicate the Disabled state.
	// As Input, software shall initialize the field to ‘0’.
	// Refer to section 4.5.3 for more information on Slot State.
	unsigned char slot_state;
	
}XHCI_SLOT_CONTEXT;

typedef struct{
	// Endpoint State (EP State). The Endpoint State identifies the current operational state of the
	// endpoint.
	// Value Definition
	// 0 Disabled The endpoint is not operational
	// 1 Running The endpoint is operational, either waiting for a doorbell ring or processing
	// TDs.
	// 2 Halted The endpoint is halted due to a Halt condition detected on the USB. SW shall issue
	// Reset Endpoint Command to recover from the Halt condition and transition to the Stopped
	// state. SW may manipulate the Transfer Ring while in this state.
	// 3 Stopped The endpoint is not running due to a Stop Endpoint Command or recovering
	// from a Halt condition. SW may manipulate the Transfer Ring while in this state.
	// 4 Error The endpoint is not running due to a TRB Error. SW may manipulate the Transfer
	// Ring while in this state.
	// 5-7
	// Reserved
	// As Output, a Running to Halted transition is forced by the xHC if a STALL condition is detected
	// on the endpoint. A Running to Error transition is forced by the xHC if a TRB Error condition is
	// detected.
	// As Input, this field is initialized to ‘0’ by software.
	// Refer to section 4.8.3 for more information on Endpoint State.
	unsigned char endpoint_state;
	
	// Mult. If LEC = ‘0’, then this field indicates the maximum number of bursts within an Interval that
	// this endpoint supports, where the valid range of values is ‘0’ to ‘2’, where ‘0’ = 1 burst, ‘1’ = 2
	// bursts, etc. 109 This field shall be ‘0’ for all endpoint types except for SS Isochronous.
	// If LEC = ‘1’, then this field shall be RsvdZ and Mult is calculated as:
	// (Max ESIT Payload / Max Packet Size / Max Burst Size) rounded up to the nearest integer value.
	unsigned char mult;
	
	// Max Primary Streams (MaxPStreams). This field identifies the maximum number of Primary
	// Stream IDs this endpoint supports. Valid values are defined below. If the value of this field is ‘0’,
	// then the TR Dequeue Pointer field shall point to a Transfer Ring. If this field is > '0' then the TR
	// Dequeue Pointer field shall point to a Primary Stream Context Array. Refer to section 4.12 for
	// more information.
	// A value of ‘0’ indicates that Streams are not supported by this endpoint and the Endpoint
	// Context TR Dequeue Pointer field references a Transfer Ring.
	// A value of ‘1’ to ‘15’ indicates that the Primary Stream ID Width is MaxPstreams+1 and the
	// Primary Stream Array contains 2 MaxPStreams+1 entries.
	// For SS Bulk endpoints, the range of valid values for this field is defined by the MaxPSASize field
	// in the HCCPARAMS1 register (refer to Table 5-13).
	// This field shall be '0' for all SS Control, Isoch, and Interrupt endpoints, and for all non-SS
	// endpoints.
	unsigned char maxpstreams;
	
	// Linear Stream Array (LSA). This field identifies how a Stream ID shall be interpreted.
	// Setting this bit to a value of ‘1’ shall disable Secondary Stream Arrays and a Stream ID shall be
	// interpreted as a linear index into the Primary Stream Array, where valid values for MaxPStreams
	// are ‘1’ to ‘15’.
	// A value of ‘0’ shall enable Secondary Stream Arrays, where the low order (MaxPStreams+1) bits
	// of a Stream ID shall be interpreted as a linear index into the Primary Stream Array, where valid
	// values for MaxPStreams are ‘1’ to ‘7’. And the high order bits of a Stream ID shall be interpreted
	// as a linear index into the Secondary Stream Array.
	// If MaxPStreams = ‘0’, this field RsvdZ.
	// Refer to section 4.12.2 for more information.
	unsigned char lsa;
	
	// Interval. The period between consecutive requests to a USB endpoint to send or receive data.
	// Expressed in 125 μs. increments. The period is calculated as 125 μs. * 2 Interval ; e.g., an Interval
	// value of 0 means a period of 125 μs. (2 0 = 1 * 125 μs.), a value of 1 means a period of 250 μs. (2 1
	// = 2 * 125 μs.), a value of 4 means a period of 2 ms. (2 4 = 16 * 125 μs.), etc. Refer to Table 6-12
	// for legal Interval field values. See further discussion of this field below. Refer to section 6.2.3.6
	// for more information.
	unsigned char interval;
	
	// Max Endpoint Service Time Interval Payload High (Max ESIT Payload Hi). If LEC = '1', then this
	// field indicates the high order 8 bits of the Max ESIT Payload value. If LEC = '0', then this field
	// shall be RsvdZ. Refer to section 6.2.3.8 for more information.
	unsigned char maxexitpayloadhigh;
	
	// Error Count (CErr) 110 . This field defines a 2-bit down count, which identifies the number of
	// consecutive USB Bus Errors allowed while executing a TD. If this field is programmed with a
	// non-zero value when the Endpoint Context is initialized, the xHC loads this value into an internal
	// Bus Error Counter before executing a USB transaction and decrements it if the transaction fails.
	// If the Bus Error Counter counts from ‘1’ to ‘0’, the xHC ceases execution of the TRB, sets the
	// endpoint to the Halted state, and generates a USB Transaction Error Event for the TRB that
	// caused the internal Bus Error Counter to decrement to ‘0’. If system software programs this field
	// to ‘0’, the xHC shall not count errors for TRBs on the Endpoint’s Transfer Ring and there shall be
	// no limit on the number of TRB retries. Refer to section 4.10.2.7 for more information on the
	// operation of the Bus Error Counter.
	// Note: CErr does not apply to Isoch endpoints and shall be set to ‘0’ if EP Type = Isoch Out ('1') or
	// Isoch In ('5').
	unsigned char cerr;
	
	// Endpoint Type (EP Type). This field identifies whether an Endpoint Context is Valid, and if so,
	// what type of endpoint the context defines.
	// Value Endpoint Type Direction
	// 0 Not Valid N/A
	// 1 Isoch Out
	// 2 Bulk Out
	// 3 Interrupt Out
	// 4 Control
	// Bidirectional
	// 5 Isoch In
	// 6 Bulk In
	// 7 Interrupt In
	unsigned char endpointtype;
	
	// Host Initiate Disable (HID). This field affects Stream enabled endpoints, allowing the Host
	// Initiated Stream selection feature to be disabled for the endpoint. Setting this bit to a value of
	// ‘1’ shall disable the Host Initiated Stream selection feature. A value of ‘0’ will enable normal
	// Stream operation. Refer to section 4.12.1.1 for more information.
	unsigned char hid;
	
	// Max Burst Size. This field indicates to the xHC the maximum number of consecutive USB
	// transactions that should be executed per scheduling opportunity. This is a “zero-based” value,
	// where 0 to 15 represents burst sizes of 1 to 16, respectively. Refer to section 6.2.3.4 for more
	// information.
	unsigned char maxburstsize;
	
	// Max Packet Size. This field indicates the maximum packet size in bytes that this endpoint is
	// capable of sending or receiving when configured. Refer to section 6.2.3.5 for more information.
	unsigned char maxpacketsize;
	
	// Dequeue Cycle State (DCS). This bit identifies the value of the xHC Consumer Cycle State (CCS)
	// flag for the TRB referenced by the TR Dequeue Pointer. Refer to section 4.9.2 for more
	// information. This field shall be ‘0’ if MaxPStreams > ‘0’.
	unsigned char dcs;
	
	// TR Dequeue Pointer. As Input, this field represents the high order bits of the 64-bit base address
	// of a Transfer Ring or a Stream Context Array associated with this endpoint. If MaxPStreams = '0'
	// then this field shall point to a Transfer Ring. If MaxPStreams > '0' then this field shall point to a
	// Stream Context Array.
	// As Output, if MaxPStreams = ‘0’ this field shall be used by the xHC to store the value of the
	// Dequeue Pointer when the endpoint enters the Halted or Stopped states, and the value of the
	// this field shall be undefined when the endpoint is not in the Halted or Stopped states. if
	// MaxPStreams > ‘0’ then this field shall point to a Stream Context Array.
	// The memory structure referenced by this physical memory pointer shall be aligned to a 16-byte
	// boundary.
	unsigned long long dequeuepointer;
	
	// Average TRB Length. This field represents the average Length of the TRBs executed by this
	// endpoint. The value of this field shall be greater than ‘0’. Refer to section 4.14.1.1 and the
	// implementation note TRB Lengths and System Bus Bandwidth for more information.
	// The xHC shall use this parameter to calculate system bus bandwidth requirements.
	unsigned char average_trb_length;
	
	// Max Endpoint Service Time Interval Payload Low (Max ESIT Payload Lo). This field indicates
	// the low order 16 bits of the Max ESIT Payload. The Max ESIT Payload represents the total
	// number of bytes this endpoint will transfer during an ESIT. This field is only valid for periodic
	// endpoints. Refer to section 6.2.3.8 for more information.
	unsigned char maxpayloadlow;
}XHCI_ENDPOINT_CONTEXT;

typedef struct{
	unsigned long bar1;
	unsigned long bar2;
	unsigned long bar3;
	unsigned long bar4;
}TRB;

extern void xhciirq();

unsigned long basebar = 0;
unsigned long usbcmd = 0;
unsigned long usbsts = 0;
unsigned long config = 0;
unsigned long bcbaap = 0;
unsigned long crcr   = 0;
unsigned long doorbel= 0;
unsigned long dnctrl = 0;
unsigned long erstsz = 0;
unsigned long erstba = 0;
unsigned long rtsoff = 0;
unsigned long deviceid = 0;
unsigned long iman_addr= 0;

TRB event_ring_queue[20] __attribute__ ((aligned (0x100))); 
TRB command_ring_control[20] __attribute__ ((aligned (0x100)));
unsigned long command_ring_offset = 0;
unsigned long event_ring_offset = 0;

void xhci_stop_codon_to_trb(TRB *out){
	out->bar1 = 0;
	out->bar2 = 0;
	out->bar3 = 0;
	out->bar4 = 0;
	
	
	if(deviceid!=XHCI_DEVICE_BOCHS){
		out->bar4 = 0;
	}else{
		out->bar4 = 1;
	}
}

void xhci_trb_enable_slot_to_trb(XHCI_TRB_ENABLE_SLOT in,TRB *out){
	out->bar1 = 0;
	out->bar2 = 0;
	out->bar3 = 0;
	out->bar4 = 0;
	
	out->bar4 =  XHCI_TRB_ENABLE_SLOT_SLOTTYPE(in.slottype) | XHCI_TRB_SET_TRB_TYPE(in.trbtype) | XHCI_TRB_SET_CYCLE_BIT(in.cyclebit);
}

void xhci_trb_set_address_to_trb(XHCI_TRB_SET_ADDRESS in,TRB *out){
	out->bar1 = 0;
	out->bar2 = 0;
	out->bar3 = 0;
	out->bar4 = 0;
	
	out->bar1 = XHCI_TRB_SET_ADDRESS_POINTER(in.input_context);
	out->bar4 = XHCI_TRB_SET_SLOT(in.slotid) | XHCI_TRB_SET_TRB_TYPE(in.trbtype) | XHCI_TRB_SET_ADDRESS_BSR(in.bsr) | XHCI_TRB_SET_CYCLE_BIT(in.cyclebit);
}

void xhci_input_control_conext_to_addr(XHCI_INPUT_CONTROL_CONTEXT in,unsigned long *out){
	out[0] = 0;
	out[1] = 0;
	out[2] = 0;
	out[3] = 0;
	out[4] = 0;
	out[5] = 0;
	out[6] = 0;
	out[7] = 0;
	
	out[0] = in.D;
	out[1] = in.A;
	
	out[7] = XHCI_INPUT_CONTROL_CONTEXT_ALTERNATE_SETTING(in.alternate_setting) | XHCI_INPUT_CONTROL_CONTEXT_INTERFACE_NUMBER(in.interface_number) | XHCI_INPUT_CONTROL_CONTEXT_CONFIGURATION_VALUE(in.configuration_value);
}

void xhci_slot_context_to_addr(XHCI_SLOT_CONTEXT in , unsigned long *out){
	out[0] = 0;
	out[1] = 0;
	out[2] = 0;
	out[3] = 0;
	out[4] = 0;
	out[5] = 0;
	out[6] = 0;
	out[7] = 0;
	
	out[0] = XHCI_SLOT_CONTEXT_CONTEXT_ENTRIES(in.context_entries) | XHCI_SLOT_CONTEXT_MTT(in.hub) | XHCI_SLOT_CONTEXT_MTT(in.mtt) | XHCI_SLOT_CONTEXT_SPEED(in.speed) | XHCI_SLOT_CONTEXT_ROUTE_STRING(in.route_string);
	out[1] = XHCI_SLOT_CONTEXT_NUM_OF_PORTS(in.number_of_ports) | XHCI_SLOT_CONTEXT_ROOT_HUB_PORT_NUMBER(in.root_hub_port_number) | XHCI_SLOT_CONTEXT_MAX_EXIT_LATENCY(in.max_exit_latency);
	out[2] = XHCI_SLOT_CONTEXT_INTERRUPTER_TARGET(in.interrupter_target) | XHCI_SLOT_CONTEXT_TTT(in.tt_think_time) | XHCI_SLOT_CONTEXT_TT_PORT_NUMBER(in.tt_port_number) | XHCI_SLOT_CONTEXT_TT_HUB_SLOT_ID(in.tt_hub_slot_id);
	out[3] = XHCI_SLOT_CONTEXT_SLOT_STATE(in.slot_state) | XHCI_SLOT_CONTEXT_USB_DEVICE_ADDRESS(in.usb_device_address);
}

void xhci_endpoint_context_to_addr(XHCI_ENDPOINT_CONTEXT in, unsigned long *out){
	out[0] = 0;
	out[1] = 0;
	out[2] = 0;
	out[3] = 0;
	out[4] = 0;
	out[5] = 0;
	out[6] = 0;
	out[7] = 0;
	
	out[0] = XHCI_ENDPOINT_CONTEXT_MAX_ENDPOINT_SERVICE(in.maxexitpayloadhigh) | XHCI_ENDPOINT_CONTEXT_INTERVAL(in.interval) | XHCI_ENDPOINT_CONTEXT_LINEAR_STREAM_ARRAY(in.lsa) | XHCI_ENDPOINT_CONTEXT_MAX_PRIMAIRY_STREAMS(in.maxpstreams) | XHCI_ENDPOINT_CONTEXT_MULT(in.mult) | XHCI_ENDPOINT_CONTEXT_ENDPOINT_STATE(in.endpoint_state);
	out[1] = XHCI_ENDPOINT_CONTEXT_MAX_PACKET_SIZE(in.maxpacketsize) | XHCI_ENDPOINT_CONTEXT_MAX_BURST_SIZE(in.maxburstsize) | XHCI_ENDPOINT_CONTEXT_HOST_INITIATE_DISABLE(in.hid) | XHCI_ENDPOINT_CONTEXT_ENDPOINT_TYPE(in.endpointtype) | XHCI_ENDPOINT_CONTEXT_ERR_CNT(in.cerr);
	out[2] = XHCI_ENDPOINT_CONTEXT_DP(in.dequeuepointer) | XHCI_ENDPOINT_CONTEXT_DSC(in.dcs);
	out[4] = XHCI_ENDPOINT_CONTEXT_MAX_ENDPOINT_SERVICE_LO(in.maxpayloadlow) | XHCI_ENDPOINT_CONTEXT_AVG_TRB_LENGTH(in.average_trb_length);
}

int xhci_set_address(unsigned long assignedSloth,unsigned long* t,unsigned char bsr){
	// Address Device Command BSR1
	TRB* trb = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	XHCI_TRB_SET_ADDRESS set_address;
	set_address.input_context = (unsigned long) t;
	set_address.cyclebit = deviceid!=XHCI_DEVICE_BOCHS?1:0;
	set_address.bsr = bsr;
	set_address.trbtype = 11;
	set_address.slotid = assignedSloth;
	xhci_trb_set_address_to_trb(set_address,trb);
	
	command_ring_offset += 0x10;
	
	// stop codon
	TRB *trb6 = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	xhci_stop_codon_to_trb(trb6);
	
	// doorbell
	((unsigned long*)doorbel)[0] = 0;
	
	// wait
	while(1){
		unsigned long r = ((unsigned long*)iman_addr)[0];
		if(r&1){
			break;
		}
	}
	
	// RESULTS
	TRB* trbres2 = ((TRB*)((unsigned long)(&event_ring_queue)+event_ring_offset));
	unsigned long completioncode2 = (trbres2->bar3 & 0b111111100000000000000000000000) >> 24;
	
	event_ring_offset += 0x10;
	return completioncode2;
}

int xhci_enable_slot(){
	TRB* trb2 = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	XHCI_TRB_ENABLE_SLOT enable_slot;
	enable_slot.cyclebit = deviceid!=XHCI_DEVICE_BOCHS?1:0;
	enable_slot.trbtype = 9;
	enable_slot.slottype = 0;
	xhci_trb_enable_slot_to_trb(enable_slot,trb2);
	
	command_ring_offset += 0x10;
	
	TRB* trb = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	xhci_stop_codon_to_trb(trb);
	
	((unsigned long*)doorbel)[0] = 0;
	
	while(1){
		unsigned long r = ((unsigned long*)iman_addr)[0];
		if(r&1){
			break;
		}
	}
	
	TRB *trbres = ((TRB*)((unsigned long)(&event_ring_queue)+event_ring_offset));
	unsigned char assignedSloth = (trbres->bar4 & 0b111111100000000000000000000000) >> 24;
	unsigned char completioncode = (trbres->bar3 & 0b111111100000000000000000000000) >> 24;
	if(completioncode!=1){
		return 0;
	}
	
	event_ring_offset += 0x10;
	return assignedSloth;
}

void irq_xhci(){
	unsigned long xhci_usbsts = ((unsigned long*)usbsts)[0];
	if(xhci_usbsts&4){
		printf("[XHCI] Host system error interrupt\n");
	}
	if(xhci_usbsts&8){
		printf("[XHCI] Event interrupt\n");
	}
	if(xhci_usbsts&0x10){
		printf("[XHCI] Port interrupt\n");
	}
	unsigned long iman_addr = rtsoff + 0x020;
	((unsigned long*)iman_addr)[0] &= ~1;
	printf("[XHCI] ISTS %x \n",((unsigned long*)iman_addr)[0]);
	outportb(0xA0,0x20);
	outportb(0x20,0x20);
}

// offset intell xhci 0x47C
void init_xhci(unsigned long bus,unsigned long slot,unsigned long function){
	printf("[XHCI] entering xhci driver....\n");
	unsigned long usbint = getBARaddress(bus,slot,function,0x3C) & 0x000000FF;
	setNormalInt(usbint,(unsigned long)xhciirq);
//	
//	GETTING BASIC INFO
	deviceid = (getBARaddress(bus,slot,function,0) & 0xFFFF0000) >> 16;
	unsigned long ccr = (getBARaddress(bus,slot,function,0x08) & 0b11111111111111111111111100000000) >> 8;
	unsigned long bar = getBARaddress(bus,slot,function,0x10);
	unsigned long sbrn = (getBARaddress(bus,slot,function,0x60) & 0x0000FF);
	
	printf("[XHCI] START=%x \n",bar);
	while(1){
		if((bar&0xFF)>0x80){
			bar++;
		}else{
			bar--;
		}
		if((bar&0xFF)==00){
			break;
		}
	}
	printf("[XHCI] HALT=%x SEGM=%x \n",bar,((unsigned char*)bar)[0]);
	
//
// Calculating base address
	basebar = bar+((unsigned char*)bar)[0];
	if(deviceid==0x22B5){
		printf("[XHCI] INTELL XHCI CONTROLLER\n");
	}else if(deviceid==0xD){
		printf("[XHCI] QEMU XHCI CONTROLLER\n");
	}else if(deviceid==XHCI_DEVICE_BOCHS){
		printf("[XHCI] BOCHS XHCI CONTROLLER\n");
	}else if(deviceid==0x1E31){
		printf("[XHCI] VIRTUALBOX XHCI CONTROLLER\n");
	}else{
		printf("[XHCI] UNKNOWN XHCI CONTROLLER %x \n",deviceid);
	}
	printf("[XHCI] Serial Bus Release Number Register %x \n",sbrn);
	printf("[XHCI] Class Code Register %x \n",ccr);
	if(!(ccr==0x0C0330&&(sbrn==0x30||sbrn==0x31))){
		printf("[XHCI] Incompatible device!\n");
		return;
	}
	unsigned long hciversionaddr = bar+2;
	unsigned long hciversion = ((unsigned long*)hciversionaddr)[0];
	printf("[XHCI] hciversion %x \n",hciversion);
	unsigned long capdb = bar+0x14;
	unsigned long hciparamadr = bar+0x04;
	unsigned long hciparam1 = ((unsigned long*)hciparamadr)[0];
	unsigned long portcount = hciparam1>>24;
	unsigned long hccparams1adr = bar+0x10;
	unsigned long hccparams1 = ((unsigned long*)hccparams1adr)[0];
	printf("[XHCI] hccparams1 %x \n",hccparams1);
	if(hccparams1&1){
		printf("[XHCI] has 64-bit Addressing Capability\n");
	}
	if(hccparams1&0xFFFF0000){
		printf("[XHCI] has xHCI Extended Capabilities Pointer ( %x )\n",(hccparams1&0xFFFF0000)>>16);
	}
	
	while(1){
		if((rtsoff&0xFF)>0x80){rtsoff++;}else{rtsoff--;}
		if((rtsoff&0xFF)==00){
			break;
		}
	}
	unsigned long rtsoffa = bar+0x18;
	rtsoff = bar+(((unsigned long*)rtsoffa)[0]&0xFFFFFFE0);
	iman_addr = rtsoff + 0x020;
	
	printf("[XHCI] Runtime offset %x (BAR: %x)\n",rtsoff,bar);
	printf("[XHCI] portcount %x \n",portcount);
	doorbel = bar+((unsigned long*)capdb)[0];
	printf("[XHCI] doorbell offset %x \n",doorbel);
	
	printf("[XHCI] basebar=%x \n",basebar);
//
// Calculating other addresses
	usbcmd = basebar;
	usbsts = basebar+0x04;
	config = basebar+0x38;
	bcbaap = basebar+0x30;
	crcr   = basebar+0x18;
	dnctrl = basebar+0x14;
	
	printf("[XHCI] default value CONFIG %x \n",((unsigned long*)config)[0]);
	printf("[XHCI] default value BCBAAP %x \n",((unsigned long*)bcbaap)[0]);
	printf("[XHCI] default value CRCR   %x \n",((unsigned long*)crcr)[0]);
	
	unsigned long xhci_usbcmd = ((unsigned long*)usbcmd)[0];
	unsigned long xhci_usbsts = ((unsigned long*)usbsts)[0];
//
// Stopping controller when running
	if((xhci_usbcmd & 1)==1){
		printf("[XHCI] controller already running! (%x)\n",xhci_usbcmd);
		// ask controller to stop
		resetTicks();
		((unsigned long*)usbcmd)[0] &= ~1;
		while(getTicks()<5);
		xhci_usbcmd = ((unsigned long*)usbcmd)[0];
		xhci_usbsts = ((unsigned long*)usbsts)[0];
		if((xhci_usbsts & 1)==1&&(xhci_usbcmd & 1)==0){
			printf("[XHCI] stopping of hostcontroller succeed\n");
		}else{
			printf("[XHCI] failed to halt hostcontroller %x %x \n",xhci_usbcmd,xhci_usbsts);
		}
	}
	
//
// Lets reset!
	printf("[XHCI] Resetting XHCI \n");
	resetTicks();
	((unsigned long*)usbcmd)[0] |= 2;
	while(getTicks()<5);
	xhci_usbcmd = ((unsigned long*)usbcmd)[0];
	xhci_usbsts = ((unsigned long*)usbsts)[0];
	printf("[XHCI] Reset XHCI finished with USBCMD %x and USBSTS %x \n",xhci_usbcmd,xhci_usbsts);
//
// Setup default parameters
	// TELL XHCI TO REPORT EVERYTHING
//	((unsigned long*)dnctrl)[0] |= 0b1111111111111111;
	
	// setting up interrupter management register
	
	// setting up "Event Ring Segment Table"
	printf("[XHCI] Setting up Event Ring Segment Table\n");
	unsigned long rsb1[20]  __attribute__ ((aligned (0x100)));
	unsigned long rsb2 = ((unsigned long)&rsb1)+4;
	unsigned long rsb3 = ((unsigned long)&rsb1)+8;
	
	((unsigned long*)&rsb1)[0] = ((unsigned long)&event_ring_queue); 	// pointer to event ring queue 0x41400
	((unsigned long*)rsb2)[0] = 0;
	((unsigned long*)rsb3)[0] |= 16;	// size of ring segment (minimal length)
	
	// setting up "Event Ring Segment Table Size Register (ERSTSZ)"
	printf("[XHCI] Setting up Event Ring Segment Size Register\n");
	unsigned long erstsz_addr = rtsoff + 0x028;
	((unsigned long*)erstsz_addr)[0] |= 1; // keep only 1 open
	
	// setting up "Event Ring Dequeue Pointer Register (ERDP)"
	printf("[XHCI] Setting up Event Ring Dequeue Pointer Register\n");
	unsigned long erdp_addr = rtsoff + 0x038;
	((unsigned long*)erdp_addr)[0] = ((unsigned long)&event_ring_queue); // set addr of event ring dequeue pointer register
	((unsigned long*)erdp_addr)[0] &= ~0b1000; // clear bit 3
	((unsigned long*)erdp_addr)[1] = 0; // clear 64bit
	
	// setting up "Event Ring Segment Table Base Address Register (ERSTBA)"
	printf("[XHCI] Setting up Event Ring Segment Table Bse Address Register\n");
	unsigned long erstba_addr = rtsoff + 0x030;
	((unsigned long*)erstba_addr)[0] = (unsigned long)&rsb1; 	// table at 0x1000 for now
	((unsigned long*)erstba_addr)[1] = 0; 	// sending 0 to make sure...
	
	// setting up "Command Ring Control Register (CRCR)"
	printf("[XHCI] Setting up Command Ring Control Register\n");
	((unsigned long*)crcr)[0] |= ((unsigned long)&command_ring_control);
	((unsigned long*)crcr)[1] = 0;
	
	// DCBAAP
	printf("[XHCI] Setting up DCBAAP\n");
	unsigned long btc[20] __attribute__ ((aligned (0x100)));
	((unsigned long*)bcbaap)[0] |= (unsigned long)&btc;
	((unsigned long*)bcbaap)[1] = 0;
	
	// setting first interrupt enabled.
	if(0){
		printf("[XHCI] Setting up First Interrupter\n");
		unsigned long iman_addr = rtsoff + 0x020;
		((unsigned long*)iman_addr)[0] |= 0b10; // Interrupt Enable (IE) – RW
	}
	// TELL XHCI TO USE INTERRUPTS
	printf("[XHCI] Use interrupts\n");
	((unsigned long*)usbcmd)[0] |= 4;
	
	printf("[XHCI] Wait 5s\n");
	resetTicks();
	while(getTicks()<5);
//
// Start system
	printf("[XHCI] Run!\n");
	((unsigned long*)usbcmd)[0] |= 1;
	
	printf("[XHCI] Wait 5s\n");
	resetTicks();
	while(getTicks()<5);
	xhci_usbcmd = ((unsigned long*)usbcmd)[0];
	xhci_usbsts = ((unsigned long*)usbsts)[0];
	printf("[XHCI] System up and running with USBCMD %x and USBSTS %x \n",xhci_usbcmd,xhci_usbsts);
	
	printf("[XHCI] Checking if portchange happened\n");
	if(xhci_usbsts & 0b10000){
		printf("[XHCI] Portchange detected!\n");
	}
	printf("[XHCI] Probing ports....\n");
	//
	// First, see which ports are available
	// USB3.0 will say automatically he is there.
	// USB2.0 should be activated manually
	//
	// After this we should initialise it by getting its port number from the ring
	unsigned int att = 0;
	for(unsigned int i = 0 ; i < 10 ; i++){//5
		unsigned long map = basebar + 0x400 + (i*0x10);
		unsigned long val = ((unsigned long*)map)[0];
		if(val&3){ // USB 3.0 does everything themselves
			printf("[XHCI] Port %x has a USB3.0 connection (%x)\n",i,val);
			att++;
		}else{ // USB 2.0 however doesnt...
			((unsigned long*)map)[0] = 0b1000010000; // activate power and reset port
			resetTicks();
			while(getTicks()<5);
			val = ((unsigned long*)map)[0];
			if(val&3){
				printf("[XHCI] Port %x has a USB2.0 connection (%x)\n",i,val);
				att++;
			}
		}
		
		
		if(val==0){
			break;
		}
		
		if(val&3){
			printf("[XHCI] Port %x is initialising....\n",i);
		
			// detecting speed....
			unsigned long speed = (val >> 10) & 0b000000000000000000000000000111;
			unsigned long devicespeed = 0;// 8 64 512
			if(speed==XHCI_SPEED_SUPER){
				printf("[XHCI] Port %x : port is a superspeed port\n",i);
				devicespeed = 512;
			}else if(speed==XHCI_SPEED_HI){
				printf("[XHCI] Port %x : port is a highspeed port\n",i);
				devicespeed = 64;
			}else if(speed==XHCI_SPEED_FULL){
				printf("[XHCI] Port %x : port is a fullspeed port\n",i);
				devicespeed = 64;
			}else if(speed==XHCI_SPEED_LOW){
				printf("[XHCI] Port %x : port is a lowspeed port\n",i);
				devicespeed = 8;
			}
			
			//
			//
			// Device Slot Assignment
			//
			//
			
			printf("[XHCI] Port %x : Obtaining device slot...\n",i);
			unsigned long assignedSloth = xhci_enable_slot();
			if(assignedSloth==0){
				printf("[XHCI] Port %x : Assignation of device slot failed!\n",i);
				continue;
			}
			
			//
			//
			// Device Slot Initialisation
			//
			//
			
			printf("[XHCI] Port %x : Device Slot Initialisation BSR1 \n",i);
			
			TRB local_ring_control[20] __attribute__ ((aligned (0x100)));
	
			printf("[XHCI] Port %x : Setting up DCBAAP for port \n",i);
			unsigned long bse = (unsigned long)malloc(0x420);
			btc[(assignedSloth*2)+0] 	= bse;
			btc[(assignedSloth*2)+1] 	= 0;
			
			printf("[XHCI] Port %x : Setting up input controll\n",i);
			unsigned long t[0x30] __attribute__ ((aligned(0x1000)));
			
			printf("[XHCI] Port %x : Setting up Input Controll Context\n",i);
			// Input Control Context
			XHCI_INPUT_CONTROL_CONTEXT input_control_context;
			input_control_context.A = 0b11;
			input_control_context.D = 0b0;
			input_control_context.alternate_setting = 0;
			input_control_context.interface_number = 0;
			input_control_context.configuration_value = 0;
			xhci_input_control_conext_to_addr(input_control_context,t);
			
			printf("[XHCI] Port %x : Setting up Slot Context\n",i);
			// Slot(h) Context
			XHCI_SLOT_CONTEXT slot_context;
			slot_context.root_hub_port_number = 1;
			slot_context.route_string = 0;
			slot_context.context_entries = 1;
			xhci_slot_context_to_addr(slot_context,&t[0x10]); 
			
			printf("[XHCI] Port %x : Setting up Endpoint Context\n",i);
			// Endpoint Context
			XHCI_ENDPOINT_CONTEXT endpoint_context;
			endpoint_context.endpointtype = 4;
			endpoint_context.maxpstreams = 0;
			endpoint_context.mult = 0;
			endpoint_context.cerr = 3;
			endpoint_context.maxburstsize = 0;
			endpoint_context.maxpacketsize = devicespeed;
			endpoint_context.interval = 0;
			endpoint_context.dequeuepointer = (unsigned long)&local_ring_control;
			endpoint_context.dcs = 1;
			xhci_endpoint_context_to_addr(endpoint_context,&t[0x20]);
			
			//
			//
			// SETADDRESS commando
			//
			//
			
			
			printf("[XHCI] Port %x : Obtaining SETADDRESS(BSR=1)\n",i);
			int sares = xhci_set_address(assignedSloth,t,1);
			if(sares!=1){
				printf("[XHCI] Port %x : Assignation of device slot address failed with %x !\n",i,sares);
				continue;
			}
			
			
			printf("[XHCI] Port %x : GET DEVICE DESCRIPTOR\n",i);
			//
			// GET DEVICE DESCRIPTOR
			// trb-type=2
			// trt=3
			// transferlength=8
			// IOC=0
			// IDT=1
			// reqtype= 0x80
			// req=6
			// wValue=0100
			// wIndex=0
			// wLength=0
			//
			
			unsigned char *devicedescriptor = (unsigned char*)malloc(8);
			printf("[XHCI] devicedescriptor located at %x \n",(unsigned long)devicedescriptor);
			TRB *dc1 = ((TRB*)((unsigned long)&local_ring_control));
			dc1->bar1 = 0b00000000000001000000011010000000;
			dc1->bar2 = 0b00000000000000000000000000000000;
			dc1->bar3 = 0b00000000010000000000000000001000;
			dc1->bar4 = 0b00000000000000110000100001000001;
			
			// single date stage
			// TRB Type = Data Stage TRB.
			// X Direction (DIR) = ‘1’.
			// X TRB Transfer Length = 8.
			// X Chain bit (CH) = 0.
			// X Interrupt On Completion (IOC) = 0.
			// X Immediate Data (IDT) = 0.
			// X Data Buffer Pointer = The address of the Device Descriptor receive buffer.
			// X Cycle bit = Current Producer Cycle State.
			TRB *dc2 = ((TRB*)((unsigned long)(&local_ring_control)+0x10));
			dc2->bar1 = (unsigned long)devicedescriptor;
			dc2->bar2 = 0b00000000000000000000000000000000;
			dc2->bar3 = 0b00000000000000000000000000001000;
			dc2->bar4 = 0b00000000000000010000110000000001;
			
			TRB *dc3 = ((TRB*)((unsigned long)(&local_ring_control)+0x20));
			dc3->bar1 = 0;
			dc3->bar2 = 0;
			dc3->bar3 = 0;
			dc3->bar4 = 0;
			
			
			((unsigned long*)doorbel)[assignedSloth] = 1;
			
			while(1){
				unsigned long r = ((unsigned long*)iman_addr)[0];
				if(r&1){
					break;
				}
			}
			
			printf("[XHCI] Port %x : devdesc %x %x %x %x %x %x %x %x \n",i,devicedescriptor[0],devicedescriptor[1],devicedescriptor[2],devicedescriptor[3],devicedescriptor[4],devicedescriptor[5],devicedescriptor[6],devicedescriptor[7]);
			printf("[XHCI] Port %x : deviceclass=%x \n",i,devicedescriptor[4]);
			if(devicedescriptor[4]==0){
				printf("[XHCI] Port %x : Deviceclass cannot be 0! \n",i);
			}else{
				printf("[XHCI] Port %x : Device initialised succesfully\n",i);
			}
			sleep(10000);
		}else{
			printf("[XHCI] Port %x : No device attached!\n",i);
		}
	}
	
	if(((unsigned long*)crcr)[0]==0x8){
		printf("[XHCI] circulair command ring is running\n");
	}
	printf("[XHCI] All finished!\n");for(;;);
}
