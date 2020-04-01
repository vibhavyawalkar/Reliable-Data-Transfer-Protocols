#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<vector>

using namespace std;
#include "../include/simulator.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* Sender Data */
int base = 0;
int nextseqnum = 0;
int N;
float t = 10.00;
vector<struct msg> buffer; /* Message buffer */
struct pkt pktBuffer[1001]; /* Packet buffer */
	
/* Receiver Data */
int expectedseqnum = 0;
struct pkt myackpkt;

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
	cout << "A_output() called" << endl;
	//buffer.push_back(message);
	if(nextseqnum < base + N) {
		struct pkt mypkt;
		memset(&mypkt, 0, sizeof(mypkt));
		mypkt.seqnum = nextseqnum;
		mypkt.acknum = -1;
		if(buffer.size() != 0) {
			//buffer.push_back(message);
			while(buffer.size() != 0 && nextseqnum < base + N) {
				for(int i = 0; i < 20; i++) {
					mypkt.payload[i] = buffer[0].data[i];
					pktBuffer[nextseqnum].payload[i] = buffer[0].data[i];
				}
				buffer.erase(buffer.begin());

				int checksum = mypkt.acknum + mypkt.seqnum;
                		int payloadChecksum = 0;
                		for(int i = 0; i < 20; i++)
                        		payloadChecksum += mypkt.payload[i];
                		checksum += payloadChecksum;

                		mypkt.checksum = checksum;
                		cout << "Sending pkt seqno: " << mypkt.seqnum << "msg: ";
                		for(int k = 0; k < 20; k++)
                        		cout << mypkt.payload[k];
                		cout <<  " checksum" << mypkt.checksum << endl;
                		pktBuffer[nextseqnum].seqnum = mypkt.seqnum;
                		pktBuffer[nextseqnum].checksum = mypkt.checksum;
                		pktBuffer[nextseqnum].acknum = mypkt.acknum;
                		cout << "PktBuffer contents : seqno: " << pktBuffer[nextseqnum].seqnum << "msg: " << pktBuffer[nextseqnum].payload << "checksum" << pktBuffer[nextseqnum].checksum << endl;
                		tolayer3(0, mypkt);
                		if(base == nextseqnum)
                        		starttimer(0, t);
                		nextseqnum++;
			}
			//return;
		}
		if(nextseqnum >= base + N)
			goto buff;
		 //else {
			for(int i = 0; i < 20; i++) {
                                mypkt.payload[i] = message.data[i];
                                pktBuffer[nextseqnum].payload[i] = message.data[i];
                        }

		//}
		int checksum = mypkt.acknum + mypkt.seqnum;
		int payloadChecksum = 0;
		for(int i = 0; i < 20; i++)
			payloadChecksum += mypkt.payload[i];
		checksum += payloadChecksum;

		mypkt.checksum = checksum;
		cout << "Sending pkt seqno: " << mypkt.seqnum << "msg: "; 
		for(int k = 0; k < 20; k++)
			cout << mypkt.payload[k];
		cout <<  " checksum" << mypkt.checksum << endl;
		pktBuffer[nextseqnum].seqnum = mypkt.seqnum;
		pktBuffer[nextseqnum].checksum = mypkt.checksum;
		pktBuffer[nextseqnum].acknum = mypkt.acknum;
		cout << "PktBuffer contents : seqno: " << pktBuffer[nextseqnum].seqnum << "msg: " << pktBuffer[nextseqnum].payload << "checksum" << pktBuffer[nextseqnum].checksum << endl;
		tolayer3(0, mypkt);
		if(base == nextseqnum)
			starttimer(0, t);
		nextseqnum++;	
	} else {
		/*Buffer the message */
buff:		buffer.push_back(message);
	}
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	cout << "A_input() called" << endl;
	int checksum = packet.acknum + packet.seqnum;
	if(checksum == packet.checksum) {
		cout << "ACK not corrupt" << endl;
		base = packet.acknum + 1;
		if(base == nextseqnum)
			stoptimer(0);
		else {
			starttimer(0, t);
		}
	} else {
		/*Packet is corrupt, do nothing */
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	cout << "A_timerinterrupt() called" << endl;
	starttimer(0, t);
	for(int i = base; i < nextseqnum; i++) {
		/*struct pkt p;
		memset(&p, 0, sizeof(p));
		p.seqnum = pktBuffer[i].seqnum;
		p.acknum = pktBuffer[i].acknum;
		p.checksum = pktBuffer[i].checksum;
		for(int j = 0; j < 20; j++)
			p.payload[j] = pktBuffer[i].payload[j];
		cout << "Sending pkt seqno: " << p.seqnum << " msg: " << p.payload << "checksum: " << p.checksum << " " << pktBuffer[i].checksum << endl;*/
		cout << "Sending pkt seqno: " << pktBuffer[i].seqnum << " msg: ";
		for(int j = 0; j < 20; j++)
			cout << pktBuffer[i].payload[j];
		cout << " checksum: " << pktBuffer[i].checksum << endl;
		tolayer3(0, pktBuffer[i]);
	}
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	cout << "A_init() called" << endl;
	base = 0;
	nextseqnum = 0;
	N = getwinsize();
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	cout << "B_input() called" << endl;
	int checksum = packet.acknum + packet.seqnum;
	int payloadChecksum;
	for(int i = 0; i < 20; i++)
		payloadChecksum += packet.payload[i];
	checksum += payloadChecksum;

	if(packet.checksum == checksum) {
		cout << "Packet not corrupt, seq : " << packet.seqnum << " expected : " << expectedseqnum<< endl;
		if(packet.seqnum == expectedseqnum) {
		char receivedMessage[20];
		memset(&receivedMessage, 0, sizeof(receivedMessage));
		for(int i = 0; i < 20; i++)
			receivedMessage[i] = packet.payload[i];
		cout << "Delivered message to application layer" << endl;
		/* Deliver data to the application layer */
		tolayer5(1, receivedMessage);

		/* Create ACK packet */
		memset(&myackpkt, 0, sizeof(myackpkt));
		myackpkt.seqnum = expectedseqnum;
		myackpkt.acknum = expectedseqnum;

		int ackChecksum = myackpkt.acknum + myackpkt.seqnum;

		myackpkt.checksum = ackChecksum;
		tolayer3(1, myackpkt);
		expectedseqnum++;
		}
	} else {
		cout << "Packet corrupt, checksum mismatch " << packet.checksum << " " << checksum <<  endl;
		tolayer3(1, myackpkt);
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	cout << "B_init() called" << endl;
	expectedseqnum = 0;
	memset(&myackpkt, 0, sizeof(myackpkt));
}
