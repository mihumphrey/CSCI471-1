#include "includes.h"

// ***************************************************************************
// * ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
// *
// * These are the functions you need to fill in.
// ***************************************************************************


// Initialization of global variables for Go Back N packet management
int BASE, NEXTSEQNUM, EXPECTEDSEQNUM, CUM_ACK, CLEAN_WRITE;

std::vector<pkt> sentPackets(20000);

const int TIMER_INTERVAL = 40;
/**
 * This method will create a packet struct
 * int seqnum - the sequence number of the packet
 * char data[]- the data passed in the message from Layer 5, size used to calculate the ACK number
 * int checksum - the checksum value of the packet
 **/
pkt makePacket(int seqnum, char data[], int checksum) {
  // initialize packet
  struct pkt newPacket;

  // populate packet
  newPacket.seqnum = seqnum;

  // sets acknumber to be the seqnumber plus the size of the data in the message
  newPacket.acknum = (seqnum + strlen(data));
  newPacket.checksum = checksum;

  bcopy(data, newPacket.payload, 20);
  return newPacket;
}

/**
 * This method will create a packet struct
 * int seqnum - the sequence number of the packet
 * int acknum - the ACK number of the packet
 * int checksum - the checksum value of the packet
 * char[] payload - the payload of the packet
 **/
pkt makePacket(int seqnum, int acknum, int checksum, char payload[]) {
  // initialize packet
  struct pkt newPacket;

  //populate packet
  newPacket.seqnum = seqnum;
  newPacket.acknum = acknum;
  newPacket.checksum = checksum;

  bcopy(payload, newPacket.payload, 20);
  return newPacket;
}

/**
 * This function checks to see if the packet's sequence number is equal to the
 * expected next sequence number
 * 
 * packet - packet sent over layer 3, pkt struct in Simulator
 * EXPECTEDSEQNUM - global variable to check the next sequence number
 **/
bool hasNextSeqNum(pkt &packet, int expectedNum){
  return packet.seqnum == expectedNum;
}

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity A routines are called. You can use it to do any initialization
// ***************************************************************************
void A_init()
{
  BASE = 1;
  NEXTSEQNUM = 1;
  // initialize BASE and nextseqnum to 1
}

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity B routines are called. You can use it to do any initialization
// ***************************************************************************
void B_init()
{
  EXPECTEDSEQNUM = 1;
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
int calculateChecksum(pkt packet) {
  // calculate check sum of fields.

  std::bitset<16> seqBit(packet.seqnum);
  std::bitset<16> ackBit(packet.acknum);

  std::vector<std::bitset<16>> packetWords;

  packetWords.push_back(seqBit);
  packetWords.push_back(ackBit);

  int parseLength = sizeof(packet.payload);
  
  for(int i = 0; i < parseLength; i++) {
    std::bitset<16> charToInt(packet.payload[i]);
    
    packetWords.push_back(charToInt);
  }

  int checkSum = packetWords[0].to_ulong();
  for(int i = 1; i < packetWords.size(); i++) {
    checkSum += packetWords[i].to_ulong();
  }


  return checkSum;
}

/**
 * Will determine whether or not a packet is corrupted
 * 
 * packet - struct from simulator.cpp
 **/
bool isPacketCorrupt(pkt packet) {
  if (packet.checksum == calculateChecksum(packet)) {
    INFO << "PACKET IS NOT CORRUPT"; 
    return false;
  } else {
    INFO << "FUCK!!!!!!!!! PACKET WAS CORRUPTED";
    return true;
  }
}

// ***************************************************************************
// * Called from layer 5, passed the data to be sent to other side 
// ***************************************************************************
int A_output(struct msg message)
{
  std::cout << "Layer 4 on side A has recieved a message from the application that should be sent to side B: " << message << std::endl;

  // maybe make ack 1, not used anywhere
  struct pkt packet = makePacket(NEXTSEQNUM, NEXTSEQNUM, 0, message.data);
  packet.checksum = calculateChecksum(packet);

  simulation->tolayer3(A,packet);
  sentPackets[NEXTSEQNUM] = packet;
  
  if (BASE == NEXTSEQNUM) {
    simulation->starttimer(A, TIMER_INTERVAL);
  }
  
  NEXTSEQNUM++;
  
  return 1;
}


// ***************************************************************************
// * Called from layer 3, when a packet arrives for layer 4 on side A
// ***************************************************************************
void A_input(struct pkt packet)
{
  std::cout << "Layer 4 on side A has recieved a packet sent over the network from side B:" << packet << std::endl;
  
  if(!isPacketCorrupt(packet)) {

    // struct msg message;
    // bcopy(packet.payload,message.data,20);
    // simulation->tolayer5(A,message);

    // sets the BASE to the ack number from B
    BASE = packet.acknum + 1;

    if(BASE == NEXTSEQNUM) {
      simulation->stoptimer(A);
    } else {
      simulation->starttimer(A, TIMER_INTERVAL);
    }

  }
   else {
    
    // restart timer
    // simulation->stoptimer(A);
    // simulation->starttimer(A,TIMER_INTERVAL);
    
    // re-initialize A since the packet from B was corrupt
    // A_init();
    return;
    // NEXTSEQNUM = BASE;
    // BASE = 1;
  }
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
  if(!isPacketCorrupt(packet) && hasNextSeqNum(packet, EXPECTEDSEQNUM)) {
    // extract data from packet payload and pass to Layer 5
    struct msg message;
    bcopy(packet.payload,message.data,20);
    simulation->tolayer5(B,message);
    
    // send a packet back to A sendpkt = makepacket(expectedseqnum, ACK, checksum)
    char emptyStr[20] = {0};
    
    // creates packet and calculates the checksum
    // sequence number is arbitrary
    struct pkt sendPacket = makePacket(1, EXPECTEDSEQNUM, 0, emptyStr);
    sendPacket.checksum = calculateChecksum(sendPacket);

    // puts packet onto the network
    simulation->tolayer3(B,sendPacket);
    
    EXPECTEDSEQNUM++;
    // LASTSEQNUM = packet.seqnum;
    // LASTSEQNUM = EXPECTEDSEQNUM;
  } else {
    // resends an ACK for the most recently received in-order packet
    
    // restart the timer
    simulation->stoptimer(A);
    simulation->starttimer(A,TIMER_INTERVAL);
    
    char emptyStr[20] = {0};

    // sequence numbmer for B is arbitrary
    struct pkt sendPacket = makePacket(1, EXPECTEDSEQNUM - 1, 0, emptyStr);
    sendPacket.checksum = calculateChecksum(sendPacket);

    simulation->tolayer3(B,sendPacket);
  }
}



// ***************************************************************************
// * Called when A's timer goes off 
// ***************************************************************************
void A_timerinterrupt()
{
  // simulation->stoptimer(A);
  simulation->starttimer(A, TIMER_INTERVAL);
  
  // std::cout << "Side A's timer has gone off. LASTSEQNUM: " << LASTSEQNUM << " BASE: " << BASE << " NEXTSEQNUM: " << NEXTSEQNUM << std::endl;
  
  // for issues with stressTest.pl
  int maxRetransmit = std::min(NEXTSEQNUM, BASE + 20);

  for(int i = BASE; i < maxRetransmit; i++) {
    simulation->tolayer3(A, sentPackets[i]);
  }
    
  // NEXTSEQNUM = BASE + maxRetransmit;
}

// XC
// ***************************************************************************
// * Called when B's timer goes off 
// ***************************************************************************
void B_timerinterrupt()
{
    std::cout << "Side B's timer has gone off." << std::endl;
}







