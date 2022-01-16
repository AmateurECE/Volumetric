///////////////////////////////////////////////////////////////////////////////
// NAME:            ev-server.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     nghttp2 server test
//
// CREATED:         01/10/2022
//
// LAST EDITED:     01/10/2022
//
// Copyright 2022, Ethan D. Twardy
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////

static void start_listen(struct event_base *evbase, const char *service,
                         app_context *app_ctx) {
  int rv;
  struct addrinfo hints;
  struct addrinfo *res, *rp;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
#ifdef AI_ADDRCONFIG
  hints.ai_flags |= AI_ADDRCONFIG;
#endif /* AI_ADDRCONFIG */

  rv = getaddrinfo(NULL, service, &hints, &res);
  if (rv != 0) {
    errx(1, "Could not resolve server address");
  }
  for (rp = res; rp; rp = rp->ai_next) {
    struct evconnlistener *listener;
    listener = evconnlistener_new_bind(
        evbase, acceptcb, app_ctx, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
        16, rp->ai_addr, (int)rp->ai_addrlen);
    if (listener) {
      freeaddrinfo(res);

      return;
    }
  }
  errx(1, "Could not start listener");
}

int main(int argc, char **argv) {
  struct sigaction act;

  if (argc < 2) {
    fprintf(stderr, "Usage: libevent-server \n");
    exit(1);
  }

  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &act, NULL);

  app_context app_ctx;
  struct event_base *evbase;

  evbase = event_base_new();
  memset(app_ctx, 0, sizeof(app_context));
  app_ctx->evbase = evbase;
  start_listen(evbase, argv[1], &app_ctx);
  event_base_loop(evbase, 0);
  event_base_free(evbase);

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
