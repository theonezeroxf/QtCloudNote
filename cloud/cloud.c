#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXSIZE 2048

// ��ȡһ�� \r\n ��β������ 

int get_line(int cfd, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;
    while ((i < size-1) && (c != '\n')) {  
        n = recv(cfd, &c, 1, 0);
        if (n > 0) {     
            if (c == '\r') {            
                n = recv(cfd, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n')) {              
                    recv(cfd, &c, 1, 0);
                } else {                       
                    c = '\n';
                }
            }
            buf[i] = c;
            i++;
        } else {      
            c = '\n';
        }
    }
    buf[i] = '\0';
    
    if (-1 == n)
    	i = n;

    return i;
}

void do_accept(int lfd, int epfd)
{
	struct sockaddr_in clt_addr;
    socklen_t clt_addr_len = sizeof(clt_addr);
    
    int cfd = accept(lfd, (struct sockaddr*)&clt_addr, &clt_addr_len);
    if (cfd == -1) {   
        perror("accept error");
        exit(1);
    }

    // ��ӡ�ͻ���IP+port
    char client_ip[64] = {0};
    printf("New Client IP: %s, Port: %d, cfd = %d\n",
           inet_ntop(AF_INET, &clt_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
           ntohs(clt_addr.sin_port), cfd);

    // ���� cfd ������
    int flag = fcntl(cfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(cfd, F_SETFL, flag);

    // ���½ڵ�cfd �ҵ� epoll ��������
    struct epoll_event ev;
    ev.data.fd = cfd;
    
    // ���ط�����ģʽ
    ev.events = EPOLLIN | EPOLLET;
    
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    if (ret == -1)  {
        perror("epoll_ctl add cfd error");
        exit(1);
    }
}

// �Ͽ�����
void disconnect(int cfd, int epfd)
{
	int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
	if (ret != 0) {
		perror("epoll_ctl error");	
		exit(1);
	}
	close(cfd);
}

int init_listen_fd(int port, int epfd)
{
    //�������������׽��� lfd
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {    
        perror("socket error");
        exit(1);
    }
    // ������������ַ�ṹ IP+port
    struct sockaddr_in srv_addr;
    
    bzero(&srv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // �˿ڸ���
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // �� lfd �󶨵�ַ�ṹ
    int ret = bind(lfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if (ret == -1) {   
        perror("bind error");
        exit(1);
    }
    // ���ü�������
    ret = listen(lfd, 128);
    if (ret == -1) { 
        perror("listen error");
        exit(1);
    }
    
    // lfd ��ӵ� epoll ����
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if (ret == -1) { 
        perror("epoll_ctl add lfd error");
        exit(1);
    }

    return lfd;
}

// �ͻ��˶˵�fd, ����ţ������������ط��ļ����ͣ� �ļ����� 
void send_respond(int cfd, int no, char *disp, char *type, int len)
{
	char buf[4096] = {0};
	
	sprintf(buf, "Cloud/1.1 %d %s\r\n", no, disp);
	send(cfd, buf, strlen(buf), 0);
	
	sprintf(buf, "Content-Type: %s\r\n", type);
	sprintf(buf+strlen(buf), "Content-Length:%d\r\n", len);
	send(cfd, buf, strlen(buf), 0);
	
	send(cfd, "\r\n", 2, 0);
}

void send_file(int cfd,char *file){
	int n = 0, ret;
	char buf[4096] = {0};
	
	// �򿪵ķ����������ļ���  --- cfd �ܷ��ʿͻ��˵� socket
	int fd = open(file, O_RDONLY);
	if (fd == -1) {
		perror("open error");
		exit(1);
	}
	
	while ((n = read(fd, buf, sizeof(buf))) > 0) {		
		ret = send(cfd, buf, n, 0);
		if (ret == -1) {
			perror("send error");	
			exit(1);
		}
		if (ret < 4096)		
			printf("-----send ret: %d\n", ret);
	}

	
	close(fd);		

}

void get_request(int cfd,char *file)
{
	struct stat sbuf;
	
	// �ж��ļ��Ƿ����
	int ret = stat(file, &sbuf);
	if (ret != 0) {
		// �ط������ 404 ����ҳ��
		perror("stat");
		exit(1);	
	}
	
	if(S_ISREG(sbuf.st_mode)) {		// ��һ����ͨ�ļ�

		//send_respond(cfd, 200, "OK", " Content-Type: text/plain; charset=iso-8859-1", sbuf.st_size);	 
		send_respond(cfd, 200, "OK", "Content-Type:image/jpeg", -1);
		//send_respond(cfd, 200, "OK", "audio/mpeg", -1);
		
		// �ط� ���ͻ��������������ݡ�
		send_file(cfd, file);
	}	
}


void doSave(int cfd,char *file){
	int fd = open(file, O_RDWR|O_CREAT,0664);
	char buf[4096] = {0};
	if (fd == -1) {
		perror("open error");
		exit(1);
	}
	while (1) {
		memset(buf,'0',sizeof(buf));
		int n = recv(cfd, buf, sizeof(buf),0);
		if(buf[0]=='\n')	break;
		printf("recv:%s n=%d\n",buf,strlen(buf));

		int ret=write(fd,buf,strlen(buf));
		if (ret == -1) {
			perror("write error");	
			exit(1);
		}
		if (ret < 4096)		
			printf("-----recv ret: %d\n", ret);
	}
	close(fd);
	printf("=====%s=====save ok\n",file);
}

void do_read(int cfd,int epfd){
	char line[1024] = {0};
	//method filename protocol ��"save file1.txt 100B cloud"
	char method[16], filename[256],lenth[16], protocol[16]; 
	int len=get_line(cfd,line,sizeof(line));
	if (len == 0) {
		printf("����������鵽�ͻ��˹ر�....\n");	
		disconnect(cfd, epfd);
	} else {
				
		sscanf(line, "%[^ ] %[^ ] %[^ ] %[^ ]", method, filename,lenth,protocol);	
		printf("method=%s, filename=%s,len=%s ,protocol=%s\n", method,filename,lenth,protocol);
		
		while (1) {
			char buf[1024] = {0};
			len = get_line(cfd, buf, sizeof(buf));	
			if (buf[0] == '\n') {
				break;	
			} else if (len == -1)
				break;
		}
		char *file=filename;
		if(strncasecmp(method, "save", 4) == 0){
			doSave(cfd,file);
		}else if(strncasecmp(method, "get", 3) == 0){
			get_request(cfd,file);
		}
		disconnect(cfd, epfd);
	}
}

void epoll_run(int port)
{
        int i = 0;
    struct epoll_event all_events[MAXSIZE];

    // ����һ��epoll��������
    int epfd = epoll_create(MAXSIZE);
    if (epfd == -1) {
        perror("epoll_create error");
        exit(1);
    }

    // ����lfd���������������
    int lfd = init_listen_fd(port, epfd);
     while (1) {
    	// �����ڵ��Ӧ�¼�
        int ret = epoll_wait(epfd, all_events, MAXSIZE, 0);
        if (ret == -1) {      
            perror("epoll_wait error");
            exit(1);
        }

        for (i=0; i<ret; ++i) {
        	    
            // ֻ������¼�, �����¼�Ĭ�ϲ�����
            struct epoll_event *pev = &all_events[i];
            
            // ���Ƕ��¼�
            if (!(pev->events & EPOLLIN)) {                     
                continue;
            }
            if (pev->data.fd == lfd) {   	// ������������   
                
                do_accept(lfd, epfd);
                
            } else {						// ������
                
				do_read(pev->data.fd, epfd);
            }
        }
    }
}





int main(int argc, char *argv[])
{
    // �����в�����ȡ �˿� �� server�ṩ��Ŀ¼
    if (argc < 3)
    {
        printf("./server port path\n");
    }

    // ��ȡ�û�����Ķ˿�
    int port = atoi(argv[1]);

    // �ı���̹���Ŀ¼
    int ret = chdir(argv[2]);
    if (ret != 0) {
        perror("chdir error");
        exit(1);
    }
	// ���� epoll����
	epoll_run(port);

    return 0;

}