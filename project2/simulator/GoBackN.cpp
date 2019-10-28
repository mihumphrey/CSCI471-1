#include "includes.h"

// ***************************************************************************
// * ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
// *
// * These are the functions you need to fill in.
// ***************************************************************************


// Initialization of global variables for Go Back N packet management
int A_BASE, NEXTSEQNUM, EXPECTEDSEQNUM, LASTSEQNUM, CUM_ACK, CLEAN_WRITE;

std::vector<pkt> sentPackets(10000);
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
  A_BASE = 1;
  NEXTSEQNUM = 1;
  // initialize A_BASE and nextseqnum to 1
}

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity B routines are called. You can use it to do any initialization
// ***************************************************************************
void B_init()
{
  EXPECTEDSEQNUM = 1;
  // CUM_ACK gets initialized to 1 since A_BASE gets initialized to 1 in A_init()
  CUM_ACK = 1;
  LASTSEQNUM = 1;
  CLEAN_WRITE = 0;
  // initialize expectedseqnum to 1, create a sendpacket (makepacket(0, ACK, checksum)), lastseqnum = -1
}



// // The main function that adds two-bit sequences and returns the addition 
// string addBitStrings( string first, string second ) 
// { 
//     string result;  // To store the sum bits 
  
//     // make the lengths same before adding 
//     int length = makeEqualLength(first, second); 
  
//     int carry = 0;  // Initialize carry 
  
//     // Add all bits one by one 
//     for (int i = length-1 ; i >= 0 ; i--) 
//     { 
//         int firstBit = first.at(i) - '0'; 
//         int secondBit = second.at(i) - '0'; 
  
//         // boolean expression for sum of 3 bits 
//         int sum = (firstBit ^ secondBit ^ carry)+'0'; 
  
//         result = (char)sum + result; 
  
//         // boolean expression for 3-bit addition 
//         carry = (firstBit & secondBit) | (secondBit & carry) | (firstBit & carry); 
//     } 
  
//     // if overflow, then add a leading 1 
//     if (carry) 
//         result = '1' + result; 
  
//     return result; 
// }

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

  struct pkt packet = makePacket(NEXTSEQNUM, NEXTSEQNUM, 0, message.data);
  packet.checksum = calculateChecksum(packet);

  simulation->tolayer3(A,packet);
  sentPackets[NEXTSEQNUM] = packet;
  
  if (A_BASE == NEXTSEQNUM) {
    INFO << "A_BASE == NEXTSEQNUM starting timer from A_output";
    simulation->starttimer(A, 100);
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

    // sets the BASE to the ack number from B
    A_BASE = packet.acknum + 1;
    if(A_BASE >= NEXTSEQNUM) {
      INFO << "STOPPING TIMER WHOOO A_BASE == NEXTSEQNUM in A_input";
      simulation->stoptimer(A);
    } else {
      simulation->stoptimer(A);
      INFO << "FUCKKKK INFINITE LOOP STARTING A_BASE != NEXTSEQNUM starting timer from A_input";
      simulation->starttimer(A, 100);
    }

    // struct msg message;
    // bcopy(packet.payload,message.data,20);
    // simulation->tolayer5(A,message);

  } else { // re-initialize A since the packet from B was corrupt
    A_init();
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
  if(packet.seqnum > 0 && !isPacketCorrupt(packet)) {

    if(packet.seqnum > 0 && hasNextSeqNum(packet, EXPECTEDSEQNUM)) {

        // extract data from packet payload and pass to Layer 5
        struct msg message;
        bcopy(packet.payload,message.data,20);
        simulation->tolayer5(B,message);
        
        // send a packet back to A sendpkt = makepacket(expectedseqnum, ACK, checksum)
        char emptyStr[20] = {0};

        // gets the size of the payload and adds it the cumulative ACK
        // CUM_ACK += sizeof(packet.payload);
        CUM_ACK++;
        CLEAN_WRITE++;
        // creates packet and calculates the checksum
        struct pkt sendPacket = makePacket(EXPECTEDSEQNUM, (CUM_ACK), 0, emptyStr);
        sendPacket.checksum = calculateChecksum(sendPacket);

        // puts packet onto the network
        simulation->tolayer3(B,sendPacket);
        
        // increment expected sequence number
        EXPECTEDSEQNUM++;
        INFO << "Setting LASTSEQNUM: " << packet.seqnum;
        LASTSEQNUM = packet.seqnum;

    } else { // discards out of order packet
      // resends an ACK for the most recently received in-order packet
      char emptyStr[20] = {0};
      EXPECTEDSEQNUM = LASTSEQNUM;
      struct pkt sendPacket = makePacket(LASTSEQNUM, LASTSEQNUM, 0, emptyStr);
      sendPacket.checksum = calculateChecksum(sendPacket);
      simulation->tolayer3(B,sendPacket);
    }
  } else {
      // resends an ACK for the most recently received in-order packet
      char emptyStr[20] = {0};
      EXPECTEDSEQNUM = LASTSEQNUM;
      struct pkt sendPacket = makePacket(LASTSEQNUM, LASTSEQNUM, 0, emptyStr);
      sendPacket.checksum = calculateChecksum(sendPacket);
      simulation->tolayer3(B,sendPacket);
  }
}



// ***************************************************************************
// * Called when A's timer goes off 
// ***************************************************************************
void A_timerinterrupt()
{
  simulation->stoptimer(A);
  std::cout << "Side A's timer has gone off. LASTSEQNUM: " << LASTSEQNUM << " A_BASE: " << A_BASE << " NEXTSEQNUM: " << NEXTSEQNUM << std::endl;
  for(int i = LASTSEQNUM + 1; i < NEXTSEQNUM; i++){
    struct pkt packet = sentPackets[i];
    simulation->tolayer3(A,packet);
  }
  // if(CLEAN_WRITE < sentPackets.size()) {
    INFO << "STARTING TIMER from A_timerinterrupt, fuck this shit";
    
    simulation->stoptimer(A);
    simulation->starttimer(A, 100);
  // }
  // send all packets from A_BASE < x < nextseqnum - 1
}

// XC
// ***************************************************************************
// * Called when B's timer goes off 
// ***************************************************************************
void B_timerinterrupt()
{
    std::cout << "Side B's timer has gone off." << std::endl;
}







