//
// Created by Phil Romig on 11/13/18.
//

#include "packetstats.h"

// ****************************************************************************
// * pk_processor()
// *  Most/all of the work done by the program will be done here (or at least it
// *  it will originate here). The function will be called once for every
// *  packet in the savefile.
// ****************************************************************************
void pk_processor(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet) {

    resultsC* results = (resultsC*)user;
    results->incrementTotalPacketCount();
    DEBUG << "Processing packet #" << results->packetCount() << ENDL;
    char s[256]; bzero(s,256); bcopy(ctime(&(pkthdr->ts.tv_sec)),s,strlen(ctime(&(pkthdr->ts.tv_sec)))-1);
    TRACE << "\tPacket timestamp is " << s;
    TRACE << "\tPacket capture length is " << pkthdr->caplen ;
    TRACE << "\tPacket physical length is " << pkthdr->len ;

    // ***********************************************************************
    // * Process the link layer header
    // *  Hint -> use the ether_header structure defined in
    // ***********************************************************************

    	// *******************************************************************
    	// * If it's an ethernet packet, extract the src/dst address and  
    	// * find the ethertype value to see what the next layer is.
	// * 
	// * If it's not an ethernet packet, count is as "other" and your done
	// * with this packet.
    	// *******************************************************************

    // ***********************************************************************
    // * Process the network layer
    // ***********************************************************************

    	// *******************************************************************
	// *  Use ether_type to decide what the next layer is.  You
	// *  If it's ARP or IPv6 count it and you are done with this packet.
	// * 
	// * If it's IPv4 extract the src and dst addresses and find the
	// * protocol field to see what the next layer is.  
	// * 
	// * If it's not ARP, IPv4 or IPv6 count it as otherNetwork.
    	// *******************************************************************

    // ***********************************************************************
    // * Process the transport layer header
    // ***********************************************************************

    	// *******************************************************************
	// * If the packet is an IPv4 packet, then use the Protcol field
	// * to find out what the next layer is.
	// * 
	// * If it's ICMP, count it and you are done with this packet.
	// *
	// * If it's UDP or TCP, decode the transport hearder to extract
	// * the src/dst ports and TCP flags if needed.
	// *
	// * If it's not ICMP, UDP or TCP, count it as otherTransport
    	// *******************************************************************


    return;
}
