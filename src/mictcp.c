#include <mictcp.h>
#include <api/mictcp_core.h>
 mic_tcp_sock sock;
/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{

   int result = initialize_components(sm); 
   printf("%d\n",result) ;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   if (result==-1) {
       return -1 ;
   } else {
        set_loss_rate(9);
        sock.fd=1 ;
        sock.state = CLOSED ;
        return sock.fd;
   }
   
}

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
 * Retourne 0 si succès, -1 si erreur
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
    // mic_tcp_pdu pdu ;
    // pdu.header.syn=1 ;
    // pdu.header.source_port=5000 ;
    // pdu.header.dest_port=9000 ;
    // mic_tcp_pdu pdu2 ;
    // if (IP_send(pdu, addr1)==-1){
    //     printf("Erreur sur le syn\n") ;
    // }
    // while ((IP_recv(&pdu2, &addr, 2000)==-1)|| pdu2.header.syn==0 || pdu2.header.ack==0)  {
    //     if (IP_send(pdu, addr)==-1){
    //     printf("Erreur sur l'ack\n") ;
    // }
    // pdu.header.ack=1 ;
    // if (IP_send(pdu, addr1)==-1){
    //     printf("Erreur sur l'ack\n") ;
    //}




    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    addr = addr1 ;
    return 0;
}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int PE=0 ;
int mic_tcp_send (int mic_sock, char* mesg, int mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    if (mic_sock==sock.fd) {
        mic_tcp_pdu pdu ;
        pdu.payload.data=mesg;
        pdu.payload.size=mesg_size ;
        pdu.header.source_port=5000 ;
        pdu.header.dest_port=9000 ;
        pdu.header.seq_num=PE ;
        int size ;
        mic_tcp_pdu pdu2 ;
        if((size=IP_send(pdu,addr))==-1) {
            printf("Erreur au moment du send \n") ;
        }
        while ((IP_recv(&pdu2,&addr,30))==-1){
            if (pdu2.header.ack!=PE){
                if((size=IP_send(pdu,addr))==-1) {
                printf("Erreur au moment du send \n") ;
                exit(1) ;
                }
            }
        }
        PE++ ;
        return size ;
    }
    return -1 ;
}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
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

/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
    return -1;
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
    if (IP_send(pdu2,addr)==-1){
        printf("Erreur au moment du send\n") ;
        exit(1);
    }
    if (pdu2.header.ack_num==PA){
        PA++ ;
        app_buffer_put(pdu.payload) ;
    }
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
}
