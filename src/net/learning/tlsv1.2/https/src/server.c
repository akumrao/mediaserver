//SSL-Server.c
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <resolv.h>
#include "openssl/ssl.h"
#include "openssl/err.h"

#define FAIL    -1

int OpenListener(int port) {
    int sd;
    struct sockaddr_in addr;

    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sd, (struct sockaddr*) &addr, sizeof (addr)) != 0) {
        perror("can't bind port");
        abort();
    }
    if (listen(sd, 10) != 0) {
        perror("Can't configure listening port");
        abort();
    }
    return sd;
}

SSL_CTX* InitServerCTX(void) {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms(); /* load & register all cryptos, etc. */
    SSL_load_error_strings(); /* load all error messages */

    method = TLSv1_2_server_method();
    ctx = SSL_CTX_new(method); /* create new context from method */
    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        abort();
    }

    SSL_CTX_set_cipher_list(ctx, "ALL:eNULL");

    return ctx;
}

void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile) {
    //New lines 
    if (SSL_CTX_load_verify_locations(ctx, CertFile, KeyFile) != 1)
        ERR_print_errors_fp(stderr);

    if (SSL_CTX_set_default_verify_paths(ctx) != 1)
        ERR_print_errors_fp(stderr);
    //End new lines

    /* set the local certificate from CertFile */
    if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* set the private key from KeyFile (may be the same as CertFile) */
    SSL_CTX_set_default_passwd_cb_userdata(ctx, "12345678");
    if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* verify private key */
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }

    //New lines - Force the client-side have a certificate
    //SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
    //SSL_CTX_set_verify_depth(ctx, 4);
    //End new lines
}

void ShowCerts(SSL* ssl) {
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if (cert != NULL) {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    } else
        printf("No certificates.\n");
}

void Servlet(SSL* ssl) /* Serve the connection -- threadable */ {
    char buf[1024];
    int sd, bytes;

    char enter[3] = {0x0d, 0x0a, 0x00};

    char output[1024];
    strcpy(output, "HTTP/1.1 200 OK");
    strcat(output, enter);
    strcat(output, "Content-Type: text/html");
    strcat(output, enter);
    strcat(output, "Content-Length: 47");
    strcat(output, enter);
    strcat(output, enter);
    strcat(output, "<html><body><h1>Hello World!</h1></body></html>");

    if (SSL_accept(ssl) == FAIL) /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    else {
        ShowCerts(ssl); /* get any certificates */
        bytes = SSL_read(ssl, buf, sizeof (buf)); /* get request */
        if (bytes > 0) {
            buf[bytes] = 0;
            printf("Client msg: \"%s", buf);
            SSL_write(ssl, output, strlen(output)); /* send reply */
        } else
            ERR_print_errors_fp(stderr);
    }
    sd = SSL_get_fd(ssl); /* get socket connection */
    SSL_free(ssl); /* release SSL state */
    close(sd); /* close connection */
}

int main(int argc, char **argv) {
    SSL_CTX *ctx;
    int server;
    char portnum[] = "5001";

    char CertFile[] = "key/certificate.crt";
    char KeyFile[] = "key/private_key.pem";

    SSL_library_init();

    ctx = InitServerCTX(); /* initialize SSL */
    LoadCertificates(ctx, CertFile, KeyFile); /* load certs */
    server = OpenListener(atoi(portnum)); /* create server socket */
    while (1) {
        struct sockaddr_in addr;
        socklen_t len = sizeof (addr);
        SSL *ssl;

        int client = accept(server, (struct sockaddr*) &addr, &len); /* accept connection as usual */
        printf("Connection: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        ssl = SSL_new(ctx); /* get new SSL state with context */
        SSL_set_fd(ssl, client); /* set connection socket to SSL state */
        Servlet(ssl); /* service connection */
    }
    close(server); /* close server socket */
    SSL_CTX_free(ctx); /* release context */
}

