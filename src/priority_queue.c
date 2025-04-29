#include "../include/priority_queue.h" 

Node* createNode(pcb_t* pcb) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->pcb = pcb;
    newNode->next = NULL;
    return newNode;
}

void enqueue(Node** head, pcb_t* pcb) {
    Node* newNode = createNode(pcb);

    if (*head == NULL || pcb->priority < (*head)->pcb->priority) {
        newNode->next = *head;
        *head = newNode;
        return;
    }

    Node* current = *head;
    while (current->next != NULL && current->next->pcb->priority <= pcb->priority) {
        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
}

pcb_t* dequeue(Node** head) {
    if (*head == NULL) {
        fprintf(stderr, "Queue underflow\n");
        exit(EXIT_FAILURE);
    }

    Node* temp = *head;
    pcb_t* pcb = temp->pcb;
    *head = (*head)->next;
    free(temp);
    return pcb;
}

int isEmpty(Node* head) {
    return head == NULL;
}
