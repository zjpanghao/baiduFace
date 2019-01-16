/*
  A trivial static http webserver using Libevent's evhttp.

  This is not the best code in the world, and it does some fairly stupid stuff
  that you would never want to do in a production webserver. Caveat hackor!

 */

/* Compatibility for possible missing IPv6 declarations */
//#include "../util-internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif
#else
#include <sys/stat.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

#ifdef EVENT__HAVE_NETINET_IN_H
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#endif
#include "event2/http_compat.h"
#include "json/json.h"
#include "image_base64.h"
#include "event2/http.h"

#include<opencv2/opencv.hpp>
#include<opencv/highgui.h>

#ifdef _WIN32
#ifndef stat
#define stat _stat
#endif
#ifndef fstat
#define fstat _fstat
#endif
#ifndef open
#define open _open
#endif
#ifndef close
#define close _close
#endif
#ifndef O_RDONLY
#define O_RDONLY _O_RDONLY
#endif
#endif
#include <thread>
#include <vector>
#include <glog/logging.h>
#include <iterator>
#include <regex>
#include "faceAgent.h"
#include "faceService.h"
#include "httpUtil.h"
#include "userControl.h"
#include "faceControl.h"


#define MAX_IMG_SIZE 1024*1024*50
using namespace kface;
using namespace cv;

char uri_root[512];

struct HttpParam {
  struct event_base *base;
};

void httpThread(void *param) {
  LOG(INFO) << "new Thread";
  struct event_base *base = ((struct HttpParam *)param)->base;
  event_base_dispatch(base);
}

void ev_server_start_multhread(int port, int nThread) {
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return;
  evutil_socket_t fd;
  struct evconnlistener *listener = NULL;
  for (int i = 0; i < nThread; i++) {
    struct event_base *base = event_base_new();
  	if (!base) {
  		LOG(ERROR) << "Couldn't create an event_base: exiting";
  		return;
  	}

      /* Create a new evhttp object to handle requests. */
  	struct evhttp *http = evhttp_new(base);
  	if (!http) {
  		LOG(ERROR) << "couldn't create evhttp. Exiting.";
  		return;
  	}
     /* The /dump URI will dump all requests to stdout and say 200 ok. */
    //evhttp_set_cb(http, "/identify", identifyCb, (void*)arg);
    std::vector<HttpControl> controls;
    initUserControl(controls);
    initFaceControl(controls);
    for (HttpControl control : controls) {
      evhttp_set_cb(http, control.url.c_str(), control.cb, NULL);
    }
    if (i == 0) {
      struct evhttp_bound_socket *bound;
      bound = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);
      if (!bound) {
        return;
      }
      fd = evhttp_bound_socket_get_fd(bound);
    } else {
      evhttp_accept_socket(http, fd);
    }
    HttpParam *param = new HttpParam();
    param->base = base;
    std::thread t(httpThread, param);
    t.detach();    
  }
}


int ev_server_start(int port)
{
	struct event_base *base;
	struct evhttp *http;
	struct evhttp_bound_socket *handle;

#ifdef _WIN32
	WSADATA WSAData;
	WSAStartup(0x101, &WSAData);
#else
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return (1);
#endif

	base = event_base_new();
	if (!base) {
		fprintf(stderr, "Couldn't create an event_base: exiting\n");
		return 1;
	}

	/* Create a new evhttp object to handle requests. */
	http = evhttp_new(base);
	if (!http) {
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");
		return 1;
	}

	/* The /dump URI will dump all requests to stdout and say 200 ok. */
  std::vector<HttpControl> controls;
  initUserControl(controls);
  initFaceControl(controls);
  for (HttpControl control : controls) {
    evhttp_set_cb(http, control.url.c_str(), control.cb, NULL);
  }

	/* We want to accept arbitrary requests, so we need to set a "generic"
	 * cb.  We can also add callbacks for specific paths. */
	//evhttp_set_gencb(http, send_document_cb, argv[1]);

	/* Now we tell the evhttp what port to listen on */
	handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);
	if (!handle) {
		fprintf(stderr, "couldn't bind to port %d. Exiting.\n",
		    (int)port);
		return 1;
	}

	{
		/* Extract and display the address we're listening on. */
		struct sockaddr_storage ss;
		evutil_socket_t fd;
		ev_socklen_t socklen = sizeof(ss);
		char addrbuf[128];
		void *inaddr;
		const char *addr;
		int got_port = -1;
		fd = evhttp_bound_socket_get_fd(handle);
		memset(&ss, 0, sizeof(ss));
		if (getsockname(fd, (struct sockaddr *)&ss, &socklen)) {
			perror("getsockname() failed");
			return 1;
		}
		if (ss.ss_family == AF_INET) {
			got_port = ntohs(((struct sockaddr_in*)&ss)->sin_port);
			inaddr = &((struct sockaddr_in*)&ss)->sin_addr;
		} else if (ss.ss_family == AF_INET6) {
			got_port = ntohs(((struct sockaddr_in6*)&ss)->sin6_port);
			inaddr = &((struct sockaddr_in6*)&ss)->sin6_addr;
		} else {
			fprintf(stderr, "Weird address family %d\n",
			    ss.ss_family);
			return 1;
		}
		addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf,
		    sizeof(addrbuf));
		if (addr) {
			printf("Listening on %s:%d\n", addr, got_port);
			evutil_snprintf(uri_root, sizeof(uri_root),
			    "http://%s:%d",addr,got_port);
		} else {
			fprintf(stderr, "evutil_inet_ntop failed\n");
			return 1;
		}
	}

	event_base_dispatch(base);

	return 0;
}
