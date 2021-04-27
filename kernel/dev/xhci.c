#include "../kernel.h"
#define XHCI_DEVICE_BOCHS 0x15
#define XHCI_DEVICE_QEMU 0xD
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
#define XHCI_ENDPOINT_CONTEXT_DP(x)				(((x)))
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
	unsigned short speed;
	
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
	unsigned short context_entries;
	
	// Max Exit Latency. The Maximum Exit Latency is in microseconds, and indicates the worst case
	// time it takes to wake up all the links in the path to the device, given the current USB link level
	// power management settings.
	// Refer to section 4.23.5.2 for more information on the use of this field.
	unsigned short max_exit_latency;
	
	// Root Hub Port Number. This field identifies the Root Hub Port Number used to access the USB
	// device. Refer to section 4.19.7 for port numbering information.
	// Note: Ports are numbered from 1 to MaxPorts.
	unsigned short root_hub_port_number;
	
	// Number of Ports. If this device is a hub (Hub = ‘1’), then this field is set by software to identify
	// the number of downstream facing ports supported by the hub. Refer to the bNbrPorts field
	// description in the Hub Descriptor (Table 11-13) of the USB2 spec. If this device is not a hub (Hub
	// = ‘0’), then this field shall be ‘0’.
	unsigned short number_of_ports;
	
	// TT Hub Slot ID. If this device is Low-/Full-speed and connected through a High-speed hub, then
	// this field shall contain the Slot ID of the parent High-speed hub 108 . If this device is attached to a
	// Root Hub port or it is not Low-/Full-speed then this field shall be '0'.
	unsigned short tt_hub_slot_id;
	
	// TT Port Number. If this device is Low-/Full-speed and connected through a High-speed hub,
	// then this field contains the number of the downstream facing port of the parent High-speed 108
	// hub. If this device is attached to a Root Hub port or it is not Low-/Full-speed then this field shall
	// be '0'.
	unsigned short tt_port_number;
	
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
	unsigned short tt_think_time;
	
	// Interrupter Target. This field defines the index of the Interrupter that will receive Bandwidth
	// Request Events and Device Notification Events generated by this slot, or when a Ring Underrun
	// or Ring Overrun condition is reported (refer to section 4.10.3.1). Valid values are between 0 and
	// MaxIntrs-1.
	unsigned short interrupter_target;
	
	// USB Device Address. This field identifies the address assigned to the USB device by the xHC,
	// and is set upon the successful completion of a Set Address Command. Refer to the USB2 spec
	// for a more detailed description.
	// As Output, this field is invalid if the Slot State = Disabled or Default.
	// As Input, software shall initialize the field to ‘0’.
	unsigned short usb_device_address;
	
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
	unsigned long slot_state;
	
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
	unsigned short endpoint_state;
	
	// Mult. If LEC = ‘0’, then this field indicates the maximum number of bursts within an Interval that
	// this endpoint supports, where the valid range of values is ‘0’ to ‘2’, where ‘0’ = 1 burst, ‘1’ = 2
	// bursts, etc. 109 This field shall be ‘0’ for all endpoint types except for SS Isochronous.
	// If LEC = ‘1’, then this field shall be RsvdZ and Mult is calculated as:
	// (Max ESIT Payload / Max Packet Size / Max Burst Size) rounded up to the nearest integer value.
	unsigned short mult;
	
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
	unsigned short maxpstreams;
	
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
	unsigned short lsa;
	
	// Interval. The period between consecutive requests to a USB endpoint to send or receive data.
	// Expressed in 125 μs. increments. The period is calculated as 125 μs. * 2 Interval ; e.g., an Interval
	// value of 0 means a period of 125 μs. (2 0 = 1 * 125 μs.), a value of 1 means a period of 250 μs. (2 1
	// = 2 * 125 μs.), a value of 4 means a period of 2 ms. (2 4 = 16 * 125 μs.), etc. Refer to Table 6-12
	// for legal Interval field values. See further discussion of this field below. Refer to section 6.2.3.6
	// for more information.
	unsigned short interval;
	
	// Max Endpoint Service Time Interval Payload High (Max ESIT Payload Hi). If LEC = '1', then this
	// field indicates the high order 8 bits of the Max ESIT Payload value. If LEC = '0', then this field
	// shall be RsvdZ. Refer to section 6.2.3.8 for more information.
	unsigned short maxexitpayloadhigh;
	
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
	unsigned short cerr;
	
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
	unsigned short endpointtype;
	
	// Host Initiate Disable (HID). This field affects Stream enabled endpoints, allowing the Host
	// Initiated Stream selection feature to be disabled for the endpoint. Setting this bit to a value of
	// ‘1’ shall disable the Host Initiated Stream selection feature. A value of ‘0’ will enable normal
	// Stream operation. Refer to section 4.12.1.1 for more information.
	unsigned short hid;
	
	// Max Burst Size. This field indicates to the xHC the maximum number of consecutive USB
	// transactions that should be executed per scheduling opportunity. This is a “zero-based” value,
	// where 0 to 15 represents burst sizes of 1 to 16, respectively. Refer to section 6.2.3.4 for more
	// information.
	unsigned short maxburstsize;
	
	// Max Packet Size. This field indicates the maximum packet size in bytes that this endpoint is
	// capable of sending or receiving when configured. Refer to section 6.2.3.5 for more information.
	unsigned short maxpacketsize;
	
	// Dequeue Cycle State (DCS). This bit identifies the value of the xHC Consumer Cycle State (CCS)
	// flag for the TRB referenced by the TR Dequeue Pointer. Refer to section 4.9.2 for more
	// information. This field shall be ‘0’ if MaxPStreams > ‘0’.
	unsigned short dcs;
	
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
	unsigned short average_trb_length;
	
	// Max Endpoint Service Time Interval Payload Low (Max ESIT Payload Lo). This field indicates
	// the low order 16 bits of the Max ESIT Payload. The Max ESIT Payload represents the total
	// number of bytes this endpoint will transfer during an ESIT. This field is only valid for periodic
	// endpoints. Refer to section 6.2.3.8 for more information.
	unsigned short maxpayloadlow;
}XHCI_ENDPOINT_CONTEXT;

typedef struct{
	
	// Cycle bit (C). This bit is used to mark the Enqueue Pointer of a Command Ring.
	unsigned char cycle_bit;
	
	// TRB Type. This field identifies the type of the TRB. Refer to Table 6-86 for the definition of the
	// Disable Slot Command TRB type ID.
	unsigned char trb_type;
	
	// Slot ID. The ID of the Device Slot to disable.
	unsigned char slot_id;
}XHCI_TRB_DISABLE_SLOT;

typedef struct{
	unsigned char bLength; // 18
	unsigned char bDescriptorType; // 1
	unsigned short bcdUSB; // specnumber
	unsigned char bDeviceClass; // classcode
	unsigned char bDeviceSubClass; 
	unsigned char bDeviceProtocol;
	unsigned char bMaxPacketSize;
}XHCI_DEVICE_DESCRIPTOR;

/**
 * The interrupter for xhci
 */
extern void xhciirq();

unsigned long basebar = 0;
unsigned long usbcmd = 0;
unsigned long usbsts = 0;
unsigned long pagesize = 0;
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
volatile unsigned long interrupter_1 = 0;

TRB event_ring_queue[20] __attribute__ ((aligned (0x100))); 
TRB command_ring_control[20] __attribute__ ((aligned (0x100)));
unsigned long command_ring_offset = 0;
unsigned long event_ring_offset = 0;
unsigned char uses_context_64 = 0;
unsigned long btc[20] __attribute__ ((aligned (0x100)));

/**
 * Update the event ring dequeue pointer
 * \param newaddress the new address for the dequeue pointer
*/
void xhci_update_event_ring_dequeue_pointer(unsigned long newaddress){
	unsigned long erdp_addr = rtsoff + 0x038;
	((unsigned long*)erdp_addr)[0] = newaddress; // set addr of event ring dequeue pointer register
	((unsigned long*)erdp_addr)[1] = 0; // clear 64bit
}

/**
 * Wait untill Bussy bit is not set
 * \returns 0 on fail, 1 on success
 */
int xhci_wait_for_ready(){

	//
	// we wait untill the status of USBSTS is ready
	// or untill a timeout toke place

	//
	// We are not ready: yet
	resetTicks();
	while(1){
		if(getTicks()>10){ // wait max 10 seconds
			return 0; // mark as failed
		}
		if((((unsigned char*)usbsts)[0]&0x800)==0){
			break;
		}
	}

	//
	// We are ready!
	return 1;
}

/**
 * Get the cycle bit which should be used
 * \returns the cyclebit which must be used
 */
unsigned char getCycleBit(){
	return deviceid!=XHCI_DEVICE_QEMU?0:1;
}

/**
 * Prints the contents of the event ring to the screen
 */
void xhci_dump_event_ring(){
	int local_event_ring_offset = 0;
	while(1){
		TRB* trbres = ((TRB*)((unsigned long)(&event_ring_queue)+local_event_ring_offset));
		unsigned long completioncode = (trbres->bar3 & 0b111111100000000000000000000000) >> 24;
		unsigned long trbtype = (trbres->bar4 & 0b000000000000001111110000000000) >> 10;
		if(completioncode==0){
			break;
		}
		if(trbtype==0x22){
			printf("[XHCI] Port change event\n");
		}else if(trbtype==0x21){
			printf("[XHCI] Command completion event\n");
		}else{
			printf("[XHCI] Unknown TRB_type=%x \n",trbtype);
		}
		local_event_ring_offset += 0x10;
	}
}

/**
 * Find the offset to the first unused event slot
 * \returns offset first free event slot
 */
int xhci_seek_end_event_queue(){
	event_ring_offset = 0;
	int eventcount = 0;
	while(1){
		TRB* trbres2 = ((TRB*)((unsigned long)(&event_ring_queue)+event_ring_offset));
		unsigned long completioncode2 = (trbres2->bar3 & 0b111111100000000000000000000000) >> 24;
		if(completioncode2==0){
			break;
		}
		event_ring_offset += 0x10;
		eventcount++;
	}
	printf("[XHCI] Event Ring Control: pointing at event %x \n",eventcount);
	return eventcount;
}

/**
 * Get last event as trb
 * \returns last trb of event
 */
TRB* xhci_get_last_event(){
	int t_event_ring_offset = 0;
	TRB* trbres2 = 0;
	TRB* trbres3 = 0;
	while(1){
		trbres2 = ((TRB*)((unsigned long)(&event_ring_queue)+t_event_ring_offset));
		unsigned long completioncode2 = (trbres2->bar3 & 0b111111100000000000000000000000) >> 24;
		if(completioncode2==0){
			break;
		}
		trbres3 = trbres2;
		t_event_ring_offset += 0x10;
	}
	return trbres3;
}

/**
 * Wait untill XHCI event queue reaches target
 * \param target the target to reach
 */
int xhci_wait(int target){
	int timeout = 0;
	while(1){
		sleep(10);
		if(xhci_seek_end_event_queue()==target){
			break;
		}
		timeout++;
		if(timeout>5){
			printf("[XHCI] Timeout while waiting!\n");
			return 0;
		}
	}
	return 1;
}

/**
 * Ring doorbell and wait untill event is popped on
 * \param number the doorbel number
 * \param callnum the endpoint to call
 * \returns 0 on failure, 1 on success
 */
int xhci_ring_and_wait(int number,int callnum){
	int stot = xhci_seek_end_event_queue();
	((volatile unsigned long*)&interrupter_1)[0] = 0;
	
	// doorbell
	((unsigned long*)doorbel)[number] = callnum;
	sleep(100);
	
	// wait
	int timeout = 0;
	while(1){
		volatile unsigned long r = ((volatile unsigned long*)iman_addr)[0];
		if(r&1){
			break;
		}
		if(((volatile unsigned long*)&interrupter_1)[0]==0xCD){
			break;
		}
		sleep(100);
		timeout++;
		if(timeout==5){
			printf("[XHCI] wait loop 1 timeout\n");
			return 0;
		}
	}
	((volatile unsigned long*)&interrupter_1)[0] = 0;
	stot = stot + 1;
	if(xhci_wait(stot)==0){
		printf("[XHCI] wait loop 2 timeout\n");
		return 0;
	}
	
	
	// RESULTS
	volatile TRB *trbres2 = xhci_get_last_event();
	unsigned long completioncode2 = (trbres2->bar3 & 0b111111100000000000000000000000) >> 24;
	
	event_ring_offset += 0x10;
	xhci_update_event_ring_dequeue_pointer((unsigned long)trbres2);
	return completioncode2;
}

/**
 * Creates a ending trb for this event
 * \param out trb to be used
 */
void xhci_stop_codon_to_trb(TRB *out){
	out->bar1 = 0;
	out->bar2 = 0;
	out->bar3 = 0;
	out->bar4 = getCycleBit()==1?0:1;
}

/**
 * Installs enable_slot on trb
 * \param in the enable_slot object
 * \param out the trb object
 */
void xhci_trb_enable_slot_to_trb(XHCI_TRB_ENABLE_SLOT in,TRB *out){
	out->bar1 = 0;
	out->bar2 = 0;
	out->bar3 = 0;
	out->bar4 = 0;
	
	out->bar4 =  XHCI_TRB_ENABLE_SLOT_SLOTTYPE(in.slottype) | XHCI_TRB_SET_TRB_TYPE(in.trbtype) | XHCI_TRB_SET_CYCLE_BIT(in.cyclebit);
}

/**
 * Installs the disable slot object to a trb
 * \param in the disable_slot object
 * \param out the trb object to install to
 */
void xhci_trb_disable_slot_to_trb(XHCI_TRB_DISABLE_SLOT in,TRB *out){
	out->bar1 = 0;
	out->bar2 = 0;
	out->bar3 = 0;
	out->bar4 = 0;
	
	out->bar4 =  XHCI_TRB_SET_SLOT(in.slot_id) | XHCI_TRB_SET_TRB_TYPE(in.trb_type) | XHCI_TRB_SET_CYCLE_BIT(in.cycle_bit);
}

/**
 * Installs set address to trb
 * \param in set_address object
 * \param out trb
*/
void xhci_trb_set_address_to_trb(XHCI_TRB_SET_ADDRESS in,TRB *out){
	out->bar1 = 0;
	out->bar2 = 0;
	out->bar3 = 0;
	out->bar4 = 0;
	
	out->bar1 = XHCI_TRB_SET_ADDRESS_POINTER(in.input_context);
	out->bar4 = XHCI_TRB_SET_SLOT(in.slotid) | XHCI_TRB_SET_TRB_TYPE(in.trbtype) | XHCI_TRB_SET_ADDRESS_BSR(in.bsr) | XHCI_TRB_SET_CYCLE_BIT(in.cyclebit);
}

/**
 * Installs control context to trb
 * \param in control context
 * \param out trb
*/
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

/**
 * Installs slot context to trb
 * \param in slot context
 * \param out trb
*/
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

/**
 * Installs endpointcontext to trb
 * \param in the endpointcontext
 * \param out the trb
*/
void xhci_endpoint_context_to_addr(XHCI_ENDPOINT_CONTEXT *in, unsigned long *out){
	out[0] = 0;
	out[1] = 0;
	out[2] = 0;
	out[3] = 0;
	out[4] = 0;
	out[5] = 0;
	out[6] = 0;
	out[7] = 0;
	
	out[0] = XHCI_ENDPOINT_CONTEXT_MAX_ENDPOINT_SERVICE(in->maxexitpayloadhigh) | XHCI_ENDPOINT_CONTEXT_INTERVAL(in->interval) | XHCI_ENDPOINT_CONTEXT_LINEAR_STREAM_ARRAY(in->lsa) | XHCI_ENDPOINT_CONTEXT_MAX_PRIMAIRY_STREAMS(in->maxpstreams) | XHCI_ENDPOINT_CONTEXT_MULT(in->mult) | XHCI_ENDPOINT_CONTEXT_ENDPOINT_STATE(in->endpoint_state);
	out[1] = XHCI_ENDPOINT_CONTEXT_MAX_PACKET_SIZE(in->maxpacketsize) | XHCI_ENDPOINT_CONTEXT_MAX_BURST_SIZE(in->maxburstsize) | XHCI_ENDPOINT_CONTEXT_HOST_INITIATE_DISABLE(in->hid) | XHCI_ENDPOINT_CONTEXT_ENDPOINT_TYPE(in->endpointtype) | XHCI_ENDPOINT_CONTEXT_ERR_CNT(in->cerr);
	out[2] = XHCI_ENDPOINT_CONTEXT_DP(in->dequeuepointer) | XHCI_ENDPOINT_CONTEXT_DSC(in->dcs);
	out[4] = XHCI_ENDPOINT_CONTEXT_MAX_ENDPOINT_SERVICE_LO(in->maxpayloadlow) | XHCI_ENDPOINT_CONTEXT_AVG_TRB_LENGTH(in->average_trb_length);
	
}

/**
 * Set address data to specified port
 * \param assignedSloth the slot to assign the address to
 * \param t the data to assign to the slot
 * \param bsr if the request is bsr
 * \return 1 for success
*/
int xhci_set_address(unsigned long assignedSloth,unsigned long* t,unsigned char bsr){
	// Address Device Command BSR1
	TRB* trb = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	XHCI_TRB_SET_ADDRESS set_address;
	set_address.input_context = (unsigned long) t;
	set_address.cyclebit = getCycleBit();
	set_address.bsr = bsr;
	set_address.trbtype = 11;
	set_address.slotid = assignedSloth;
	xhci_trb_set_address_to_trb(set_address,trb);
	
	command_ring_offset += 0x10;
	
	// stop codon
	TRB *trb6 = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	xhci_stop_codon_to_trb(trb6);
	command_ring_offset += 0x10;
	return xhci_ring_and_wait(0,0);
}

/**
 * Disable specified slot
 * \param assignedSloth the slot to disable
 * \returns 1 for success
*/
unsigned int xhci_disable_slot(unsigned long assignedSloth){
	xhci_seek_end_event_queue();
	TRB* trb2 = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	XHCI_TRB_DISABLE_SLOT disable_slot;
	disable_slot.cycle_bit = getCycleBit();
	disable_slot.trb_type = 10;
	disable_slot.slot_id = assignedSloth;
	xhci_trb_disable_slot_to_trb(disable_slot,trb2);
	
	command_ring_offset += 0x10;
	
	TRB* trb = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	xhci_stop_codon_to_trb(trb);
	
	return xhci_ring_and_wait(0,0);
}

/**
 * Issue enable slot command
 * \returns the portnumber which is assigned
*/
int xhci_enable_slot(){
	unsigned char cs = deviceid==XHCI_DEVICE_BOCHS||deviceid==XHCI_DEVICE_QEMU?1:0;
	int stot = xhci_seek_end_event_queue();
	TRB* trb2 = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	trb2->bar1 = 0;
	trb2->bar2 = 0;
	trb2->bar3 = 0;
	trb2->bar4 = (9<<10) | cs; // 9 for enable slot command
	trb2->bar4 |= (1<<16); // slottype 1
	
	command_ring_offset += 0x10;
	
	TRB* trb = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	trb->bar1 = 0;
	trb->bar2 = 0;
	trb->bar3 = 0;
	trb->bar4 = (cs==1?0:1);
	
	((volatile unsigned long*)&interrupter_1)[0] = 0;
	xhci_wait_for_ready();
	((unsigned long*)doorbel)[0] = 0;
	sleep(10);
	while(1){
		volatile unsigned long r = ((volatile unsigned long*)iman_addr)[0];
		if(r&1){
			break;
		}
		if(((volatile unsigned long*)&interrupter_1)[0]==0xCD){
			break;
		}
	}
	((volatile unsigned long*)&interrupter_1)[0] = 0;
	sleep(10);
	stot = stot + 1;
	xhci_wait(stot);
	
	volatile TRB *trbres = xhci_get_last_event();
	volatile unsigned char completioncode = (trbres->bar3 & 0b111111100000000000000000000000) >> 24;
	volatile unsigned char assignedSloth = (trbres->bar4 & 0b111111100000000000000000000000) >> 24;
	event_ring_offset += 0x10;
	if(completioncode!=1){
		return -1;
	}
	return assignedSloth;
}

/**
 * Issue no-operation command
 * \returns 1 when succeed
*/
int xhci_noop(){
	unsigned char cs = deviceid==XHCI_DEVICE_BOCHS||deviceid==XHCI_DEVICE_QEMU?1:0;
	xhci_seek_end_event_queue();
	TRB* trb2 = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	trb2->bar1 = 0;
	trb2->bar2 = 0;
	trb2->bar3 = 0;
	trb2->bar4 = (23<<10) | cs;
	
	command_ring_offset += 0x10;
	
	TRB* trb = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	trb->bar1 = 0;
	trb->bar2 = 0;
	trb->bar3 = 0;
	trb->bar4 = (cs==1?0:1);

	return xhci_ring_and_wait(0,0);
}

/**
 * Handle interrupt
*/
void irq_xhci(){
	unsigned long xhci_usbsts = ((unsigned long*)usbsts)[0];
	if(xhci_usbsts&4){
		printf("[XHCI] Host system error interrupt\n");
	}else if(xhci_usbsts&8){
		printf("[XHCI] Event interrupt\n");
	}else if(xhci_usbsts&0x10){
		printf("[XHCI] Port interrupt\n");
	}else{
		printf("[XHCI] Unknown interrupt\n");
	}
	((volatile unsigned long*)&interrupter_1)[0] = 0xCD;

	// acknowledge packages
	((unsigned long*)usbsts)[0] = xhci_usbsts;
	
	//	((unsigned long*)usbsts)[0] |= 0x8;
	//	((unsigned long*)usbsts)[0] |= 0b10000;
	unsigned long iman_addr = rtsoff + 0x020;
	((unsigned long*)iman_addr)[0] |= 0b11;
	outportb(0xA0,0x20);
	outportb(0x20,0x20);
}

/**
 * Send trb objects to port
 * \param device the device to send to
 * \param setup the setup trb
 * \param data the data trb
 * \param end the status trb
 * \returns the return status of te event
 */
unsigned char xhci_send_message(USB_DEVICE* device,TRB setup,TRB data,TRB end){
	xhci_seek_end_event_queue();
	TRB *dc1 = ((TRB*)((unsigned long)(device->localring)+device->localringoffset));
	dc1->bar1 = setup.bar1;
	dc1->bar2 = setup.bar2;
	dc1->bar3 = setup.bar3;
	dc1->bar4 = setup.bar4;
	device->localringoffset += 0x10;
	
	if(!(data.bar1==0&&data.bar2==0&&data.bar3==0&&data.bar4==0)){
		TRB *dc2 = ((TRB*)((unsigned long)(device->localring)+device->localringoffset));
		dc2->bar1 = data.bar1;
		dc2->bar2 = data.bar2;
		dc2->bar3 = data.bar3;
		dc2->bar4 = data.bar4;
		device->localringoffset+=0x10;
	}
	
	TRB *dc3 = ((TRB*)((unsigned long)(device->localring)+device->localringoffset));
	dc3->bar1 = end.bar1;
	dc3->bar2 = end.bar2;
	dc3->bar3 = end.bar3;
	dc3->bar4 = end.bar4;
	device->localringoffset+=0x10;
	((volatile unsigned long*)&interrupter_1)[0] = 0;
	
	((unsigned long*)doorbel)[device->assignedSloth] = 1;
	
	while(1){
		unsigned long r = ((unsigned long*)iman_addr)[0];
		if(r&1){
			break;
		}
		if(((volatile unsigned long*)&interrupter_1)[0]==0xCD){
			break;
		}
	}
	((volatile unsigned long*)&interrupter_1)[0] = 0;
	
	TRB *trbres = ((TRB*)((unsigned long)(event_ring_queue)+event_ring_offset));
	unsigned char completioncode = (trbres->bar3 & 0b111111100000000000000000000000) >> 24;
	event_ring_offset += 0x10;
	
	return completioncode;
}

unsigned char xhci_dump_endpoint_context(unsigned long *statecontext){
	printf(" -----------------------------------------------------------\n");
	printf("Endpointstate        : %x   ",(statecontext[0]&0b00000000000000000000000000000111)>>0);
	printf("Mult                 : %x \n",(statecontext[0]&0b00000000000000000000001100000000)>>8);
	printf("MaxPStreams          : %x   ",(statecontext[0]&0b00000000000000000111110000000000)>>10);
	printf("LSA                  : %x \n",(statecontext[0]&0b00000000000000001000000000000000)>>15);
	printf("Interval             : %x   ",(statecontext[0]&0b00000000111111110000000000000000)>>16);
	printf("Max ESIT Payload Hi  : %x \n",(statecontext[0]&0b11111111000000000000000000000000)>>24);
	printf("CErr                 : %x   ",(statecontext[1]&0b00000000000000000000000000000110)>>1);
	printf("EPType               : %x \n",(statecontext[1]&0b00000000000000000000000000111000)>>3);
	printf("HID                  : %x   ",(statecontext[1]&0b00000000000000000000000010000000)>>7);
	printf("Max Burst Size       : %x \n",(statecontext[1]&0b00000000000000001111111100000000)>>8);
	printf("Max Packet Size	     : %x  ",(statecontext[1]&0b11111111111111110000000000000000)>>16);
	printf("DCS                  : %x \n",(statecontext[2]&0b00000000000000000000000000000001)>>0);
	printf("TR Dequeuepointer    : %x %x \n",(statecontext[2]&0b11111111111111111111111111110000)>>0,statecontext[3]);
	printf("Average TRB Length   : %x   ",(statecontext[4]&0b00000000000000001111111111111111)>>0);
	printf("Max ESIT Payload Lo  : %x \n",(statecontext[4]&0b11111111111111110000000000000000)>>0);
	printf(" -----------------------------------------------------------\n");
	return statecontext[0] & 0b00000000000000000000000000000111;
}

/**
 * Print object according to the standard
 * \param statecontext the location where the object is
 * \returns the state of the port
*/
unsigned char xhci_dump_port_info_raw(unsigned long *statecontext){
	unsigned char slotstate = statecontext[3]>>27;
	printf("------------------------------------------------------------\n");
	printf("Routestring              : %x   ",(statecontext[0]&0b00000000000011111111111111111111));
	printf("Speed                    : %x \n",(statecontext[0]&0b00000000111100000000000000000000)>>20);
	printf("Multi-TT                 : %x   ",(statecontext[0]&0b00000010000000000000000000000000)>>25);
	printf("Hub                      : %x \n",(statecontext[0]&0b00000100000000000000000000000000)>>26);
	printf("Context entries          : %x   ",(statecontext[0]&0b11111000000000000000000000000000)>>27);
	printf("Max Exit Latancy         : %x \n",(statecontext[1]&0b00000000000000001111111111111111));
	printf("Root Hub Port Number     : %x   ",(statecontext[1]&0b00000000111111110000000000000000)>>16);
	printf("Number of Ports          : %x \n",(statecontext[1]&0b11111111000000000000000000000000)>>24);
	printf("Parent Hub Slot Id       : %x   ",(statecontext[2]&0b00000000000000000000000011111111));
	printf("Parent Hub Port Number   : %x \n",(statecontext[2]&0b00000000000000001111111100000000)>>8);
	printf("TT Think time            : %x   ",(statecontext[2]&0b00000000000000110000000000000000)>>16);
	printf("Interrupter Target       : %x \n",(statecontext[2]&0b11111111110000000000000000000000)>>22);
	printf("USB Device Address       : %x   ",(statecontext[3]&0b00000000000000000000000011111111));
	printf("Slot State               : %x \n",(statecontext[3]&0b11111000000000000000000000000000)>>27);
	printf("------------------------------------------------------------\n");
	return slotstate;
}

/**
 * Dump port info by assigned slot
 * \param assignedSloth the slot to check
 * \returns the state of the port
 */
unsigned char xhci_dump_port_info(unsigned char assignedSloth){
	unsigned long *statecontext = (unsigned long*)(btc[(assignedSloth*2)+0]);
	return xhci_dump_port_info_raw(statecontext);
}

/**
 * Install port
 * \param i physical port to install
 */
void xhci_probe_port(int i){
	unsigned long map = basebar + 0x400 + (i*0x10);
		unsigned long val = ((unsigned long*)map)[0];
		if(val&3){ // USB 3.0 does everything themselves
			printf("[XHCI] Port %x : Port has a USB3.0 connection (%x)\n",i,val);
		}else{ // USB 2.0 however doesnt...
			((unsigned long*)map)[0] = 0b1000010000; // activate power and reset port
			resetTicks();
			while(getTicks()<5);
			val = ((unsigned long*)map)[0];
			if(val&3){
				printf("[XHCI] Port %x : Port has a USB2.0 connection (%x)\n",i,val);
			}
		}
		
		
		if(val==0){
			return;
		}
		
		if(val&3){
			printf("[XHCI] Port %x : Port is initialising....\n",i);
		
			// detecting speed....
			unsigned long speed = (val >> 10) & 0b000000000000000000000000000111;
			unsigned long devicespeed = 0;// 8 64 512
			if(speed==XHCI_SPEED_SUPER){
				printf("[XHCI] Port %x : Port is a superspeed port\n",i);
				devicespeed = 512;
			}else if(speed==XHCI_SPEED_HI){
				printf("[XHCI] Port %x : Port is a highspeed port\n",i);
				devicespeed = 64;
			}else if(speed==XHCI_SPEED_FULL){
				printf("[XHCI] Port %x : Port is a fullspeed port\n",i);
				devicespeed = 64;
			}else if(speed==XHCI_SPEED_LOW){
				printf("[XHCI] Port %x : Port is a lowspeed port\n",i);
				devicespeed = 8;
			}

			if(val & 0x20000){
				printf("[XHCI] Port %x : Status changed since last visit\n",i);
			}

			if(val & 0x40000000){
				printf("[XHCI] Port %x : Device is removable\n",i);
			}

			//
			// Sleep a little bit to make sure everyone is at the same point
			sleep(100);
			
			//
			//
			// Device Slot Assignment
			//
			//
			
			printf("[XHCI] Port %x : Obtaining device slot...\n",i);
			int assignedSloth = xhci_enable_slot();
			if(assignedSloth==-1){
				printf("[XHCI] Port %x : Assignation of device slot failed!\n",i);for(;;);
				goto disabledevice;
			}
			printf("[XHCI] Port %x : Device device slot: %x \n",i,assignedSloth);

			// seabios updates the state of the port...Port Link State (PLS)
			unsigned long plsupdate1 = ((unsigned long*)map)[0]&0xFFFFFF00;
			((unsigned long*)map)[0] = plsupdate1 + 0x21;
			
			//
			//
			// Device Slot Initialisation
			//
			//
			
			printf("[XHCI] Port %x : Device Slot Initialisation BSR1 \n",i);
			
			TRB *local_ring_control = (TRB*)malloc_align(sizeof(TRB)*20,0xFF);//[20] __attribute__ ((aligned (0x100)));
			printf("[XHCI] Port %x : Local ring at %x \n",i,local_ring_control);
			unsigned char offsetA = uses_context_64==0?32:64;
			unsigned char offsetB = uses_context_64==0?64:128;
			
			printf("[XHCI] Port %x : Setting up input controll\n",i);
			unsigned long t[0x60] __attribute__ ((aligned(0x1000)));
	
			printf("[XHCI] Port %x : Setting up DCBAAP for port \n",i);
			unsigned long bse = (unsigned long)(((unsigned long)&t)+offsetA);//malloc_align(0x420,0xFF);//malloc(0x420);
			btc[(assignedSloth*2)+0] 	= bse;
			btc[(assignedSloth*2)+1] 	= 0;
			printf("[XHCI] Port %x : BCPAAP for port at %x \n",i,bse);
			
			printf("[XHCI] Port %x : Setting up Input Controll Context at %x \n",i,&t);
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
			slot_context.route_string = 0;
			slot_context.root_hub_port_number = (i+1);
			slot_context.context_entries = 1;
			slot_context.speed = speed;
			slot_context.number_of_ports = 0;
			unsigned long kappa = ((unsigned long)&t)+offsetA;
			xhci_slot_context_to_addr(slot_context,(unsigned long*)(kappa)); 
			
			printf("[XHCI] Port %x : Setting up Endpoint Context \n",i);
			// Endpoint Context
			XHCI_ENDPOINT_CONTEXT* endpoint_context = (XHCI_ENDPOINT_CONTEXT*)malloc(sizeof(XHCI_ENDPOINT_CONTEXT));
			endpoint_context->endpoint_state = 0;
			endpoint_context->endpointtype = 4;
			endpoint_context->maxpstreams = 0;
			endpoint_context->mult = 0;
			endpoint_context->cerr = 3;
			endpoint_context->maxburstsize = 0;
			endpoint_context->maxpacketsize = devicespeed;
			endpoint_context->interval = 0;
			endpoint_context->dequeuepointer = (unsigned long)local_ring_control;
			endpoint_context->dcs = 1;
			unsigned long sigma = ((unsigned long)&t)+offsetB;
			xhci_endpoint_context_to_addr(endpoint_context,(unsigned long*)(sigma));
			printf("[XHCI] Port %x : local transfer ring points to %x \n",i,(unsigned long)local_ring_control);
			
			//
			//
			// SETADDRESS commando
			//
			//
			
			printf("[XHCI] Port %x : Obtaining SETADDRESS(BSR=1)\n",i);
			int sares = xhci_set_address(assignedSloth,(unsigned long*)&t,1);
			if(sares==5){
				printf("[XHCI] Port %x : Local ring not defined as it should be....\n",i);for(;;);
				goto disabledevice;
			}else if(sares!=1){
				printf("[XHCI] Port %x : Assignation of device slot address failed with %x !\n",i,sares);for(;;);
				goto disabledevice;
			}
			
			USB_DEVICE* device = (USB_DEVICE*) malloc(sizeof(USB_DEVICE));
			device->drivertype = 3;
			device->portnumber = i;
			device->localring = (unsigned long)local_ring_control;
			device->localringoffset = 0;
			device->sendMessage = (unsigned long)&xhci_send_message;
			device->assignedSloth = assignedSloth;
			
			//
			// check if the bit is set by default or not
			TRB *checkbit = ((TRB*)((unsigned long)(device->localring)+device->localringoffset));
			unsigned char checkbitset = checkbit->bar4&1;
			if(checkbitset){
				printf("[XHCI] Port %x : Transferring is cyclebit is set by default\n",device->portnumber);
			}else{
				printf("[XHCI] Port %x : Transferring is cyclebit is not set by default\n",device->portnumber);
			}
			
			//
			// and see if this is everywhere the case
			for(int i = 0 ; i < 10 ; i++){
				TRB *checkbit2 = ((TRB*)((unsigned long)(device->localring)+(i*0x10)));
				if(checkbitset!=(checkbit2->bar4&1)){
					printf("[XHCI] empty localring is not empty at all!\n");for(;;);
					goto disabledevice;
				}
			}

			sleep(100);
			// 8400000 10000 0 8000000
			unsigned char slotstate = xhci_dump_port_info(assignedSloth);
			printf("[XHCI] Port %x : Slot state should be 1 and is %x \n",device->portnumber,slotstate);
			if(slotstate!=1){
				printf("[XHCI] Unexpected slotstate!\n");for(;;);
				goto disabledevice;
			}

			unsigned long *yu = (unsigned long*)(((unsigned long)&t)+offsetB);
			unsigned char endpointstate = xhci_dump_endpoint_context(yu);
			if(endpointstate!=1){
				printf("[XHCI] Unexpected endpointstate!\n");for(;;);
				goto disabledevice;
			}

			//
			// and test
			if(0){
				printf("[XHCI] Port %x : NOOP ring control\n",device->portnumber);
				((volatile unsigned long*)&interrupter_1)[0] = 0;
				TRB *trbx1 = ((TRB*)((unsigned long)(device->localring)+device->localringoffset));
				trbx1->bar1 = 0;
				trbx1->bar2 = 0;
				trbx1->bar3 = 0;
				trbx1->bar4 = 1 | (8<<10);//8
				device->localringoffset += 0x10;

				TRB *trbx2 = ((TRB*)((unsigned long)(device->localring)+device->localringoffset));
				trbx2->bar1 = 0;
				trbx2->bar2 = 0;
				trbx2->bar3 = 0;
				trbx2->bar4 = 1 | (4<<10) | 0x20 | (1 << 16);
				device->localringoffset += 0x10;
				// stop codon
				TRB *trb6 = ((TRB*)((unsigned long)(device->localring)+device->localringoffset));
				xhci_stop_codon_to_trb(trb6);
				device->localringoffset += 0x10;
				int tres = xhci_ring_and_wait(assignedSloth,1);
				event_ring_offset += 0x10;
				if(tres!=1){
					printf("[XHCI] Port %x : Could not NOOP on ring %x !\n",device->portnumber,tres);
					for(;;);
					return;
				}
				printf("[XHCI] Port %x : It is working!\n",device->portnumber);
				for(;;);
			}
			
			printf("[XHCI] Port %x : GET DEVICE DESCRIPTOR\n",device->portnumber);
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
			
			unsigned char devicedescriptor[12];
			((volatile unsigned long*)&interrupter_1)[0] = 0;
			volatile TRB *dc1 = ((volatile TRB*)((volatile unsigned long)(device->localring)+device->localringoffset));
			dc1->bar1 = 0;
			dc1->bar2 = 0;
			dc1->bar3 = 0;
			dc1->bar4 = 0;
			
			dc1->bar1 |= 0x80; // reqtype=0x80
			dc1->bar1 |= (0x06<<8); // req=6
			dc1->bar1 |= (0x100 << 16); // wValue = 0100
			dc1->bar2 |= 0; // windex=0
			dc1->bar2 |= (8 << 16); // wlength=0 // 0x80000
			dc1->bar3 |= 8; // trbtransferlength
			dc1->bar3 |= (0 << 22); // interrupetertrager
			dc1->bar4 |= 1; // cyclebit =1
			dc1->bar4 |= (0<<5); // ioc=0
			dc1->bar4 |= (1<<6); // idt=1
			dc1->bar4 |= (2<<10); // trbtype
			dc1->bar4 |= (3<<16); // trt = 3;
			dc1->bar4 |= 0x40;
			device->localringoffset += 0x10;
			
			// single date stage
			// TRB Type = Data Stage TRB.
			// X Direction (DIR) = ‘1’.
			// X TRB Transfer Length = 8.
			// X Chain bit (CH) = 0.
			// X Interrupt On Completion (IOC) = 0.
			// X Immediate Data (IDT) = 0.
			// X Data Buffer Pointer = The address of the Device Descriptor receive buffer.
			// X Cycle bit = Current Producer Cycle State.
			volatile TRB *dc2 = ((volatile TRB*)((volatile unsigned long)(device->localring)+device->localringoffset));
			dc2->bar1 = (unsigned long)&devicedescriptor;
			dc2->bar2 = 0b00000000000000000000000000000000;
			dc2->bar3 = 0b00000000000000000000000000001000;
			dc2->bar4 = 1 | 0b00000000000000010000110000000000; // 0b00000000000000010000110000000001
			device->localringoffset+=0x10;
			
			volatile TRB *dc3 = ((volatile TRB*)((volatile unsigned long)(device->localring)+device->localringoffset));
			dc3->bar1 = 0;
			dc3->bar2 = 0;
			dc3->bar3 = 0;
			dc3->bar4 = 0b1000000100001;//1 | (4<<10) | 0x20 | (1 << 16); // 1 | (4<<10) | 0x20 | (1 << 16);
			device->localringoffset+=0x10;

			unsigned char completioncode = xhci_ring_and_wait(assignedSloth,1);
			printf("[XHCI] Port %x : complection code is %x \n",device->portnumber,completioncode);
			if(completioncode!=1){
				printf("[XHCI] Port %x : completioncode is not 1 but %x\n",device->portnumber,completioncode);for(;;);
				goto disabledevice;
			}
			event_ring_offset += 0x10;
			
			unsigned char* sigma2 = (unsigned char*)&devicedescriptor;
			unsigned long maxpackagesize = 0;
			if(!(sigma2[0]==0x12&&sigma2[1]==1)){
				printf("[XHCI] Port %x : Deviceclass cannot be 0! \n",device->portnumber);for(;;);
				goto disabledevice;
			}
			if(sigma2[7]==8||sigma2[7]==16||sigma2[7]==32||sigma2[7]==64){
				maxpackagesize = sigma2[7];
			}else{
				maxpackagesize = pow(2,sigma2[7]);
			}
			printf("[XHCI] Port %x : Device with maxpackagesize %x \n",device->portnumber,maxpackagesize);
			
			// reset the device again....
			printf("[XHCI] Port %x : Second reset port\n",device->portnumber);
			((unsigned long*)map)[0] = 0b1000010000; // activate power and reset port
			resetTicks();
			while(getTicks()<5);
			val = ((unsigned long*)map)[0];
			if((val&3)==0){
				printf("[XHCI] Port %x : Second reset failed\n",device->portnumber);for(;;);
				goto disabledevice;
			}
			sleep(100);
			device->localringoffset=0;
			
			// set address with BSR=0
			printf("[XHCI] Port %x : Obtaining SETADDRESS(BSR=0)\n",device->portnumber);
			endpoint_context->maxpacketsize = maxpackagesize;
			sigma = ((unsigned long)&t)+offsetB;
			xhci_endpoint_context_to_addr(endpoint_context,(unsigned long*)(sigma));
			slot_context.usb_device_address = 1;
			xhci_slot_context_to_addr(slot_context,(unsigned long*)(kappa)); 

			sares = xhci_set_address(assignedSloth,(unsigned long*)&t,0);
			if(sares==5){
				printf("[XHCI] Port %x : Local ring not defined as it should be....\n",device->portnumber);for(;;);
				goto disabledevice;
			}else if(sares!=1){
				printf("[XHCI] Port %x : Assignation of device slot address failed with %x !\n",device->portnumber,sares);for(;;);
				goto disabledevice;
			}
			printf("[XHCI] Port %x : Second portreset ended succesfully\n",device->portnumber);for(;;);
			
			unsigned char descz = 8;
			printf("[XHCI] Port %x : Getting %x bytes of device descriptor\n",device->portnumber,descz);
			
			XHCI_DEVICE_DESCRIPTOR *xdd = (XHCI_DEVICE_DESCRIPTOR*) sigma2;
			printf("[XHCI] Port %x : device descriptor deviceclass=%x \n",device->portnumber,xdd->bDeviceClass);
			unsigned char deviceclass = xdd->bDeviceClass;
			unsigned char devsubclass = xdd->bDeviceSubClass;
			unsigned char devprotocol = xdd->bDeviceProtocol;
			if(deviceclass==0x00){
				printf("[XHCI] Port %x : Deviceclass of devicedescriptor is NULL\n",device->portnumber);
				
				//
				// Demand configuration data...
				unsigned char deviceconfig[0x15];
				xhci_seek_end_event_queue();
				((volatile unsigned long*)&interrupter_1)[0] = 0;
				TRB *dc4 = ((TRB*)((unsigned long)(device->localring)+device->localringoffset));
				dc4->bar1 = 0;
				dc4->bar2 = 0;
				dc4->bar3 = 0;
				dc4->bar4 = 0;
				
				dc4->bar1 |= 0x80; // reqtype=0x80
				dc4->bar1 |= (0x06<<8); // req=6
				dc4->bar1 |= (0x200 << 16); // wValue = 0100
				dc4->bar2 |= 0; // windex=0
				dc4->bar2 |= (0x15 << 16); // 8 0x15 wlength=0 // 0x80000
				dc4->bar3 |= 8; // trbtransferlength
				dc4->bar3 |= (0 << 22); // interrupetertrager
				dc4->bar4 |= 1; // cyclebit
				dc4->bar4 |= (00<<5); // ioc=0
				dc4->bar4 |= (1<<6); // idt=1
				dc4->bar4 |= (2<<10); // trbtype
				dc4->bar4 |= (3<<16); // trt = 3;
				device->localringoffset += 0x10;
				
				// single date stage
				TRB *dc5 = ((TRB*)((unsigned long)(device->localring)+device->localringoffset));
				dc5->bar1 = (unsigned long)&deviceconfig;
				dc5->bar2 = 0b00000000000000000000000000000000;
				dc5->bar3 = 0x15;
				dc5->bar4 = 0b00000000000000010000110000000001;
				device->localringoffset+=0x10;
				
				TRB *dc6 = ((TRB*)((unsigned long)(device->localring)+device->localringoffset));
				dc6->bar1 = 0;
				dc6->bar2 = 0;
				dc6->bar3 = 0;
				dc6->bar4 = 1 | (4<<10) | 0x20 | (1 << 16);
				device->localringoffset+=0x10;
			
				((unsigned long*)doorbel)[assignedSloth] = 1;
				sleep(100);
				
				while(1){
					unsigned long r = ((unsigned long*)iman_addr)[0];
					if(r&1){
						break;
					}
					if(((volatile unsigned long*)&interrupter_1)[0]==0xCD){
						break;
					}
				}
				
				sleep(100);
				((volatile unsigned long*)&interrupter_1)[0] = 0;
				event_ring_offset += 0x10;
				
				unsigned char* sigma3 = (unsigned char*)&deviceconfig;
				printf("[XHCI] Port %x : There are %x interfaces supported by this device\n",device->portnumber,sigma3[4]);
				deviceclass = sigma3[8+6];
				devsubclass = sigma3[8+7];
				devprotocol = sigma3[8+8];
				printf("[XHCI] Port %x : Deviceclass = %x \n",device->portnumber,deviceclass);
			}
			
			device->class = deviceclass;
			device->subclass = devsubclass;
			device->protocol = devprotocol;
			
			if(device->class==0x00){
				printf("[XHCI] Port %x : Failed to detect deviceclass\n",device->portnumber);
				goto disabledevice;
			}else{ 
				
				//
				// Set config
				xhci_seek_end_event_queue();
				((volatile unsigned long*)&interrupter_1)[0] = 0;
				TRB *dc4 = ((TRB*)((unsigned long)(device->localring)+device->localringoffset));
				dc4->bar1 = 0x10900; 
				dc4->bar2 = 0;
				dc4->bar3 = 0x8;
				dc4->bar4 = 0x841;
				device->localringoffset += 0x10;
				
				TRB *dc5 = ((TRB*)((unsigned long)(device->localring)+device->localringoffset));
				dc5->bar1 = 0;
				dc5->bar2 = 0;
				dc5->bar3 = 0;
				dc5->bar4 = 1 | (4<<10) | 0x20 | (1 << 16);
				device->localringoffset+=0x10;
			
				((unsigned long*)doorbel)[assignedSloth] = 1;
				sleep(100);
				
				while(1){
					unsigned long r = ((unsigned long*)iman_addr)[0];
					if(r&1){
						break;
					}
					if(((volatile unsigned long*)&interrupter_1)[0]==0xCD){
						break;
					}
				}
				sleep(100);
				
				((volatile unsigned long*)&interrupter_1)[0] = 0;
				event_ring_offset += 0x10;
				
				usb_device_install(device);
			}
			
			printf("[XHCI] Port %x : Finished installing port\n",i);
			for(;;);
			return;
			
			disabledevice:
			printf("[XHCI] Port %x : Disabling port\n",i);
			xhci_disable_slot(assignedSloth);
			for(;;);
		}else{
			printf("[XHCI] Port %x : No device attached!\n",i);
		}
}

/**
 * Probe and install new ports
 */
void xhci_probe_ports(){
	printf("[XHCI] Probing ports....\n");
	//
	// First, see which ports are available
	// USB3.0 will say automatically he is there.
	// USB2.0 should be activated manually
	//
	// After this we should initialise it by getting its port number from the ring
	for(unsigned int i = 0 ; i < 10 ; i++){
		xhci_probe_port(i);
	}
}

/**
 * Installs XHCI. This is the main function
 * intell uses xhci offset 0x47c
 * \param bus bus to check it
 * \param slot slot to check 
 * \param function function to check
 */
void init_xhci(unsigned long bus,unsigned long slot,unsigned long function){
	printf("[XHCI] entering xhci driver....\n");
	#ifdef DISABLE_USB
	printf("[XHCI] XHCI disabled by programmer\n");
	return;
	#endif
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
	}else if(deviceid==XHCI_DEVICE_QEMU){
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
		printf("[XHCI] Incompatible device!\n");getch();
		return;
	}
	unsigned long hciversionaddr = bar+2;
	unsigned long hciversion = ((unsigned long*)hciversionaddr)[0];
	printf("[XHCI] hciversion %x \n",hciversion);
	unsigned long capdb = bar+0x14;
	unsigned long hciparamadr = bar+0x04;
	unsigned long hciparam1 = ((unsigned long*)hciparamadr)[0];
	unsigned long hciparamadr2 = bar+0x08;
	unsigned long hciparam2 = ((unsigned long*)hciparamadr2)[0];
	unsigned long portcount = hciparam1>>24;
	unsigned long hccparams1adr = bar+0x10;
	unsigned long hccparams1 = ((unsigned long*)hccparams1adr)[0];
	printf("[XHCI] hccparams1 %x \n",hccparams1);
	printf("[XHCI] hccparams2 %x \n",hciparam2);
	unsigned long maxscratchpad = (hciparam2 & 0xFBE00000)>>21;
	printf("[XHCI] Scratchpad %x \n",maxscratchpad);
	//unsigned char uses_bandwith_nego = 0;
	if(hccparams1&1){
		printf("[XHCI] has 64-bit Addressing Capability\n");
	}
	if(hccparams1&2){
		printf("[XHCI] has bandwith negociation Capability\n");
		//uses_bandwith_nego = 1;
	}
	if(hccparams1&4){
		printf("[XHCI] has 64bit context size datastructures Capability\n");
		uses_context_64 = 1;
	}
	if(hccparams1&0xFFFF0000){
		unsigned long extcappoint = bar+((hccparams1&0xFFFF0000)>>14);
		printf("[XHCI] has xHCI Extended Capabilities Pointer ( %x : %x )\n",extcappoint,(hccparams1&0xFFFF0000)>>14);
		int offsetx = 0;
		for(int i = 0 ; i < 20 ; i++){
			unsigned long tx = extcappoint+offsetx;
			unsigned long fault = ((unsigned long*)tx)[0];
			unsigned char capid = fault & 0x00FF;
			unsigned char capof = (fault & 0xFF00)>>8;
			if(capid==0){
				break;
			}
			printf("[XHCI] capid=%x capof=%x \n",capid,capof);
			if(capid==1){
				printf("[XHCI] Legacy extended capability found!\n");
				if(fault&0x1010000){
					printf("[XHCI] Owner specified!\n");
					if(fault&0x10000){
						printf("[XHCI] Currently, BIOS owns XHCI controller\n");
						printf("[XHCI] Grabbing the controlls\n");
						((unsigned long*)tx)[0] |= 0x1000000;
						tryagain:
						fault = ((volatile unsigned long*)tx)[0];
						if(fault&0x10000){
							goto tryagain;
						}
					}
					if(fault&0x1000000){
						printf("[XHCI] Currently, We own the XHCI controller\n");
					}
				}else{
					printf("[XHCI] No owner specified!\n");
				}
			}else if(capid==2){
				printf("[XHCI] Supported Protocol\n");
				unsigned long protname = ((unsigned long*)tx)[1];
				unsigned char nA = (protname & 0xFF000000) >> 24;
				unsigned char nB = (protname & 0x00FF0000) >> 16;
				unsigned char nC = (protname & 0x0000FF00) >> 8;
				unsigned char nD = (protname & 0x000000FF);
				printf("[XHCI] Protocolname [%c%c%c%c] \n",nD,nC,nB,nA);
				unsigned long protid = ((unsigned long*)tx)[3] & 0x0000000F;
				printf("[XHCI] Porttype [%x] \n",protid);
				if(protid!=0){
					printf("[XHCI] XHCI protocol not supported!\n");
				}
				unsigned char nE = (fault & 0xFF000000) >> 24;
				unsigned char nF = (fault & 0x00FF0000) >> 16;
				printf("[XHCI] Portprotocol [%x] [%x] \n",nE,nF);
				unsigned long portrange = ((unsigned long*)tx)[0];
				unsigned char portrangeoffset = portrange & 0x00FF;
				unsigned char portrangecount = (portrange & 0xFF00)>>8;
				printf("[XHCI] Portrange offset=%x count=%x \n",portrangeoffset,portrangecount);
			}
			offsetx += (capof*sizeof(unsigned long));
			if(capof==0){
				break;
			}
		}
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
	pagesize = basebar+0x08;
	config = basebar+0x38;
	bcbaap = basebar+0x30;
	crcr   = basebar+0x18;
	dnctrl = basebar+0x14;

	if(0){
		unsigned long adt = (((unsigned long*)bcbaap)[0]);
		unsigned long *adx = (unsigned long*) adt;
		unsigned long *statecontext = (unsigned long*)(adx[(1*2)+0]);
		if(statecontext==0){
			for(int i = 2 ; i < 10 ; i++){
				if(adx[i]!=0){
					printf("[XHCI] Using dev %x for dump\n",i/2);
					statecontext = adx[i];
					xhci_dump_port_info_raw(statecontext);
				}
				i++;
			}
		}else{
			xhci_dump_port_info_raw(statecontext);
		}
		for(;;);
	}
	
	printf("[XHCI] default value CONFIG %x \n",((unsigned long*)config)[0]);
	printf("[XHCI] default value BCBAAP %x \n",((unsigned long*)bcbaap)[0]);
	printf("[XHCI] default value CRCR   %x \n",((unsigned long*)crcr)[0]);
	
	unsigned long xhci_usbcmd = ((unsigned long*)usbcmd)[0];
	unsigned long xhci_usbsts = ((unsigned long*)usbsts)[0];
	unsigned long xhci_pagesize = ((unsigned long*)pagesize)[0];
	printf("[XHCI] pagesize %x defcmd %x defsts %x \n",xhci_pagesize,xhci_usbcmd,xhci_usbsts);
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
	xhci_wait_for_ready();//while(getTicks()<5);
	xhci_usbcmd = ((unsigned long*)usbcmd)[0];
	xhci_usbsts = ((unsigned long*)usbsts)[0];
	printf("[XHCI] Reset XHCI finished with USBCMD %x and USBSTS %x \n",xhci_usbcmd,xhci_usbsts);
	
	//
	// setting config
	((unsigned long*)config)[0] |= 2; // device slots to be used
	//
	// Setup default parameters
	// TELL XHCI TO REPORT EVERYTHING
	((unsigned long*)dnctrl)[0] |= 0b1111111111111111;
	
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
	xhci_update_event_ring_dequeue_pointer(((unsigned long)&event_ring_queue));
	
	// setting up "Event Ring Segment Table Base Address Register (ERSTBA)"
	printf("[XHCI] Setting up Event Ring Segment Table Base Address Register\n");
	unsigned long erstba_addr = rtsoff + 0x030;
	((unsigned long*)erstba_addr)[0] = (unsigned long)&rsb1; 	// table at 0x1000 for now
	((unsigned long*)erstba_addr)[1] = 0; 	// sending 0 to make sure...
	
	// setting up "Command Ring Control Register (CRCR)"
	printf("[XHCI] Setting up Command Ring Control Register\n");
	((unsigned long*)crcr)[0] |= ((unsigned long)&command_ring_control);
	((unsigned long*)crcr)[1] = 0;
	
	// DCBAAP
	printf("[XHCI] Setting up DCBAAP\n");
	((unsigned long*)bcbaap)[0] |= (unsigned long)&btc;
	((unsigned long*)bcbaap)[1] = 0;
	
	printf("[XHCI] Setting up scratchpad buffer \n");
	unsigned long* bse = (unsigned long*)malloc_align(maxscratchpad*sizeof(unsigned long),0xFF);
	for(unsigned int i = 0 ; i < 2 ; i++){
		unsigned long spbl = (unsigned long)malloc_align(1 << ((__builtin_ffs(xhci_pagesize) - 1) + 12),0xFFF);
		bse[i] = spbl;
	}
	btc[0] 	= (unsigned long)bse;
	btc[1] 	= 0;
	unsigned long iman_addr = rtsoff + 0x020;

	// setting first interrupt enabled.
	if(deviceid==XHCI_DEVICE_BOCHS||deviceid==XHCI_DEVICE_QEMU){
		printf("[XHCI] Setting up First Interrupter\n");
		((unsigned long*)iman_addr)[0] |= 0b11; // Interrupt Enable (IE) – RW
		sleep(50);
		printf("[XHCI] Use interrupts\n");
		((unsigned long*)usbcmd)[0] |= 4;
		sleep(50);
		// TELL XHCI TO USE INTERRUPTS
	}
	
	if(xhci_seek_end_event_queue()!=0){
		printf("[XHCI] PANIC: should be 0!\n");for(;;);
		return;
	}
	//
	// Start system
	printf("[XHCI] Run!\n");
	((unsigned long*)usbcmd)[0] |= 1;
	
	xhci_wait_for_ready();

	sleep(100);
	
	xhci_usbcmd = ((unsigned long*)usbcmd)[0];
	xhci_usbsts = ((unsigned long*)usbsts)[0];
	printf("[XHCI] System up and running with USBCMD %x and USBSTS %x \n",xhci_usbcmd,xhci_usbsts);

	int nope = xhci_noop();
	printf("[XHCI] nope status is %x \n",nope);
	if(nope==0){
		printf("[XHCI] something goes wrong with the event queue!\n");for(;;);
		return;
	}
	xhci_dump_event_ring();

	printf("[XHCI] Checking if portchange happened\n");
	if(xhci_usbsts & 0b10000){
		printf("[XHCI] Portchange detected!\n");
	}
	
	xhci_probe_ports();
	
	if(((unsigned long*)crcr)[0]==0x8){
		printf("[XHCI] circulair command ring is running\n");
	}
}
