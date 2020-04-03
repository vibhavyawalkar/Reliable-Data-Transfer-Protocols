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

/* Sender Data */
int send_base = 0;
int nextseqnum = 0;
int N_sender;
float t = 40.00;
struct senderMsg {
	char message[20];
	float start_time;
	int acked;
};
struct senderMsg senderBuffer[1001];
vector<struct msg> buffer;

/* Receiver Data */
int rcv_base = 0;
int N_receiver;

struct receiverMsg {
	char message[20];
	int acked;
};

struct receiverMsg receiverBuffer[1010];

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
	cout << "A_output called, send_base: "<< send_base << " nextseqnum :" << nextseqnum << " N_sender: " << N_sender << endl;
	buffer.push_back(message);
	while(nextseqnum < send_base + N_sender && buffer.size()!= 0) {
		struct pkt mypkt;
		memset(&mypkt, 0, sizeof(mypkt));
		mypkt.seqnum = nextseqnum;
		mypkt.acknum = 0; // Setting 2 when the ACK is not received and 1 when the ack is received
		int checksum = mypkt.acknum + mypkt.seqnum;	
		for(int i = 0; i < 20; i++) {
			mypkt.payload[i] = buffer[0].data[i];
			senderBuffer[nextseqnum].message[i] = buffer[0].data[i];
			checksum += mypkt.payload[i];
		}
		buffer.erase(buffer.begin());
		mypkt.checksum = checksum;
		cout << "Sending pkt seqno: " << mypkt.seqnum << "msg : ";
		for(int i = 0; i < 20; i++)
			cout << mypkt.payload[i];
		cout << " checksum:" << mypkt.checksum;
		senderBuffer[nextseqnum].acked = 2;
		senderBuffer[nextseqnum].start_time = get_sim_time(); /* time when the packet was sent */ 
		cout << " send time: " << get_sim_time() << endl;
		tolayer3(0, mypkt);
		/*if(send_base == nextseqnum)
			starttimer(0, 1.00); */
		nextseqnum++;
	}
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	cout << "A_input called" << endl;
	int checksum = packet.acknum + packet.seqnum;
	if(checksum == packet.checksum) {
		cout << "ACK not corrupt, received for seqnum:" << packet.acknum << endl;
		//if(packet.acknum < send_base + nextseqnum/*N_sender*/) {
		/* Mark the packet as received/acked, if it lies in the window */
			senderBuffer[packet.acknum].acked = 1;
		//}
		if(packet.acknum == send_base) {

		/* The window base if moved forward to the unacknowledged packet with the smallest sequence number*/
			//for(int i = send_base; i < nextseqnum/*send_base + N_sender*/; i++)
			//{
				while(senderBuffer[send_base].acked == 1) {
					send_base++;
					cout << "Incremented send_base : " << send_base << endl;
				}
			//}

		/* Send all the buffered packets when the window is moved */
			while(nextseqnum < send_base + N_sender && buffer.size() != 0) {
				cout << "Sending all the buffered packets when the window is moved" << endl;
                		struct pkt mypkt;
                		memset(&mypkt, 0, sizeof(mypkt));
                		mypkt.seqnum = nextseqnum;
                		mypkt.acknum = 0; // Setting 2 when the ACK is not received and 1 when the ack is received
                		int checksum = mypkt.acknum + mypkt.seqnum;
                		for(int j = 0; j < 20; j++) {
					mypkt.payload[j] = buffer[0].data[j];
                        		senderBuffer[nextseqnum].message[j] = buffer[0].data[j];
                        		checksum += mypkt.payload[j];
                		}
				buffer.erase(buffer.begin());
                		mypkt.checksum = checksum;
                		cout << "Sending pkt seqno: " << mypkt.seqnum << "msg : ";
                		for(int j = 0; j < 20; j++)
                        		cout << mypkt.payload[j];
                		cout << " checksum:" << mypkt.checksum;
                		senderBuffer[nextseqnum].acked = 2;
                		senderBuffer[nextseqnum].start_time = get_sim_time(); /* time when the packet was sent */
                		cout << " send time: " << get_sim_time() << endl;
                		tolayer3(0, mypkt);
                		/*if(send_base == nextseqnum)
                         	starttimer(0, 1.00); */
                		nextseqnum++;
        		}

		}
		
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	cout << "A_timerinterrupt called at " << get_sim_time() << endl;
	cout << "Value of send base " << send_base << endl;
	for(int i = send_base; i < nextseqnum; i++)
	{
		float diff = get_sim_time() - senderBuffer[i].start_time;
		//cout << "Sendbuffer start time: " << senderBuffer[i].start_time << endl;
		//cout << "diff: " << abs((int)diff) << " " << t << endl;
		if(senderBuffer[i].acked == 2 && diff >= (int)(t))
		{
			cout << "diff: " << diff << endl;
			/* Resend the packets who's ack is lost or not received by the receiver */
			struct pkt mypkt;
                	memset(&mypkt, 0, sizeof(mypkt));
                	mypkt.seqnum = i;
                	mypkt.acknum = 0;
			cout << "Send Buffer contains ";
                	int checksum = mypkt.acknum + mypkt.seqnum;
                	for(int j = 0; j < 20; j++) {
                        	mypkt.payload[j] = senderBuffer[i].message[j];
				cout << senderBuffer[i].message[j];
                        	checksum += mypkt.payload[j];
                	}
			cout << endl;
                	mypkt.checksum = checksum;
                	cout << "Sending pkt seqno: " << mypkt.seqnum << "msg : ";
                	for(int j = 0; j < 20; j++)
                        	cout << mypkt.payload[j];
                	cout << " checksum:" << mypkt.checksum << endl;
                	senderBuffer[i].acked = 2;
                	senderBuffer[i].start_time = get_sim_time(); /* time when the packet was sent */
                	tolayer3(0, mypkt); 
		}
	}
	//stoptimer(0);
	starttimer(0, 1.00);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	cout << "A_init called" << endl;
	send_base = 0;
	nextseqnum = 0;
	N_sender = getwinsize();
	starttimer(0, 1.00);
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	cout << "B_input called" << endl;
	int checksum = packet.acknum + packet.seqnum;
	for(int i = 0; i < 20; i++)
		checksum += packet.payload[i];

	if(packet.checksum == checksum) {
		cout << "Packet received ";
        	for(int j = 0; j < 20; j++) {
                	receiverBuffer[packet.seqnum].message[j] = packet.payload[j];
			cout << packet.payload[j];
		}
		cout << endl;

		if(packet.seqnum >= rcv_base && packet.seqnum < rcv_base + N_receiver) {
			struct pkt myackpkt;
			memset(&myackpkt, 0, sizeof(myackpkt));
			myackpkt.seqnum = packet.seqnum;
			myackpkt.acknum = packet.seqnum;
			int ackChecksum = myackpkt.acknum + myackpkt.seqnum;
			myackpkt.checksum = ackChecksum;
			tolayer3(1, myackpkt);
			receiverBuffer[packet.seqnum].acked = 1;
			if(packet.seqnum == rcv_base)
			{
				for(int k = rcv_base; k < rcv_base + N_receiver; k++)
				{
					if(receiverBuffer[k].acked == 1)
					{
						tolayer5(1, receiverBuffer[k].message);	
						rcv_base++;
					} else {
						break;
					}
				}
			}
		} else if(packet.seqnum <= rcv_base-1 && packet.seqnum >= rcv_base-N_receiver) {
			struct pkt myackpkt;
			memset(&myackpkt, 0, sizeof(myackpkt));
			myackpkt.seqnum = packet.seqnum;
			myackpkt.acknum = packet.seqnum;
			int ackChecksum = myackpkt.acknum + myackpkt.seqnum;
			myackpkt.checksum = ackChecksum;
			tolayer3(1, myackpkt);
		}
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	cout << "B_init called" << endl;
	rcv_base = 0;
	N_receiver = getwinsize();
	//memset(receiverBuffer, 0, sizeof(receiverBuffer));
}
