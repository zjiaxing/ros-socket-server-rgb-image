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
using namespace std;;

class rgbClient
{

  
public:
rgbClient(): it_(nh_)
  {
	iniTcp();
    // Subscrive to input video feed 
    image_sub_rgb = it_.subscribe("/camera/rgb/image_color", 1, &rgbClient::imageRgb, this);
   
  }

  ~rgbClient()
  {
   closeTcp();
  }

  void imageRgb(const sensor_msgs::ImageConstPtr& msg);
  void iniTcp();
  void sendrgb(cv::Mat src);
  void closeTcp();
private:
  	ros::NodeHandle nh_;
  	image_transport::ImageTransport it_;
  	image_transport::Subscriber image_sub_rgb;
  	image_transport::Publisher image_pub_;
  
  	cv::Mat rgb;
  	int client_sockfd;
  	int rgblen;
  	struct sockaddr_in remote_addr; //server network address structure
  	char buf[BUFSIZ];  //buffer for data transfer

};
 void rgbClient::imageRgb(const sensor_msgs::ImageConstPtr& msg)
  {
    cv_bridge::CvImagePtr cvRgb_ptr;
    try
    {
      cvRgb_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cvRgb_bridge exception: %s", e.what());
      return;
    }
	
	rgb=cvRgb_ptr->image;
	//cv::imshow("rgb", rgb);
   	//cv::waitKey(3);
        sendrgb(rgb);
   
    
  }

void rgbClient::iniTcp()
  {
	memset(&remote_addr,0,sizeof(remote_addr)); //Data initialization - clear
	remote_addr.sin_family=AF_INET; //Set to IP communication
	remote_addr.sin_addr.s_addr=inet_addr("127.0.0.1");//Server IP address
	remote_addr.sin_port=htons(8000); //Server port number
	
	/*Create client socket --IPv4 protocol, connection oriented communication, TCP protocol*/
	if((client_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket");
		return ;
	}
	
	/*Bind the socket to the network address of the server*/
	if(connect(client_sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)
	{
		perror("connect");
		return ;
	}
	printf("connected to server/n");
	
	
         return ;
	
  }
void rgbClient::sendrgb(cv::Mat src)
  {
	
	
		printf("Enter rgbimage to send:");
		cv::Mat rgbk;
		src.copyTo(rgbk); 
		//cv::imshow("rgbClient", src);
		//cv::waitKey(30);
			
	        rgbk=(rgbk.reshape(0,1));//to make it continuous
		int  rgbSize = rgbk.total()*rgbk.elemSize();
		printf("send image:");
		rgblen=send(client_sockfd,rgbk.data,rgbSize,0);
		
	
	//close(client_sockfd);//close socket
	return ;
  }

void rgbClient::closeTcp()
{
	close(client_sockfd);//close socket
}
int main(int argc, char** argv)
{
  	ros::init(argc, argv, "rgbClient");
 	
  	rgbClient rc;

  	ros::spin();
	return 0;


         
}





