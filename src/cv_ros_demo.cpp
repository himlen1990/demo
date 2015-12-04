/*
  how it works
  1. find the desk using color filter (the color of the desk is already known
  2. find the contour of desk (it should be the largest contour)
  3. creat a bounding box of the desk(largest contour)
  4. make the bounding box smaller, so that we can detect the cup easier
  5. find the cup using houghcircle
  6. vote and decide the mid point of the cup
*/

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <geometry_msgs/PointStamped.h>



using namespace cv;

class ImageConverter
{
  ros::NodeHandle nh_;
  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
  image_transport::Publisher image_pub_;

public:
  ImageConverter()
    : it_(nh_)
  {

    image_sub_ = it_.subscribe("/camera/rgb/image_rect_color", 1, 
                               &ImageConverter::imageCb, this);
    
    image_pub_ = it_.advertise("cup_center", 100);

    cv::namedWindow("show_win",0);
  }

  ~ImageConverter()
  {
    cv::destroyWindow("show_win");
  }

  void imageCb(const sensor_msgs::ImageConstPtr& msg)
  {
    cv_bridge::CvImagePtr cv_ptr;
    try
      {
        cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
      }
    catch (cv_bridge::Exception& e)
      {
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
      }

    // Draw an example circle on the video stream
    IplImage img = cv_ptr->image;
    IplImage* img2= &img;

    // Update GUI Window
       cvShowImage("show_win", img2);
       cv::waitKey(3);
    
    // Output modified video stream
    image_pub_.publish(cv_ptr->toImageMsg());
  }
};


int main(int argc, char** argv)
{
  ros::init(argc, argv, "image_converter");
  ImageConverter ic;
  ros::Rate loop_rate(1);
  while(ros::ok())
    {
      ros::spinOnce();
      loop_rate.sleep();
    }
  return 0;
}
