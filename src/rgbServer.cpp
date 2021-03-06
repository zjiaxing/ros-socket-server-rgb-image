#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace cv;
using namespace std;

class rgbServer
{

public:
rgbServer():it(nh_)
{
	iniTcp();
	recvimage();
        rgb_pub_ = it.advertise("/image_recv/rgbd_image", 1);
}
	
void iniTcp();
void recvimage();
void closeTcp();

 ~rgbServer()
{
closeTcp();
}

private:
	int server_sockfd;//Server socket
	int client_sockfd;//Client socket
	cv::Mat rgb;
        int rgblen;
	int rgbSize;
	int len;
	struct sockaddr_in my_addr;   //server network address structure
	struct sockaddr_in remote_addr; //client network address structure
	socklen_t sin_size;
	char buf[BUFSIZ];  //buffer for data transfer
	

	//public recv image
	ros::NodeHandle nh_;
	image_transport::ImageTransport it;
	image_transport::Publisher rgb_pub_;
	sensor_msgs::ImagePtr msg;


};

void rgbServer::iniTcp()
{
	memset(&my_addr,0,sizeof(my_addr)); //Data initialization - clear
	my_addr.sin_family=AF_INET; //Set to IP communication
	my_addr.sin_addr.s_addr=INADDR_ANY;//Server IP address - allows to connect to all local addresses
	my_addr.sin_port=htons(8000); //Server port number
	
	/*Create client socket --IPv4 protocol, connection oriented communication, TCP protocol*/
	if((server_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
	{  
		perror("socket");
		return ;
	}
 
        /*Bind the socket to the network address of the server*/
	if (bind(server_sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))<0)
	{
		perror("bind");
		return ;
	}
	
	/*Monitor connection request - monitor the queue length of 5*/
	listen(server_sockfd,5);
	
	sin_size=sizeof(struct sockaddr_in);
	
	/*Waiting for client connection requests to arrive*/
	if((client_sockfd=accept(server_sockfd,(struct sockaddr *)&remote_addr,&sin_size))<0)
	{
		perror("accept");
		return ;
	}
	printf("accept client %s/n",inet_ntoa(remote_addr.sin_addr));
	len=send(client_sockfd,"Welcome to my server/n",21,0);//Send welcome message
	return ;
}

void rgbServer::recvimage()
{
	while(ros::ok())
	{
	
	
         /*Receives the data from the client and sends it to the client --recv to return the number of bytes received. 
           Send returns the number of   bytes sent.*/
		rgb = Mat::zeros( 480,640, CV_8UC3);
                int  rgbSize = rgb.total()*rgb.elemSize();
                uchar sockDataRgb[rgbSize];

               //Receive data here

              for (int i = 0; i < rgbSize; i += rgblen) 
		{
  		 	if ((rgblen = recv(client_sockfd, sockDataRgb +i, rgbSize-i, 0)) == -1) 
			{
    				 cout<<"recv failed"<<endl;
    			}
   		}

 

 		int ptr=0;
 		for (int i = 0;  i < rgb.rows; i++) {
 	 		for (int j = 0; j < rgb.cols; j++) {                                     
 	  	rgb.at<cv::Vec3b>(i,j) = cv::Vec3b(sockDataRgb[ptr+ 0],sockDataRgb[ptr+1],sockDataRgb[ptr+2]);
 	 	 ptr=ptr+3;
 	  		}
 		 }

	  cv::imshow("rgbServer",rgb);
	  cv::waitKey(30);

	 //public recv image
          
	 msg= cv_bridge::CvImage(std_msgs::Header(), "bgr8", rgb).toImageMsg();
	 rgb_pub_.publish(msg);
	// ros::spinOnce();
         //cv::waitKey(30);

	
	
	}
	//closeTcp();
	return ;
}
void rgbServer::closeTcp()
{
	close(client_sockfd);
	close(server_sockfd);
}

int main(int argc, char *argv[])
{
	ros::init(argc, argv, "rgbServer");
      
	rgbServer rs;
	
	ros::spin();
	
        return 0;



}
