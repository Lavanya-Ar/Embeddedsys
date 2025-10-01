#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "secrets.h"

#define ECHO_PORT 4242

static struct udp_pcb* pcb;

static void udp_recv_cb(void* arg, struct udp_pcb* upcb, struct pbuf* p, const ip_addr_t* addr, u16_t port) {
    if (!p) return;
    // Print first byte (or size)
    if (p->len > 0) {
        uint8_t b = ((uint8_t*)p->payload)[0];
        printf("RX %u bytes, first=0x%02X from %s:%u\n", p->len, b, ipaddr_ntoa(addr), port);
    } else {
        printf("RX 0 bytes from %s:%u\n", ipaddr_ntoa(addr), port);
    }
    // Echo back "OK\n"
    const char *msg = "OK\n";
    struct pbuf* q = pbuf_alloc(PBUF_TRANSPORT, (u16_t)strlen(msg), PBUF_RAM);
    if (q) {
        memcpy(q->payload, msg, strlen(msg));
        udp_sendto(upcb, q, addr, port);
        pbuf_free(q);
    }
    pbuf_free(p);
}

int main() {
    stdio_init_all();
    sleep_ms(300);
    printf("WiFi UDP Echo starting...\n");

    if (cyw43_arch_init()) {
        printf("cyw43_arch_init failed\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    printf("Connecting to WiFi SSID=%s...\n", WIFI_SSID);
    int rc = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000);
    if (rc) {
        printf("WiFi connect failed: %d\n", rc);
        return 1;
    }
    printf("Connected.\n");

    // Print IP
    const ip4_addr_t* ip = netif_ip4_addr(cyw43_state.netif);
    printf("IP: %s\n", ip4addr_ntoa(ip));
    printf("Use:  echo -n A | nc -u %s %d\n", ip4addr_ntoa(ip), ECHO_PORT);

    pcb = udp_new();
    if (!pcb) {
        printf("udp_new failed\n");
        return 1;
    }
    if (udp_bind(pcb, IP_ANY_TYPE, ECHO_PORT) != ERR_OK) {
        printf("udp_bind failed\n");
        return 1;
    }
    udp_recv(pcb, udp_recv_cb, NULL);

    while (true) {
        sleep_ms(100);
    }
}