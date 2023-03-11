
#include <sys/epoll.h>
#include "async.h"
#include "log.h"



static void* async_task_proc(void *arg) 
{
	async_context_t *ctx = (async_context_t*)arg;
	int i = 0;

	int epfd = ctx->epfd;

	while (1) {

		struct epoll_event events[ASYNC_CLIENT_NUM] = {0};

		int nready = epoll_wait(epfd, events, ASYNC_CLIENT_NUM, -1);
		if (nready < 0) {
			if (errno == EINTR || errno == EAGAIN) {
				continue;
			} else {
				break;
			}
		} else if (nready == 0) {
			continue;
		}

		//DHCP_LOG_DEBUG("nready:%d", nready);
	
		for (i = 0;i < nready;i ++) {

			struct ep_arg *data = (struct ep_arg*)events[i].data.ptr;
			int sockfd = data->sockfd;

			char buffer[1024] = {0};
			struct sockaddr_in addr;
			size_t addr_len = sizeof(struct sockaddr_in);
			int buffLen = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, (socklen_t*)&addr_len);

			void * para = NULL;
            
            /*包解析回调*/
			para = ctx->cb(buffer,buffLen);

            /*回复处理回调*/
			if(para != NULL)
				data->cb(para); //call cb

		}

	}

}


async_context_t * async_task_init(int sockfd,async_result_cb asyncCb,packet_parse_cb packetCb) 
{
	int ret = 0;
	pthread_t thread_id;
	struct epoll_event ev;

	int epfd = epoll_create(1); 
	if (epfd < 0) return NULL;

	async_context_t *ctx = calloc(1, sizeof(struct async_context));
	if (ctx == NULL) {
		close(epfd);
		return NULL;
	}
	ctx->epfd = epfd;
    ctx->cb   = packetCb;

	struct ep_arg *eparg = (struct ep_arg*)calloc(1, sizeof(struct ep_arg));
	if (eparg == NULL) return NULL;
	eparg->sockfd = sockfd;
	eparg->cb = asyncCb;

	ev.data.ptr = eparg;
	ev.events = EPOLLIN;

	ret = epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, sockfd, &ev); 


	ret = pthread_create(&thread_id, NULL, async_task_proc, ctx);
	if (ret) {
		DHCP_LOG_ERROR("pthread_create failed");
		return NULL;
	}

	return ctx;
}
