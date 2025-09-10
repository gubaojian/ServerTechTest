#include <iostream>

#include "mongoose.h"


// HTTP server event handler function
void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        struct mg_http_serve_opts opts = { .root_dir = "./web_root/" };
        mg_http_serve_dir(c, hm, &opts);
    }
}

/**
*https://github.com/wg/wrk

 wrk -t12 -c400 -d10s  http://127.0.0.1:8000/
Running 10s test @ http://127.0.0.1:8000/
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     4.85ms    6.77ms 170.19ms   95.13%
    Req/Sec     5.88k     3.59k   14.80k    64.67%
  586547 requests in 10.11s, 0.85GB read
  Socket errors: connect 155, read 0, write 0, timeout 0
Requests/sec:  58043.67
Transfer/sec:     86.46MB
 *
 * @return
 */
int main(void) {
    struct mg_mgr mgr;  // Declare event manager
    mg_mgr_init(&mgr);  // Initialise event manager
    mg_http_listen(&mgr, "http://0.0.0.0:8000", ev_handler, NULL);  // Setup listener
    for (;;) {          // Run an infinite event loop
        mg_mgr_poll(&mgr, 1000);
    }
    return 0;
}