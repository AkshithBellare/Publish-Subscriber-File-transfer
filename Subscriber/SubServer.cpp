#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <string.h>
#include <netdb.h>

using namespace std;

#define SERV_PORT 8000
#define SUB_PUB_PORT 7000

string Publisher_IP;
string KEY;

class SubscriberConnection{
public:
    char buffer[4098]; //this buffer is used to receive messages
    struct sockaddr_in server_address;
    int SubscriberSockfd; 
    int SubToPubSockfd; //socket for publisher to subscriber connection
    struct sockaddr_in publisher_as_server;
    
    //server side functions
    void connectToServer(char* argv);
    void serverShowsListOfTopics();
    void subscribe();
    void ReceiveMessageFromServer();
    string ReceiveKeyFromServer();
    void SendMessageToServer(const char* message);
   
    void setPublisherIP(char* PublisherIP);
    char* getPublisherIP();
    
    //publisher side functions
    void connectToPublisher(char* Pub_IP);
    void ReceiveFileFromPublisher();
    void ReceiveMessageFromPublisher();
    void SendMessageToPublisher(char* message);
    void error(const char *msg){
        perror(msg);
        exit(0);
    }
};

void SubscriberConnection::ReceiveMessageFromServer(){ //To receive messages from the server
    bzero(buffer, sizeof(buffer));
    recv(SubscriberSockfd,buffer,sizeof(buffer),0);
    cout << buffer;
}

void SubscriberConnection::SendMessageToServer(const char* message){ //To send messages to the server
    send(SubscriberSockfd,message,sizeof(message),0);
}


void SubscriberConnection::ReceiveMessageFromPublisher(){ //To receive messages from the Publisher
    bzero(buffer, sizeof(buffer));
    int bytesRcvd;
    string str;

    do{
        bytesRcvd = recv(SubToPubSockfd,buffer,sizeof(buffer)-2,0);
        buffer[bytesRcvd] = '\0';
        str+=buffer;
    }while(bytesRcvd!=0);

    cout<<str;
}

string SubscriberConnection::ReceiveKeyFromServer(){
    bzero(buffer, sizeof(buffer));
    int n = recv(SubscriberSockfd,buffer,sizeof(buffer),0);
    KEY = buffer;
    return KEY;
}

void SubscriberConnection::SendMessageToPublisher(char* message){ //To send messages to the Pubisher
    send(SubToPubSockfd,message,sizeof(message),0);
}

// void SubscriberConnection::ReceiveFileFromPublisher(){ //To receive file from the publisher
//     char* fileName  = "test.txt";
//     //getline(cin,fileName);
//     write(SubscriberSockfd,fileName,sizeof(fileName));
//     int ch = 0;
//     char buffer[256]= {0};
//     FILE* fp;
//     //recv(SubToPubSockfd, fileName, strlen(fileName),0);
//     fp = fopen(fileName, "a");
//     int words;

//     read(SubToPubSockfd, &words, sizeof(int));
//     cout<<words;
//     while(ch!=words){
//         read(SubToPubSockfd,buffer, 255);
//         fprintf(fp,"%s",buffer);
//         cout<<buffer;
//         ch++;
//     } 
//     printf("The file was received successfully\n\n");
// }

void SubscriberConnection::connectToServer(char* argv){
    
    SubscriberSockfd = socket(AF_INET,SOCK_STREAM,0);
    if(SubscriberSockfd < 0) {
        error("ERROR opening socket");
    }
    
    bzero(&server_address,sizeof(server_address));
    server_address.sin_family=AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port=htons(SERV_PORT);
    
    inet_pton(AF_INET,argv,&server_address.sin_addr);
    
    if (connect(SubscriberSockfd,(struct sockaddr*)&server_address,sizeof(server_address)) < 0) {
        error("ERROR connecting");
        exit(-1);
    }
    string subName;

    cout <<"Connected to server"<<endl;
    cout<<"Enter your name:"<<endl;
    cin>>subName;
    write(SubscriberSockfd,subName.c_str(),subName.length());
    cout<<"Sent name"<<endl;
    //bzero(buffer, sizeof(buffer));

    int ch;
	do{
        //cin.clear(); cin.sync();
        send(SubscriberSockfd, "TEST", 5, 0);
        bzero(buffer, sizeof(buffer));
        cout<<"Receiving new notifs"<<endl;
        bzero(buffer, sizeof(buffer));
        if ( recv(SubscriberSockfd, buffer, sizeof(buffer), 0 ) < 0){
                 error("ERROR reading from socket\n");
                 exit(0);
        }
       // cout<<buffer<<endl;
		cout<<"Do you want to see the list of categories(1) or request file(2)  or subscribe(3) 0 for exit?"<<endl;
        cin>>ch;
        cout<<"choice " <<ch<<endl;
        
		switch(ch)
		{
		case 1:{
			send(SubscriberSockfd, "1", 2, 0);
			cout<<"Displaying list"<<endl;
            bzero(buffer, sizeof(buffer));
            if ( read(SubscriberSockfd, buffer, sizeof(buffer) ) < 0){
                error("ERROR reading from socket\n");
                exit(0);
            }
            cout<<buffer<<endl;
			break;
		}
		case 2:{
            string category, filename;
			send(SubscriberSockfd, "2", 2, 0);
			cout<<"Enter the category fromm switch"<<endl;
            cin>>category;
            send(SubscriberSockfd, category.c_str(), category.length(), 0);
            bzero(buffer, sizeof(buffer));
            //recv file
            if( recv(SubscriberSockfd, buffer, sizeof(buffer), 0) > 0 ){
                filename = buffer;
                cout<<"Filename:"<<filename<<endl;
                send(SubscriberSockfd, filename.c_str(), filename.length(), 0);
                int recv_file_size;
                bzero(buffer,sizeof(buffer));
                if( recv_file_size = recv(SubscriberSockfd, buffer, sizeof(buffer), 0) > 0 ){
                    cout<<"Recieved IP:key "<<buffer<<endl;
                    string IP_KEY = buffer;
                    int pos = IP_KEY.find(":");
                    Publisher_IP = IP_KEY.substr(0, pos-1);
                    KEY = IP_KEY.substr(pos+1, recv_file_size-1);
                    cout<<"IP : "<<Publisher_IP<<"\n key : "<<KEY<<endl;
                }
            }
			break;
        
		}
        case 3:{
            string category;
            send(SubscriberSockfd, "3", 2, 0);
			cout<<"Displaying list"<<endl;
            bzero(buffer, sizeof(buffer));
            //recv categories to subscribe
            if ( read(SubscriberSockfd, buffer, sizeof(buffer) ) < 0){
                error("ERROR reading from socket\n");
                exit(0);
            }
            cout<<buffer<<endl;
            cout<<"Enter the category you want to subscribe to:"<<endl;
            cin>>category;
            write(SubscriberSockfd, category.c_str(), category.length());

			break;
        }
		case 0:{
			cout<<"Closing socket and exiting"<<endl;
			close(SubscriberSockfd);
			exit(0);
			break;
		}
		default:
			cout<<"Invalid choice!"<<endl;
			break;
		}
        //bzero(buffer, 256);
        // if ( read(SubscriberSockfd, buffer, sizeof(buffer) ) < 0){
        //          error("ERROR reading from socket\n");
        //          exit(0);
        // }
        // 
        //cout<<buffer<<endl;	
    }while(ch!=0);
}

void SubscriberConnection::subscribe(){ //subscriber is done
    SendMessageToServer("Subscribe");
    ReceiveMessageFromServer(); //Receive list of topics from the subscriber
    cout << "Enter the topics you want to subscribe to\n";
    char* topic; //Topic to subscribe
    cin >> topic;
    SendMessageToServer(topic);
    //Modify the below part based on how the data is stored in the database
    ReceiveMessageFromServer(); //receive table of IPs and file names
    cout << "Enter the Publisher IP address:";
    cin >> Publisher_IP;
    SendMessageToServer(Publisher_IP.c_str());
    ReceiveKeyFromServer(); //Receive key of the publisher from the server
}

void SubscriberConnection::connectToPublisher(char* Pub_IP){ //connection to publisher works
    
    SubToPubSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(SubToPubSockfd < 0){
        error("ERROR opening socket");
    }
    struct hostent *server;
    server = gethostbyname(Pub_IP);
    bzero(&publisher_as_server,sizeof(publisher_as_server));
    publisher_as_server.sin_family = AF_INET;
    bcopy((char*)server->h_addr,(char*)&publisher_as_server.sin_addr.s_addr,server->h_length);
    //publisher_as_server.sin_addr.s_addr = INADDR_ANY;
    publisher_as_server.sin_port = htons(SUB_PUB_PORT);
    
    inet_pton(AF_INET,Pub_IP,&publisher_as_server.sin_addr);


    if (connect(SubToPubSockfd,(struct sockaddr *) &publisher_as_server,sizeof(publisher_as_server)) < 0) {
        error("ERROR connecting");
    }
    cout<<"Connected to Publisher\n";
    cout << "Message from Publisher:";
    ReceiveMessageFromPublisher();
    //ReceiveFileFromPublisher();
}

int main(int argc, char** argv){
    SubscriberConnection *subscriber = new SubscriberConnection();
     subscriber->connectToServer("127.0.0.1");
}