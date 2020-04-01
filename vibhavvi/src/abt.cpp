#include<iostream>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<vector>
#include "../include/simulator.h"
using namespace std;
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

struct pkt mypkt; 	// Packet sent by the sender A
struct pkt myackpkt;	// ACK packet to be sent from receiver B
static int seqSender;
static int seqSenderAck;
static int seqReceiver;
static float t = 11.00;

vector<struct msg> buffer;

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
	cout << "A_output() called" << endl;
	buffer.push_back(message);
	if(seqSender == seqSenderAck) { // If the sending sequence number and the expected Acknumber match then send packet.
		memset(&mypkt, 0, sizeof(mypkt));
		mypkt.seqnum = seqSender;
		mypkt.acknum = -1;
		if(buffer.size() != 0)
		{	
			cout << "Sending buffered message ";
			for(int i = 0; i < 20; i++)
				cout << buffer[0].data[i];
			cout << endl;

			for(int i = 0; i < 20; i++)
				mypkt.payload[i] = buffer[0].data[i];
			buffer.erase(buffer.begin());

		} else {
			cout << "Sending message ";
			for(int i = 0; i < 20; i++)
				cout << message.data[i];
			cout << endl;

 			for (int i=0; i<20; i++)
				mypkt.payload[i] = message.data[i];
		}
		// Need to calculate the checksum of the entire packet here
		int checksum = mypkt.acknum + mypkt.seqnum;
		int payloadChecksum;
		for(int i = 0; i < 20; i++)
			payloadChecksum += mypkt.payload[i];

		checksum += payloadChecksum;

		mypkt.checksum = checksum;
		cout << "Sender side checksum:" << checksum << endl;
		tolayer3(0, mypkt);
		seqSenderAck = mypkt.seqnum;
		// Send the packet and start the timer for the ACK
		starttimer(0, t);
		if(seqSender == 0)
			seqSender = 1;
		else
			seqSender = 0;
	}
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	cout << "A_input() called" << endl;

	/* Check if the ACK is corrupted, verify the checksum */
	int checksum = packet.acknum + packet.seqnum;
	if(checksum == packet.checksum) {
		if(packet.seqnum == seqSenderAck) {
			cout << "Ack received successfully for seq:" << packet.seqnum <<  endl;
			stoptimer(0); // Successfully received acknowledgement
			if(seqSenderAck == 0)
				seqSenderAck = 1;
			else
				seqSenderAck = 0;
		} else {
			cout << "Ack received for the unexpected seq:" << packet.seqnum << endl;			
		}
	} else {
		cout << "Ack corrupted" << endl;
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	cout << "A_timerinterrupt() called" << endl;
	// Retransmit the previously sent packet
	tolayer3(0, mypkt);
	seqSenderAck = mypkt.seqnum;
	starttimer(0, t);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	cout << "A_init() called" << endl;
	seqSender = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	cout << "B_input() called" << endl;

	// validate the checksum of the packet
	int checksum = packet.acknum + packet.seqnum;
	int payloadChecksum;
	for(int i = 0; i < 20; i++)
		payloadChecksum += packet.payload[i];

	checksum += payloadChecksum;

	cout << "Receiver side checksum:" << checksum << endl;

	if(packet.checksum == checksum) {
		if(seqReceiver == packet.seqnum) // Receives the expected sequence number
		{
			// if checksum is correct send packetdata to application layer and send ACK to A

			char receivedMessage[20];
        		memset(&receivedMessage, 0, sizeof(receivedMessage));
        		for(int i = 0; i < 20; i++)
                		receivedMessage[i] = packet.payload[i];

			// Deliver data to the application layer
			tolayer5(1, receivedMessage);
        		cout << "Received message ";
        		for(int i = 0; i < 20; i++)
        			cout << receivedMessage[i];
        		cout << endl;
			if(seqReceiver == 0)
				seqReceiver = 1;
			else
				seqReceiver = 0;

			// Create ACK packet
			memset(&myackpkt, 0, sizeof(myackpkt));
                        myackpkt.seqnum = packet.seqnum;
                        myackpkt.acknum = 1;

                        int ackChecksum = myackpkt.acknum + myackpkt.seqnum;

                        cout << "Receiver side ACK checksum:" << ackChecksum << endl;

                        myackpkt.checksum = ackChecksum;
                        tolayer3(1, myackpkt);

		} else {
			/*
			memset(&myackpkt, 0, sizeof(myackpkt));
                	myackpkt.seqnum = packet.seqnum;
                	myackpkt.acknum = 1;

                	int ackChecksum = myackpkt.acknum + myackpkt.seqnum;

                	cout << "Receiver side ACK checksum:" << ackChecksum << endl;

                	myackpkt.checksum = ackChecksum;*/
                	tolayer3(1, myackpkt);
		}
	} else {
        	/* The packet is corrupted, send the acknowledgement for the
                   previous packet which was received successfully, so that the
	           sender resends/retransmits */
		//cout << "Packet is corrupted, send the previous ACK" << endl;
		//tolayer3(1, myackpkt);
        }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	cout << "B_init() called" << endl;
	seqReceiver = 0;
}
