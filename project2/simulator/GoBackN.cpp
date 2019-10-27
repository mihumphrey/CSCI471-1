#include "includes.h"

// ***************************************************************************
// * ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
// *
// * These are the functions you need to fill in.
// ***************************************************************************


/**
 * This method will create a packet struct
 * int seqnum - the sequence number of the packet
 * int acknum - the ACK number of the packet
 * int checksum - the checksum value of the packet
 * char[] payload - the payload of the packet
 **/
// packet makePacket(int seqnum, int acknum, int checksum, char[] payload)

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity A routines are called. You can use it to do any initialization
// ***************************************************************************
void A_init()
{
  // initialize base and nextseqnum to 1
}

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity B routines are called. You can use it to do any initialization
// ***************************************************************************
void B_init()
{
  // initialize expectedseqnum to 1, create a sendpacket (makepacket(0, ACK, checksum)), lastseqnum = -1
}





/**
 * Will calculate the checksum of a packet.
 * Convert seqnum (int to binary), acknum (int to binary), payload (char[] to binary) all to binary. 
 * Will need to split up the data field of the packet to have the same length as seqnum and acknum
 * Will sum all of these fields together using binary addition.
 * Apply 1's complement to the sum, this is the checksum
 * 
 * packet - struct from simulator.cpp
 **/
// int calculateChecksum(packet)

/**
 * Will determine whether or not a packet is corrupted
 * 
 * packet - struct from simulator.cpp
 **/
// boolean isCorrupt(packet)

// ***************************************************************************
// * Called from layer 5, passed the data to be sent to other side 
// ***************************************************************************
int A_output(struct msg message)
{
  std::cout << "Layer 4 on side A has recieved a message from the application that should be sent to side B: " << message << std::endl;

  struct pkt packet;
  packet.seqnum = 1;
  packet.acknum = 1;
  
  // need to construct the checksum here, will not be zero
  packet.checksum = 0;


  bcopy(message.data,packet.payload,20);


  simulation->tolayer3(A,packet);

  // if base == nextseqnum
  // nextseqnum++
  return 1;
}


// ***************************************************************************
// * Called from layer 3, when a packet arrives for layer 4 on side A
// ***************************************************************************
void A_input(struct pkt packet)
{
    std::cout << "Layer 4 on side A has recieved a packet sent over the network from side B:" << packet << std::endl;
    
    // if packet && !isPacketCorrupt
      // base = getAcknum(packet) + 1
      // if base == nextseqnum
          // stop_timer
      // else
          // start_timer
    // else
      // handle corrupt packet

    struct msg message;
    bcopy(packet.payload,message.data,20);
    simulation->tolayer5(A,message);
}

// XC implementation
// ***************************************************************************
// * Called from layer 5, passed the data to be sent to other side
// ***************************************************************************
int B_output(struct msg message)
{
    std::cout << "Layer 4 on side B has recieved a message from the application that should be sent to side A: " << message << std::endl;
    return 1; /* Return a 0 to refuse the message */
}


// ***************************************************************************
// // called from layer 3, when a packet arrives for layer 4 on side B 
// ***************************************************************************
void B_input(struct pkt packet)
{
  std::cout << "Layer 4 on side B has recieved a packet from layer 3 sent over the network from side A:" << packet << std::endl;
  // if packet && !isCorrupt(packet)
    // if hasnextseqnum(packet, expectedsegnum)
      // extract data from packet payload and pass to Layer 5
      struct msg message;
      bcopy(packet.payload,message.data,20);
      simulation->tolayer5(B,message);
      
      // send a packet back to A sendpkt = makepacket(expectedseqnum, ACK, checksum)
      //  simulation->tolayer3(A,sendpkt);
      //  expectedseqnum++
      // lastseqnum = packet.seqnum
    // else
      // discards out of order packet
  // else
    // discrards the packet
    // resends an ACK for the most recently received in-order packet


}



// ***************************************************************************
// * Called when A's timer goes off 
// ***************************************************************************
void A_timerinterrupt()
{
  std::cout << "Side A's timer has gone off." << std::endl;
  // send all packets from base < x < nextseqnum - 1
}

// ***************************************************************************
// * Called when B's timer goes off 
// ***************************************************************************
void B_timerinterrupt()
{
    std::cout << "Side B's timer has gone off." << std::endl;
}







