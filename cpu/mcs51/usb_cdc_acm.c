#include <cc2511_map.h>
#include <cc2511_types.h>
#include <usb.h>
#include <usb_com.h>
#include <board.h>           // just for boardStartBootloader() and serialNumberString
#include <time.h>            // just for timing the start of the bootloader

// NOTE: We could easily remove the dependency on time.h if we added a
// function called startBootloaderSoon() in board.h that started the bootloader
// after some delay.

/* CDC ACM Library Configuration **********************************************/
// Note: USB 2.0 says that the maximum packet size for full-speed bulk endpoints
// can only be 8, 16, 32, or 64 bytes.
// We picked endpoint 4 for the data because it has a 256-byte FIFO memory area,
// which is exactly enough for us to have two 64-byte IN buffers and two 64-byte
// OUT buffers.

/*#define 64          64
#define 64           64
#define 0 0
#define 1    1

#define 1    1
#define USBF1        USBF1   // This must match 1!

#define 4            4
#define USBF4                USBF4   // This must match 4!
*/
/* CDC and ACM Constants ******************************************************/

// USB Class Codes
/*#define 2 2                  // (CDC 1.20 Section 4.1: Communications Device Class Code).
#define 0xA 0xA // (CDC 1.20 Section 4.5: Data Class Interface Codes).

// USB Subclass Codes
#define 2  2           // (CDC 1.20 Section 4.3: Communications Class Subclass Codes).  Refer to USBPSTN1.2.

// USB Protocol Codes
#define 1 1          // (CDC 1.20 Section 4.4: Communications Class Protocol Codes).

// USB Descriptor types from CDC 1.20 Section 5.2.3, Table 12
#define 0x24 0x24
#define 0x25  0x25

// USB Descriptor sub-types from CDC 1.20 Table 13: bDescriptor SubType in Communications Class Functional Descriptors
#define 0                       0
#define 1              1
#define 2  2
#define 6                        6

// Request Codes from CDC 1.20 Section 6.2: Management Element Requests.
#define 0 0
#define 1 1

// Request Codes from PSTN 1.20 Table 13.
#define 0x20 0x20
#define 0x21 0x21
#define 0x22 0x22

// Notification Codes from PSTN 1.20 Table 30.
#define 0x01 0x01
#define 0x20 0x20
*/
/* Private Prototypes *********************************************************/
//static void doNothing();

/* USB COM Variables **********************************************************/

// TODO: look at usb-to-serial adapters and figure out good default values for usbComControlLineState (RTS and CTS)
uint8 XDATA usbComControlLineState = 0;

uint8 XDATA usbComSerialState = 0;

// The last state we reported to the computer, or 0xFF if we have not reported
// a state yet.
//static uint8 lastReportedSerialState = 0xFF;
uint8 XDATA lastReportedSerialState = 0xFF; // lets keep this on the stack not on the direct accessable memory

ACM_LINE_CODING XDATA usbComLineCoding =
{
    9600,     // dwDTERate (baud rate)
    0,        // bCharFormat = 0: 1 stop bit
    0,        // bParityType = 0: no parity
    8,        // bDataBits = 8
};

//HandlerFunction * usbComLineCodingChangeHandler = doNothing;
HandlerFunction *usbComLineCodingChangeHandler = doNothing;
// This bit is true if we need to send an empty (zero-length) packet of data to
// the computer soon.  Every data transfer needs to be ended with a packet that
// is less than full length, so sometimes we need to send empty packets.
//static BIT sendEmptyPacketSoon = 0;

// The number of bytes that we have loaded into the IN FIFO that are NOT yet
// queued up to be sent.  This will always be less than 64 because
// once we've loaded up a full packet we should always send it immediately.
static uint8 XDATA inFifoBytesLoaded = 0;
//uint8 XDATA inFifoBytesLoaded = 0;

// True iff we have received a command from the user to enter bootloader mode.
static BIT startBootloaderSoon = 0;
 

// The lower 8-bits of the time (in ms) when the request to enter bootloader mode
// was received.  This allows us to delay for some time before actaully entering
// bootloader mode.  This variable is only valid when startBootloaderSoon == 1.
static uint8 XDATA startBootloaderRequestTime;

//uint8 XDATA startBootloaderRequestTime;

/* CDC ACM USB Descriptors ****************************************************/

USB_DESCRIPTOR_DEVICE CODE usbDeviceDescriptor =
{
    sizeof(USB_DESCRIPTOR_DEVICE),
    1u,
    0x0200,                 // USB Spec Release Number in BCD format
    2,              // Class Code: Communications Device Class
    0,                      // Subclass code: must be 0 according to CDC 1.20 spec
    0,                      // Protocol code: must be 0 according to CDC 1.20 spec
    32,    // Max packet size for Endpoint 0
    0x1FFB,   // Vendor ID
    0x2200,                 // Product ID (Generic Wixel with one CDC ACM port)
    0x0000,                 // Device release number in BCD format
    1,                      // Index of Manufacturer String Descriptor
    2,                      // Index of Product String Descriptor
    3,                      // Index of Serial Number String Descriptor
    1                       // Number of possible configurations.
};

CODE struct CONFIG1 {
    USB_DESCRIPTOR_CONFIGURATION configuration;

    USB_DESCRIPTOR_INTERFACE communication_interface;
    unsigned char class_specific[19];  // CDC-Specific Descriptors
    USB_DESCRIPTOR_ENDPOINT notification_element;

    USB_DESCRIPTOR_INTERFACE data_interface;
    USB_DESCRIPTOR_ENDPOINT data_out;
    USB_DESCRIPTOR_ENDPOINT data_in;
} usbConfigurationDescriptor
=
{
    {                                                    // Configuration Descriptor
        sizeof(USB_DESCRIPTOR_CONFIGURATION),
        2u,
        sizeof(struct CONFIG1),                          // wTotalLength
        2,                                               // bNumInterfaces
        1,                                               // bConfigurationValue
        0,                                               // iConfiguration
        0xC0,                                            // bmAttributes: self powered (but may use bus power)
        50,                                              // bMaxPower
    },
    {                                                    // Communications Interface: Used for device management.
        sizeof(USB_DESCRIPTOR_INTERFACE),
        4u,
        0,                    // bInterfaceNumber
        0,                                               // bAlternateSetting
        1,                                               // bNumEndpoints
        2,                                       // bInterfaceClass
        2,                                // bInterfaceSubClass
        1,                               // bInterfaceProtocol
        0                                                // iInterface
    },
    {                                                    // Functional Descriptors.

        5,                                               // 5-byte General Descriptor: Header Functional Descriptor
        0x24,
        0,
        0x20,0x01,                                       // bcdCDC.  We conform to CDC 1.20.


        4,                                               // 4-byte PTSN-Specific Descriptor: Abstract Control Management Functional Descriptor.
        0x24,
        2,
        2,                                               // bmCapabilities.  See USBPSTN1.2 Table 4.  We support SetLineCoding,
                                                         //SetControlLineState, GetLineCoding, and SerialState notifications.

        5,                                               // 5-byte General Descriptor: Union Interface Functional Descriptor (CDC 1.20 Table 16).
        0x24,
        6,
        0,                    // index of the control interface
        1,                       // index of the subordinate interface

        5,                                               // 5-byte PTSN-Specific Descriptor
        0x24,
        1,
        0x00,                                            // bmCapabilities.  USBPSTN1.2 Table 3.  Device does not handle call management.
        1                        // index of the data interface
    },
    {
        sizeof(USB_DESCRIPTOR_ENDPOINT),
        5u,
        0x80 | 1,  // bEndpointAddress
        3,                     // bmAttributes
        10,                                              // wMaxPacketSize
        1,                                               // bInterval
    },
    {
        sizeof(USB_DESCRIPTOR_INTERFACE),         // Data Interface: used for RX and TX data.
        4u,
        1,                       // bInterfaceNumber
        0,                                               // bAlternateSetting
        2,                                               // bNumEndpoints
        0xA,                        // bInterfaceClass
        0,                                               // bInterfaceSubClass
        0,                                               // bInterfaceProtocol
        0                                                // iInterface
    },
    {                                                    // OUT Endpoint: Sends data out to Wixel.
        sizeof(USB_DESCRIPTOR_ENDPOINT),
        5u,
        0x00 | 4,    // bEndpointAddress
        2,                          // bmAttributes
        64,                             // wMaxPacketSize
        0,                                               // bInterval
    },
    {
        sizeof(USB_DESCRIPTOR_ENDPOINT),
        5u,
        0x80 | 4,     // bEndpointAddress
        2,                          // bmAttributes
        64,                              // wMaxPacketSize
        0,                                               // bInterval
    },
};

uint8 CODE usbStringDescriptorCount = 4;
uint16 CODE languages[] = {4 | 3u<<8, 0x0409};
uint16 CODE manufacturer[] = {38 | 3u<<8, 'P','o','l','o','l','u',' ','C','o','r','p','o','r','a','t','i','o','n'};
uint16 CODE product[] = {12 | 3u<<8, 'W','i','x','e','l'};
//DEFINE_STRING_DESCRIPTOR(languages, 1, 0x0409)
//DEFINE_STRING_DESCRIPTOR(manufacturer, 18, 'P','o','l','o','l','u',' ','C','o','r','p','o','r','a','t','i','o','n')
//DEFINE_STRING_DESCRIPTOR(product, 5, 'W','i','x','e','l')
uint16 CODE * CODE usbStringDescriptors[] = { languages, manufacturer, product, serialNumberStringDescriptor };

/* CDC ACM USB callbacks ******************************************************/
// These functions are called by the low-level USB module (usb.c) when a USB
// event happens that requires higher-level code to make a decision.

void usbCallbackInitEndpoints()
{
    usbInitEndpointIn(1, 10);
    usbInitEndpointOut(4, 64);
    usbInitEndpointIn(4, 64);

    // Force an update to be sent to the computer.
    lastReportedSerialState = 0xFF;
}

// Implements all the control transfers that are required by D1 of the
// ACM descriptor bmCapabilities, (USBPSTN1.20 Table 4).
void usbCallbackSetupHandler()
{
    if ((usbSetupPacket.bmRequestType & 0x7F) != 0x21)   // Require Type==Class and Recipient==Interface.
        return;

    if (!(usbSetupPacket.wIndex == 0 || usbSetupPacket.wIndex == 1))
        return;

    switch(usbSetupPacket.bRequest)
    {
        case 0x20:                          // SetLineCoding (USBPSTN1.20 Section 6.3.10 SetLineCoding)
            usbControlWrite(sizeof(usbComLineCoding), (uint8 XDATA *)&usbComLineCoding);
            break;

        case 0x21:                          // GetLineCoding (USBPSTN1.20 Section 6.3.11 GetLineCoding)
            usbControlRead(sizeof(usbComLineCoding), (uint8 XDATA *)&usbComLineCoding);
            break;

        case 0x22:                   // SetControlLineState (USBPSTN1.20 Section 6.3.12 SetControlLineState)
            usbComControlLineState = usbSetupPacket.wValue;
            usbControlAcknowledge();
            break;
    }
}

void usbCallbackClassDescriptorHandler(void)
{
    // Not used by CDC ACM
}

static void doNothing(void)
{
    // Do nothing.
}

void usbCallbackControlWriteHandler()
{
    usbComLineCodingChangeHandler();

    if (usbComLineCoding.dwDTERate == 333 && !startBootloaderSoon)
    {
        // The baud rate has been set to 333.  That is the special signal
        // sent by the USB host telling us to enter bootloader mode.

        startBootloaderSoon = 1;
        startBootloaderRequestTime = (uint8)getMs();
    }
}

/* CDC ACM RX Functions *******************************************************/
// These functions can be called by the higher-level user of the CDC ACM library
// to receive bytes from the computer.

uint8 usbComRxAvailable()
{
    if (usbDeviceState != 16)
    {
        // We have not reached the Configured state yet, so we should not be touching the non-zero endpoints.
        return 0;
    }

    USBINDEX = 4;      // Select the data endpoint.
    if (USBCSOL & 0x01)  // Check the OUTPKT_RDY flag because USBCNTL is only valid when it is 1.
    {
        // Assumption: We don't need to read USBCNTH because we can't receive packets
        // larger than 255 bytes.
        return USBCNTL;
    }
    else
    {
        return 0;
    }
}

// Assumption: We don't need to read USBCNTH because we can't receive packets
// larger than 255 bytes.
// Assumption: The user has previously called usbComRxAvailable and its return value
// was non-zero.
uint8 usbComRxReceiveByte()
{
    uint8 tmp;

    USBINDEX = 4;         // Select the CDC data endpoint.
    tmp = USBF4;                  // Read one byte from the FIFO.

    if (USBCNTL == 0)                     // If there are no bytes left in this packet...
    {
        USBCSOL &= ~0x01;   // Tell the USB module we are done reading this packet, so it can receive more.
    }

    usbActivityFlag = 1;
    return tmp;
}

// Assumption: The user has previously called usbComRxAvailable and its return value
// was greater than or equal to size.
void usbComRxReceive(uint8 XDATA * buffer, uint8 XDATA size)
{
    usbReadFifo(4, size, buffer);

    if (USBCNTL == 0)
    {
        USBCSOL &= ~0x01;   // Tell the USB module we are done reading this packet, so it can receive more.
    }
}


/* CDC ACM TX Functions *******************************************************/
// These functions can be called by the higher-level user of the CDC ACM library
// to send bytes to the computer.

static void sendPacketNow()
{
    USBINDEX = 4;
    USBCSIL |= 0x01;                      // Send the packet.

    // If the last packet transmitted was a full packet, we should send an empty packet later.
    //sendEmptyPacketSoon = (inFifoBytesLoaded == 64);

    // There are 0 bytes in the IN FIFO now.
    inFifoBytesLoaded = 0;

    // Notify the USB library that some activity has occurred.
    usbActivityFlag = 1;
}

void usbComService(void)
{
    usbPoll();

    // Start bootloader if necessary.
    if (startBootloaderSoon && (uint8)(getMs() - startBootloaderRequestTime) > 70)
    {
        // It has been 50 ms since the user requested that we start the bootloader, so
        // start it.

        // The reason we don't start the bootloader right away when we get the request is
        // because we want to have time to finish the status phase of the control transfer
        // so the host knows the request was processed correctly.  Also, the Windows
        // usbser.sys sends several requests for about 7 ms after the SET_LINE_CODING
        // request before the operation (SetCommState) finally succeeds.
        boardStartBootloader();
    }

    if (usbDeviceState != 16)
    {
        // We have not reached the Configured state yet, so we should not be touching the non-zero endpoints.
        return;
    }

    // Send a packet now if there is data loaded in the FIFO waiting to be sent OR
    //
    // Typical USB systems wait for a short or empty packet before forwarding the data
    // up to the software that requested it, so this is necessary.  However, we only transmit
    // an empty packet if there are no packets currently loaded in the FIFO.
    USBINDEX = 4;
    //if (inFifoBytesLoaded || ( sendEmptyPacketSoon && !(USBCSIL & 0x02) ) )
    if (inFifoBytesLoaded || ( (inFifoBytesLoaded == 64) && !(USBCSIL & 0x02) ) )
    {
        sendPacketNow();
    }

    // Notify the computer of the current serial state if necessary.
    USBINDEX = 1;
    if (usbComSerialState != lastReportedSerialState && !(USBCSIL & 0x01))
    {
        // The serial state has changed since the last time we sent it.
        // AND we are ready to send it to the USB host, so send it.
        // See PSTN Section 6.5.4, SerialState for an explanation of this packet.

        USBF1 = 0b10100001;   // bRequestType: Direction=IN, Type=Class, Sender=Interface
        USBF1 = 0x20; // bRequest

        // wValue is zero.
        USBF1 = 0;
        USBF1 = 0;

        // wIndex is the number of the interface this notification comes from.
        USBF1 = 0;
        USBF1 = 0;

        // wLength is 2 because the data part has two bytes
        USBF1 = 2;
        USBF1 = 0;

        // Data
        USBF1 = usbComSerialState;
        USBF1 = 0;

        USBCSIL |= 0x01;

        // As specified in PSTN 1.20 Section 6.5.4, we clear the "irregular" signals.
        usbComSerialState &= ~ACM_IRREGULAR_SIGNAL_MASK;

        lastReportedSerialState = usbComSerialState;

        // Notify the USB library that some activity has occurred.
        usbActivityFlag = 1;
    }
}

// Assumption: We are using double buffering, so we can load either 0, 1, or 2
// packets into the FIFO at this time.
uint8 usbComTxAvailable()
{
    uint8 tmp;

    if (usbDeviceState != 16)
    {
        // We have not reached the Configured state yet, so we should not be touching the non-zero endpoints.
        return 0;
    }

    USBINDEX = 4;
    tmp = USBCSIL;
    if (tmp & 0x02)
    {
        if (tmp & 0x01)
        {
            return 0;                                       // 2 packets are in the FIFO, so no room
        }
        return 64 - inFifoBytesLoaded;      // 1 packet is in the FIFO, so there is room for 1 more
    }
    else
    {
        return (64<<1) - inFifoBytesLoaded; // 0 packets are in the FIFO, so there is room for 2 more
    }
}

// Assumption: The user called usbComTxAvailable() before calling this function,
// and it returned a number greater than or equal to size.
void usbComTxSend(const uint8 XDATA * buffer, uint8 XDATA size)
{
    uint8 packetSize;
    while(size)
    {
        packetSize = 64 - inFifoBytesLoaded;   // Decide how many bytes to send in this packet (packetSize).
        if (packetSize > size){ packetSize = size; }

        usbWriteFifo(4, packetSize, buffer);    // Write those bytes to the USB FIFO.

        buffer += packetSize;                                   // Update pointers.
        size -= packetSize;
        inFifoBytesLoaded += packetSize;

        if (inFifoBytesLoaded == 64)
        {
            sendPacketNow();
        }
    }
}

void usbComTxSendByte(uint8 byte)
{
    // Assumption: usbComTxAvailable() recently returned a non-zero number

    USBF4 = byte;                          // Give the byte to the USB module's FIFO.
    inFifoBytesLoaded++;

    if (inFifoBytesLoaded == 64)
    {
        sendPacketNow();
    }

    // Don't set usbActivityFlag here; wait until we actually send the packet.
}

/* CDC ACM CONTROL SIGNAL FUNCTIONS *******************************************/

uint8 usbComRxControlSignals()
{
    return usbComControlLineState;
}

void usbComTxControlSignals(uint8 signals)
{
    usbComSerialState = (usbComSerialState & ACM_IRREGULAR_SIGNAL_MASK) | signals;
}

void usbComTxControlSignalEvents(uint8 signalEvents)
{
    usbComSerialState |= signalEvents;
}
