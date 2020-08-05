#include <stdio.h>
#include "../network.h"

#define RELIABLE_RESEND_RATE 0.10f

struct PacketLinkedList {
    struct Packet p;
    clock_t lastSend;
    struct PacketLinkedList* prev;
    struct PacketLinkedList* next;
};

struct PacketLinkedList* head = NULL;
struct PacketLinkedList* tail = NULL;

static void remove_node_from_list(struct PacketLinkedList* node) {
    if (node == head) {
        head = node->next;
        if (head != NULL) { head->prev = NULL; }
    }
    if (node == tail) {
        tail = node->prev;
        if (tail != NULL) { tail->next = NULL; }
    }

    if (node->prev != NULL) { node->prev->next = node->next; }
    if (node->next != NULL) { node->next->prev = node->prev; }
    free(node);
}

void network_send_ack(struct Packet* p) {
    // grab seq num
    u16 seqId = 0;
    memcpy(&seqId, &p->buffer[1], 2);
    if (seqId == 0) { return; }

    // send back the ACK
    struct Packet ack = { 0 };
    packet_init(&ack, PACKET_ACK, false);
    packet_write(&ack, &seqId, 2);
    network_send(&ack);
}

void network_receive_ack(struct Packet* p) {
    // grab seq num
    u16 seqId = 0;
    packet_read(p, &seqId, 2);

    // find in list and remove
    struct PacketLinkedList* node = head;
    while (node != NULL) {
        if (node->p.seqId == seqId) {
            remove_node_from_list(node);
            break;
        }
        node = node->next;
    }
}

void network_remember_reliable(struct Packet* p) {
    if (!p->reliable) { return; }
    if (p->sent) { return; }

    struct PacketLinkedList* node = malloc(sizeof(struct PacketLinkedList));
    node->p = *p;
    node->p.sent = true;
    node->lastSend = clock();
    node->prev = NULL;
    node->next = NULL;

    if (tail == NULL) {
        // start of the list
        assert(head == NULL);
        head = node;
        tail = node;
        return;
    }

    // add to end of list
    assert(tail->next == NULL);
    tail->next = node;
    node->prev = tail;
    tail = node;
}

void network_update_reliable(void) {
    struct PacketLinkedList* node = head;
    while (node != NULL) {
        float elapsed = (clock() - node->lastSend) / CLOCKS_PER_SEC;
        if (elapsed > RELIABLE_RESEND_RATE) {
            // resend
            network_send(&node->p);
            node->lastSend = clock();
        }
        node = node->next;
    }
}