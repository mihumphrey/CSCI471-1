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

// Network Layer constants
const int ETHERNET_1_THRESH = 1500;
const int IP_V_4 = 2048;
const int ARP = 2054;
const int IP_V_6 = 34525;

const int IP_OFFSET = 14;

// Transport Layer constants
const int ICMP = 1;
const int TCP = 6;
const int UDP = 17;

const int TRANSPORT_LAYER_OFFSET = 35;

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
	struct ether_header *link_header;
	link_header = ( struct ether_header * ) packet;
	u_int64_t mac_src = 0;
	u_int64_t mac_dst = 0;
	memcpy( &mac_src, link_header->ether_shost, sizeof mac_src );
	memcpy( &mac_dst, link_header->ether_dhost, sizeof mac_dst );

	results->newSrcMac( mac_src );
	results->newDstMac( mac_dst );

	// log src/dest bytes
	TRACE << "\tSource MAC = " << ether_ntoa( ( const ether_addr * ) link_header->ether_shost ) << ENDL;
	TRACE << "\tDestination MAC = " << ether_ntoa( ( const ether_addr * ) link_header->ether_dhost ) << ENDL;

	// extract the length or type of the ethernet field
	uint16_t len_type_bytes = ntohs( link_header->ether_type );

	// log trace len/type
	// TRACE << "Length or type value from ethernet header: " << len_type_bytes << ENDL;
	
	// *******************************************************************
	// * If it's an ethernet packet, extract the src/dst address and  
	// * find the ethertype value to see what the next layer is.
	// * 
	// * If it's not an ethernet packet, count is as "other" and you're done
	// * with this packet.
	// *******************************************************************

	if(len_type_bytes < ETHERNET_1_THRESH) {
		results->newOtherLink(len_type_bytes);

		TRACE << "\tIs not an Ethernet packet" << ENDL;
		// add log for other link
		return;
	}

	// add logging for ethernet II
	TRACE << "\tEther Type = " << len_type_bytes << ENDL;
	TRACE << "\t\tPacket is Ethernet II" << ENDL;

	results->newEthernet( pkthdr->caplen );

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
	bool isIPv4 = false;

	if( len_type_bytes == ARP ) {
		results->newARP( pkthdr->caplen );

		return;
	} else if( len_type_bytes == IP_V_6 ) {
		results->newIPv6( pkthdr->caplen );
		TRACE << "\tPacket is IPv6" << ENDL;

		return;
	} else if( len_type_bytes == IP_V_4 ) {
		isIPv4 = true;

		results->newIPv4( pkthdr->caplen );
		TRACE << "\tPacket is IPv4" << ENDL;

		struct ip* ip_v4_header;
		ip_v4_header = ( struct ip * )( packet + IP_OFFSET );

		uint32_t ip_src = ip_v4_header->ip_src.s_addr;
		results->newSrcIPv4( ( uint64_t ) ip_v4_header->ip_src.s_addr );
		TRACE << "\tSource IP address is " << inet_ntoa( ip_v4_header->ip_src ) << ENDL;

		uint32_t ip_dst = ip_v4_header->ip_dst.s_addr;
		results->newDstIPv4( ( uint64_t ) ip_v4_header->ip_dst.s_addr );
		TRACE << "\tDestination IP address is " << inet_ntoa( ip_v4_header->ip_dst ) << ENDL;

		struct ip_timestamp* ip_flags = ( struct ip_timestamp * )( packet + IP_OFFSET + 6);
		
		// uint64_t flagr = 0;
		// memcpy(&flagr, ip_flags->ipt_flg, sizeof flagr);
		TRACE << "\tipFlags = " << ntohs ( ip_flags->ipt_code ) << ENDL;
	} else {
		results->newOtherNetwork( pkthdr->caplen );
		TRACE << "\tPacket is from another network" << ENDL;
	}

    // ***********************************************************************
    // * Process the transport layer header
    // ***********************************************************************

	// *******************************************************************
	// * If the packet is an IPv4 packet, then use the Protcol field
	// * to find out what the next layer is.
	// * 
	// * If it's ICMP, count it and you are done with this packet.
	// *
	// * If it's UDP or TCP, decode the transport header to extract
	// * the src/dst ports and TCP flags if needed.
	// *
	// * If it's not ICMP, UDP or TCP, count it as otherTransport
	// *******************************************************************
	if( isIPv4 ) {
		struct ip* ip_v4_header;
		ip_v4_header = ( struct ip * )( packet + IP_OFFSET );

		uint32_t ip_v4_protocol = ( uint32_t )( ip_v4_header->ip_p );

		switch( ip_v4_protocol ) {
			case ICMP:
				results->newICMP( pkthdr->caplen );
				TRACE << "\tPacket is ICMP" << ENDL;
				return;
			case TCP:
				// extract the src/dst ports and TCP flags
				struct tcphdr* tcp_header;

				tcp_header = ( struct tcphdr * ) ( ( uint64_t ) ( packet + IP_OFFSET + ( ip_v4_header->ip_hl ) * 4 ) );
				
				// add TCP header
				results->newTCP( pkthdr->caplen );
				TRACE << "Packet is TCP" << ENDL;

				// extract src port from TCP header
				results->newSrcTCP( ntohs ( tcp_header->th_sport ) );
				TRACE << "Source port #" << ntohs( tcp_header->th_sport ) <<  ENDL;

				// extract dst port from TCP header
				results->newDstTCP( ntohs ( tcp_header->th_dport ) );
				TRACE << "Destination port #" << ntohs( tcp_header->th_dport ) << ENDL;

				// check if SYN bit is set in TCP header
				if( tcp_header->th_flags & TH_SYN ) {
					results->incrementSynCount();
					TRACE << "SYN bit set" << ENDL;
				}
				// if( tcp_header->th_flags & TH_SYN ) {
				// 	results->incrementSynCount();
				// 	TRACE << "Incrementing SYN count" << ENDL;
				// }


				// check if FIN bit is set in TCP header
				if( tcp_header->th_flags & TH_FIN ) {
					results->incrementFinCount();
					TRACE << "Incrementing Fin count" << ENDL;
				}
				// if( ( tcp_header->th_flags & TH_FIN ) != 0 ) {
				// 	results->incrementFinCount();
				// 	TRACE << "Incrementing Fin count" << ENDL;
				// }

				break;
			case UDP:
				// extract the src/dst ports
				struct udphdr* udp_header;

				udp_header = ( struct udphdr * ) ( ( uint64_t ) ( packet + IP_OFFSET + ( ip_v4_header->ip_hl ) * 4 ) );
				
				// add udp header
				results->newUDP(pkthdr->caplen);
				TRACE << "Packet is UDP" << ENDL;

				// extract UDP src port from UDP header
				results->newSrcUDP( ntohs ( udp_header->uh_sport ) );
				TRACE << "Source port #" << ntohs( udp_header->uh_sport ) << ENDL;

				// extract UDP dst port from UDP header
				results->newDstUDP( ntohs ( udp_header->uh_dport ) );
				TRACE << "Destination port #" << ntohs( udp_header->uh_dport ) << ENDL;

				break;
			default:
				results->newOtherTransport( pkthdr->caplen );
				TRACE << "Packet is another transport protocol" << ENDL;

				break;
		}
	}

    return;
}
