#include <mictcp.h>
#include <api/mictcp_core.h>

#define FENETRE 20

mic_tcp_sock sock;


/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'pkt_loss
 */
int mic_tcp_socket(start_mode sm)
{

   int result = initialize_components(sm); 
   printf("%d\n",result) ;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   if (result==-1) {
       return -1 ;
   } else {
        set_loss_rate(0);
        sock.fd=1 ;
        sock.state = IDLE ;
        return sock.fd;
   }
   
}
//Taux accept=nombre d'erreur acceptable sur la fenetre donnée, ici, 5 erreur sont acceptables sur la fenetre de 20.
float taux_accept = 1 ;
//int FENETRE = 5 ;
/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
{
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   return 0;
}

/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si pkt_loss
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    return 0;
}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
mic_tcp_sock_addr addr ;
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr1)
{
   mic_tcp_pdu pdu = {0} ;
   printf("Entrée dans le connect\n") ;
    int boolean=1 ;
    pdu.header.syn=1 ;
    pdu.header.source_port=5000 ;
    pdu.header.dest_port=9000 ;
    mic_tcp_pdu pdu2 ;
    if (IP_send(pdu, addr1)==-1){
        printf("pkt_loss sur le syn\n") ;
    }
    sock.state=SYN_SENT ;
    while (boolean){
        if (IP_recv(&pdu2, &addr, 5)==-1){
            if (IP_send(pdu, addr1)==-1){
            printf("pkt_loss sur le syn\n") ;
            }
        }
        else if (pdu2.header.syn==0 || pdu2.header.ack==0){
            if (IP_send(pdu, addr1)==-1){
            printf("pkt_loss sur le syn\n") ;
            }
        }
        else {
            sock.state=ESTABLISHED;
            boolean=0 ;
        }
    }
    pdu.header.ack=1 ;
    if (IP_send(pdu, addr1)==-1){
    printf("pkt_loss sur l'ack\n") ;
    }
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    addr = addr1 ;
    return 0;
}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'pkt_loss
 */
int PE=0 ; 
int cpt_pkt = 0;


int somme (int tab[], int size){
    int som=0 ;
    for (int i=0; i<size ; i++){
        som+=tab[i] ;
    }
    return som ;
}


int mic_tcp_send (int mic_sock, char* mesg, int mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    static int tabl[FENETRE] = {0};
    if (mic_sock == sock.fd) {
        
        mic_tcp_pdu pdu ;
        
        // Construction du pdu
        pdu.payload.data=mesg;
        pdu.payload.size=mesg_size ;
        pdu.header.source_port=5000 ;
        pdu.header.dest_port=9000 ;
        pdu.header.seq_num=PE ;
        
        int size ;
        mic_tcp_pdu pdu2 = {0} ;
        
        int boolean=1;

        /* Réinitialisatiion du compteur  */
        
        if (cpt_pkt == FENETRE){
            cpt_pkt = 0 ;
        }

        /* Transmission du payload */
        
        if((size=IP_send(pdu,addr))==-1) {
            printf("pkt_loss au moment du send \n") ;
        }

        while (boolean){

            /* Attente d'un ack */
            
            if (IP_recv(&pdu2,&addr,30) == -1){
                if (somme(tabl, FENETRE) < (taux_accept)){
                    boolean = 0 ;
                    tabl[cpt_pkt]=1 ;
                    boolean = 0 ;
                }
                else if((size=IP_send(pdu,addr))==-1) {
                    printf("pkt_loss au moment du send \n") ;
                    exit(1) ;
                }
            }
            else {
                tabl[cpt_pkt]=0 ;
                boolean=0 ;
                PE++ ;
            }
        }
        cpt_pkt++ ;
        printf("%d,\n", cpt_pkt) ;
        printf("%d,\n", somme(tabl,FENETRE)) ;
        return size ;
    }
    return -1 ;
}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’pkt_loss
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    mic_tcp_pdu pdu ;
    int size=-1 ;
    if (socket==sock.fd){
        pdu.payload.data=mesg ;
        pdu.payload.size= max_mesg_size ;
        size=app_buffer_get(pdu.payload) ;
    }
    return size;
}

int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
    return 0;
}

/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
int PA=0 ;
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr)
{
    mic_tcp_pdu pdu2 ;
    pdu2.header.ack_num=pdu.header.seq_num ;
    pdu2.header.source_port=9000 ;
    pdu2.header.dest_port=5000 ;
    
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");

    switch (sock.state){
        case IDLE : 
            if(pdu.header.syn && !pdu.header.ack) {
                pdu.header.ack=1 ;
                sock.state=ESTABLISHED ;
                IP_send(pdu, addr) ;
            }
            break ;
        case ESTABLISHED :
            if(pdu.header.syn && !pdu.header.ack) {
                pdu.header.ack=1 ;
                IP_send(pdu, addr) ;
            }
            else if (IP_send(pdu2,addr)==-1){
                    printf("pkt_loss au moment du send\n") ;
                    exit(1);
            }
            else if (pdu2.header.ack_num == PA && !pdu.header.ack){
                    PA+=pdu2.header.ack_num-PA+1 ;
                    app_buffer_put(pdu.payload) ;
            }
            break ;

        default : 
            break ;
    }

    //pdu récu

    // Désencapsuler

    // Vérifie est-ce que c'est le paquet à transmettre à l'application ==> Vérification numéro de séquence

    //Si c'est bon ==> app_buffer_put
    // Envoi d'un ack

    //sinon envoi un ack avec le dernier numéro de séquence
}
